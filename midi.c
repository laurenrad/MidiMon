/* Copyright 2023 Lauren Rad
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /*
  * Application: !Midimon
  * File: midi.c
  * Author: Lauren Rad
  * Purpose: Code for interacting with the USBMIDI module.
  */

#pragma check_swix_formats

#include <stdio.h>

#include "werr.h"               // RISC_OSLib
#include "kernel.h"
#include "swis.h"
#include "event.h"

// MidiMon stuff
#include "common.h"
#include "midi.h"
#include "preporter.h"
#include "monitorwin.h"

/*
 * These structs correspond to the data coming from the MIDIEvent helper module.
 */
typedef struct MIDIEventData {
    int event;
} MIDIEventData;

typedef struct PollWordData {
    int nonzero;
    int key_count;
    int midi_count;
} PollWordData;

/*
 * device_count
 * Returns the number of MIDI devices detected.
 */
int device_count(void)
{
    int count;
    _kernel_oserror *err;

    /*
     * If this SWI isn't known (or anything else goes wrong, report 0
     * devices.
     */
    err = _swix(MIDI_USBInfo, _IN(0) | _OUT(0), 0, &count);
    if (err != NULL) {
        report_printf("MidiMon: Error while scanning devices: %x %s", err->errnum, err->errmess);
        return 0;
    }

    return count;
}


/*
 * clear_rx_buf
 * New and simplified, this will just force clear all Rx Buffers.
 */
int clear_rx_buf()
{
    _swi(MIDI_Init, _IN(0), 2);
    return 0;
}

/*
 * midi_incoming
 * Handle incoming MIDI notifications from the helper module.
 */
#define MIDI_DataReceived   0
#define MIDI_Error          1
#define MIDI_Connect        10
#define MIDI_Disconnect     11
#define MIDI_Initialised    20
#define MIDI_Dying          21
int midi_incoming(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
    WimpPollWordNonZeroEvent *e = (WimpPollWordNonZeroEvent *)event;
    PollWordData *pword = (PollWordData *)(e->poll_word);
    char printbuf[MaxLine];
    int command;
    int midi_event;
    int buffree = -1;

    if (pword->midi_count > 0) {
        _swi(MIDIEvent_GetMIDIEvent,_OUT(0),&midi_event);
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Received MIDI event %d",midi_event);
#endif

        switch (midi_event) {
            case MIDI_DataReceived:
            /* MidiSupport will tack on the port number to padding as well, so
               mask it off first before checking */
            while (((command = read_rx_command(-1) & 0x0FFFFFFF)) != 0) {
                parse_command(command, printbuf, MaxLine);
                add_to_monitor(printbuf);
            }
            /* At this point, we should be done in almost all current cases, BUT
             * the MidiSupport module will pass through 0 padding.
             * One solution would be to check buffer size on this end, but
             * MIDI_InqBufferSize seems to behave inconsistently between modules
             * and I'm not sure yet of a standard way to get max size.
             * Note to self: report that MIDI_InqBufferSize off-by-one to Peter when
             * I get a chance. It's reporting 2044 when clear.
             * So for now, the cheapest solution seems to be to just force Rx
             * buffers clear to ensure any future events will fire. Feel free to
             * revisit decision later.
             */
            clear_rx_buf();
            break;
            /* The rest of these aren't fully implemented yet. */
            case MIDI_Error:
            report_printf("MidiMon: A MIDI error has occurred");
            break;
            case MIDI_Connect:
            report_printf("MidiMon: A MIDI device has been connected");
            break;
            case MIDI_Disconnect:
            report_printf("MidiMon: A MIDI device has been disconnected");
            break;
            case MIDI_Initialised:
            report_printf("MidiMon: MIDI Module has been initialised");
            break;
            case MIDI_Dying:
            report_printf("MidiMon: MIDI Module is dying");
            break;
            default:
            report_printf("MidiMon: Unknown event received from MIDIEvent: %d",midi_event);
            break;
        }
        return 1;
    }

    return 0; // if there was no MIDI event, see if there are any KeyEvents
}

/*
 * read_rx_command
 * Returns the next message from the Rx buf. This should be called repeatedly
 * until the buffer is clear, so new MIDIEvent messages are triggerd.
 */
int read_rx_command(int device)
{
    report_printf("read_rx_command: device is %d",device);
    int command;
    _swi(MIDI_RxCommand, _IN(0) | _OUT(0), device, &command);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: received new command: %x", command);
#endif

    return command;
}

/*
 * tx_noteon
 * Sends a note on message; oct_shift can shift octave up and down, but if this
 * results in out of range note number it will be ignored.
 * Returns 0 on success or the most recent error status given by the MIDI module on
 * failure.
 */
int tx_noteon(int note, int velocity, int oct_shift)
{
    int status = 0;             // error status, if any
    _kernel_oserror *err;

    note = note + (12 * oct_shift);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Note on tx: %d", note);
#endif

    err = _swix(MIDI_SetTxChannel, _IN(0), global_choices.opt_txchan);
    err = _swix(MIDI_TxNoteOn, _INR(0, 1) | _OUT(1), note, velocity, &status);

    if (err != NULL) {
        return err->errnum;
    }

    return status;
}

/*
 * tx_noteoff
 * Sends a note off message: as above invalid note numbers will be ignored.
 * This doesn't use MIDI_TxNoteOff for reasons I don't remember, but are probably valid.
 * Depending on the global choice opt_altnoteoff, this will either send a note off message
 * for the given note, or a zero-velocity note on message.
 * Returns 0 on success or most recent module error code on failure.
 */
int tx_noteoff(int note, int velocity, int oct_shift)
{
    note = note + (12 * oct_shift);
    _kernel_oserror *err;

    if (global_choices.opt_altnoteoff == 1) {   // zero velocity note on
        err = _swix(MIDI_SetTxChannel, _IN(0), global_choices.opt_txchan);
        err = _swix(MIDI_TxNoteOn, _INR(0, 1), note, 0);
    } else {                    // note off message. Currently hardcoded port=0
        int comm = 0;
        comm = 0x80;            // byte 0 high bits = command
        comm = comm | global_choices.opt_txchan - 1;    // byte 0 low bits = chan
        comm = comm | (note << 8);      // byte 1 is note
        comm = comm | (velocity << 16); // byte 2 is velocity
        err = _swix(MIDI_TxCommand, _INR(0, 1), comm, 0); // send immediately
    }

    if (err != NULL) {
        return err->errnum;
    }

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Note off tx: note=%d, vel=%d, chan=%d", note, velocity,
                  global_choices.opt_txchan);
#endif

    return 0;
}

/*
 * tx_progchg
 * Sends a program change message.
 */
void tx_progchg(int prog)
{
    _swi(MIDI_TxProgramChange, _IN(0), prog);

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Prog change tx: prog=%d", prog);
#endif
}

/*
 * tx_controlchg
 * Sends a control change message.
 */
void tx_controlchg(int control, int value)
{
    _swi(MIDI_TxControlChange, _INR(0, 1), control, value);

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Control change tx: control=%d, value=%d", control, value);
#endif

}

/*
 * tx_songstart
 * Sends a songs start command.
 */
void tx_songstart(void)
{
    _swi(MIDI_TxStart, 0);

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Song start tx");
#endif
}

/*
 * tx_songcontinue
 * Sends a song continue command.
 */
void tx_songcontinue(void)
{
    _swi(MIDI_TxContinue, 0);

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Song continue tx");
#endif
}

/*
 * tx_songstop
 * Sends a song stop command.
 */
void tx_songstop(void)
{
    _swi(MIDI_TxStop, 0);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Song stop tx");
#endif
}

/*
 * tx_songsel
 * Sends a song select message with the given song number.
 */
void tx_songsel(int num)
{
    _swi(MIDI_TxSongSelect, _IN(0), num);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Song sel tx: song=%d", num);
#endif
}

/*
 * tx_tunereq
 * Sends a tune request command.
 */
void tx_tunereq(void)
{
    _swi(MIDI_TxTuneRequest, 0);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Tune request tx");
#endif
}

/*
 * tx_sysreset
 * Sends a system reset command.
 */
void tx_sysreset(void)
{
    _swi(MIDI_TxSystemReset, 0);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: System reset tx");
#endif
}

/*
 * tx_pitchwheel
 * Sends a pitch wheel command.
 */
void tx_pitchwheel(int pitch)
{
    _swi(MIDI_TxPitchWheel, _IN(0), pitch);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Pitch wheel tx: pitch=%d", pitch);
#endif
}

/*
 * ignore_timing
 * If argument is nonzero, sets the MIDI module to ignore timing messages.
 * If argument is 0, sets the MIDI module to receive timing messages.
 */
void ignore_timing(int option)
{
    if (option == 0) {
        _swi(MIDI_IgnoreTiming, _IN(0), 0);
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Timing Rx set to On.");
#endif
    } else {
        _swi(MIDI_IgnoreTiming, _IN(0), 1);
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Timing Rx set to Off.");
#endif
    }
}

/* Set fake fast clock. Pass 0 for FFC off, 1 for on. Returns options bitmap or -1 on error. */
/*
 * fake_fast_clock
 * If argument is nonzero, enables fake fast clock in the MIDI module.
 * If argument is zero, disables fake fast clock in the MIDI module.
 * Returns options bitmap or -1 on error.
 */
int fake_fast_clock(int option)
{
    int bitmap = -1;

    if (option == 0) {
        _swi(MIDI_Options, _IN(0), 0);
    } else {
        _swi(MIDI_Options, _IN(0), 1);
    }

    _swi(MIDI_Options, _IN(0) | _OUT(0), -1, &bitmap);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: FFC altered. Options bitmap is %x", bitmap);
#endif

    return bitmap;
}

/*
 * reset_midi
 * Resets the MIDI module status by calling SWI MIDI_Init.
 */
void reset_midi(void)
{
    _swi(MIDI_Init, _IN(0), 0);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: MIDI module status reset.");
#endif
}

/*
 * set_tx_channel
 * Sets the Tx channel by calling SWI MIDI_SetTxChannel. Returns new channel.
 */
int set_tx_channel(int device, int channel)
{
    int new_chan = 0;
    channel = channel + (16 * (device - 1));    // convert channel number to port number based on device
    _swi(MIDI_SetTxChannel, _IN(0) | _OUT(0), channel, &new_chan);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Tx channel set. device=%d channel=%d", device, new_chan);
#endif

    return new_chan;
}

/*
 * midi_error
 * Debug function to report MIDI error messages from the MIDIEvent module.
 * This is intended to be called only if REPORTER_DEBUG is on,
 * as many of these errors are common and expected.
 */
int midi_error(WimpMessage *message, void *handle)
{
    report_printf("MidiMon: A MIDI error has occurred. Details:");
    int err = 0;
    _kernel_oserror *oserr = NULL;
    oserr = _swix(MIDI_InqError, _OUT(0), &err);
    if (oserr != NULL) {
        report_printf("  Ironically, an error occurred while trying to get the MIDI error.");
    } else {
        /*
         * each byte of err represents a different device, with LSB being dev 0.
         */
        int dev_errs[4];
        dev_errs[0] = err & 0xFF;
        dev_errs[1] = (err >> 8) & 0xFF;
        dev_errs[2] = (err >> 16) & 0xFF;
        dev_errs[3] = (err >> 24) & 0xFF;
        for (int i = 0; i < 4; i++) {
            switch (dev_errs[i]) {
            case 0:
                report_printf("  Device %d: No error", i);
                break;
            case 65:
                report_printf("  Device %d: Error 65: Active sensing no longer received.", i);
                break;
            case 66:
                report_printf("  Device %d: Error 66: Receive buffer is full, data lost.", i);
                break;
            case 68:
                report_printf("  Device %d: Error 68: Unrecognised data discarded.", i);
                break;
            case 88:
                report_printf("  Device %d: Error 88: USB device has been disconnected.", i);
                break;
            case 47:
                report_printf("  Device %d: Error 47: USB Device not present.", i);
                break;
            default:
                report_printf("  Device %d: Unrecognised error.", i);
                break;
            }
        }
    }

    return 1;
}

/*
 * midi_dying
 * Handles MIDI dying messages from the MIDIEvent module.
 * This needs to actually do more, but currently just throws up an error
 * if the module quits while the application is running.
 */
int midi_dying(WimpMessage *message, void *handle)
{
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: MIDI module is dying");
#endif
    werr(1, "The MIDI module is no longer running. MidiMon will now exit.");
    return 1;
}

/*
 * midi_dev_connected
 * Handles MIDI device connect messages from the MIDIEvent module.
 * Since only one device is currently supported, this is unused other than
 * printing a message to Reporter.
 */
int midi_dev_connected(WimpMessage *message, void *handle)
{
    report_printf("MidiMon: A new MIDI device has been connected");
    return 1;
}

/*
 * midi_dev_disconnected
 * Handles a MIDI device disconnected message from the MIDIEvent module.
 * As there's no way of necessarily knowing which device was disconnected and they may be
 * renumbered, this will just set no device and then update the device display.
 */
int midi_dev_disconnected(WimpMessage *message, void *handle)
{
    report_printf("MidiMon: A MIDI device has been disconnected");
    device_num = -1;            // flag no selected device
    update_device_display();
    return 1;
}

/*
 * get_product_name
 * Returns a pointer to the name of a given device. Devices are numbered 1-4.
 * Returns NULL if anything goes wrong.
 */
char *get_product_name(int device)
{
    char *prod_name;
    _kernel_oserror *err;
    err = _swix(MIDI_USBInfo, _IN(0) | _OUT(2), device, &prod_name);

    if (err != NULL) {
        return NULL;
    }

    return prod_name;
}

/*
 * parse_command
 * Parses a MIDI message and puts a printable string in buf.
 */
void parse_command(int command, char *buf, int buf_size)
{
    /*
     * unpack the message
     */
    unsigned int status = (unsigned int)command & 0xFF;      // byte 0: status byte
    unsigned int size = ((unsigned int)command >> 24) & 3;     // bits 24-25 are size of command
    unsigned int data1 = ((unsigned int)command >> 8) & 0xFF;  // byte 1: data byte 1
    unsigned int data2 = ((unsigned int)command >> 16) & 0xFF; // byte 2: data byte 2
    int channel;
    static int in_sysex = 0; // are we in the middle of SysEx?
    int buflen = 0; // track used buffer length

#ifdef REPORTER_DEBUG
    report_printf("parsing command. in_sysex=%d status=%x size=%x data1=%x data2=%x", in_sysex, status, size, data1, data2);
#endif
    if (in_sysex) { // handle sysex bytes until done
        buflen += snprintf(buf, buf_size, "[System Exclusive] ");
        for (int i = 0; i < size; i++) {
            status = command &0xFF;
            if (status == 0xF7) {
                in_sysex = 0;
                snprintf(buf+(sizeof(char)*buflen),
                        buf_size, " [System Exclusive End]");
                break;
            }

            buflen += snprintf(buf+(sizeof(char)*buflen),
                                buf_size, "0x%2x",status);
            command >>= 8;
        }
    }
    else if (status >= 0x80 && status <= 0xEF) {     // channel-specific
        channel = status & 0x0F;        // lower nibble is channel
        status = status & 0xF0; // higher nibble command
        switch (status) {
        case 0x80:             // Note Off
            snprintf(buf, buf_size, "[Note Off] Note=%d Velocity=%d", data1, data2);
            break;
        case 0x90:             // Note On
            snprintf(buf, buf_size, "[Note On] Note=%d Velocity=%d", data1, data2);
            break;
        case 0xA0:             // Aftertouch / Key Pressure
            snprintf(buf, buf_size, "[Aftertouch] Key=%d Pressure=%d", data1, data2);
            break;
        case 0xB0:             // Controller Change
            snprintf(buf, buf_size, "[Controller Change] Controller=%d Value=%d", data1, data2);
            break;
        case 0xC0:             // Program Change
            snprintf(buf, buf_size, "[Program Change] Program=%d", data1);
            break;
        case 0xD0:             // Channel Pressure
            snprintf(buf, buf_size, "[Channel Pressure] Pressure=%d", data1);
            break;
        case 0xE0:             // Pitch Bend
            snprintf(buf, buf_size, "[Pitch Bend] LSB=0x%2x MSB=0x%2x", data1, data2);
            break;
        default:
            snprintf(buf, buf_size, "Unknown command");
            break;
        }
    } else {                    // Not channel-specific
        switch (status) {
        case 0xF0:             // System Exclusive. Not fully implemented but at least reports that there was one
            snprintf(buf, buf_size, "[System Exclusive] 0x%2x 0x%2x", data1, data2);
            in_sysex = 1;
            break;
        case 0xF1:             // MTC Quarter Frame
            snprintf(buf, buf_size, "MTC Quarter Frame: 0x%2x %2x", data1, data2);
            break;
        case 0xF2:             // Song Position Pointer
            snprintf(buf, buf_size, "[Song Position] LSB=0x%2x MSB=0x%2x", data1, data2);
            break;
        case 0xF3:             // Song Select
            snprintf(buf, buf_size, "[Song Select] Song=%d", data1);
            break;
        case 0xF5:             // Bus Select: nonstandard, vendor-specific. Untested
            snprintf(buf, buf_size, "[Bus Select] Bus=%d", data1);
            break;
        case 0xF6:             // Tune Request
            snprintf(buf, buf_size, "[Tune Request]");
            break;
        case 0xF8:             // Clock
            snprintf(buf, buf_size, "Clock");
            break;
        case 0xFA:             // Start
            snprintf(buf, buf_size, "Start");
            break;
        case 0xFB:             // Continue
            snprintf(buf, buf_size, "Continue");
            break;
        case 0xFC:             // Stop
            snprintf(buf, buf_size, "Stop");
            break;
        case 0xFE:             // Active Sensing
            snprintf(buf, buf_size, "Active Sensing");
            break;
        case 0xFF:             // System Reset
            snprintf(buf, buf_size, "System Reset");
            break;
        default:
            snprintf(buf, buf_size, "Unknown command: %x",command);
            break;
        }
    }

}
