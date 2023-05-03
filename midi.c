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

/* Library headers */
#include <stdio.h>

/* RISC_OSLib headers */
#include "werr.h"

/* System headers */
#include "kernel.h"
#include "swis.h"
#include "event.h"

/* MidiMon headers */
#include "common.h"
#include "midi.h"
#include "preporter.h"
#include "monitorwin.h"

#pragma check_swix_formats

/* Return the number of MIDI devices. */
int device_count(void)
{
  int count;
  _kernel_oserror *err;

  /* If this SWI isn't known (or anything else goes wrong, report 0
     devices. */
  err = _swix(MIDI_USBInfo,_IN(0)|_OUT(0),0,&count);
  if (err != NULL) {
    report_printf("MidiMon: Error while scanning devices: %x %s",err->errnum,err->errmess);
    return 0;
  }

  return count;
}

/* Clear the Rx Buffer. Returns 0 on success, or the error number of any
   _kernel_oserror detected. */
int clear_rx_buf(int device)
{
  int buf_free, error_code, command;
  int buf_last_free; /* last known buffer free space. so we can stop if clearing isn't working. */
  _kernel_oserror *err;

  /* Since this is called before entering the polling loop, check that
     everything is ok. For now, at least checking here should ensure that
     the MIDI module is loaded before startup, otherwise a SWI not known
     error will be returned here. */
  err = _swix(MIDI_InqError,_OUT(0),&error_code);
  if (err != NULL) {
    return err->errnum;
  }
  if (error_code == 'B') {
    report_printf("MidiMon: Receive buffer full!");
  }

  /* Prior to 0.08, USB-MIDI had a bug where MIDI_InqBufferSize returned
     the buffer size rather than the number of unused buffer bytes. This
     appears to have now been fixed, but if written like this it will
     require 0.08. */
  /* bits 1-2 specify device, bit 0=0 read buf size. Note that for
     this SWI, devices are numbered from 0 (0-3) rather than 1 */
  _swi(MIDI_InqBufferSize,_IN(0)|_OUT(0),(device-1)<<1,&buf_free);
  buf_last_free = buf_free;
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: clear_rx_buf: device %d buffer free %d",device,buf_free);
#endif

  /* keep reading from the Rx buffer until it's empty. note that currently
  if no device is connected, this may hang the system! FIXME */

  while (buf_free < 2048) {
    _swi(MIDI_RxCommand,_IN(0)|_OUT(0),device,&command);
    _swi(MIDI_InqBufferSize,_IN(0)|_OUT(0),(device-1)<<1,&buf_free);

    if (buf_free == buf_last_free) {
      report_printf("MidiMon: err: can't empty buffer!");
      break; /* break out if the buffer isn't clearing. this will still return 0 though. */
    }
  }

  return 0;
}

/* Return the next message from the Rx buf. This should be called repeatedly
   until the buffer is clear, so new Wimp messages are triggered */
int read_rx_command(int device)
{
  int command;
  _swi(MIDI_RxCommand,_IN(0)|_OUT(0),0,&command);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: received new command: %x",command);
#endif

  return command;
}

/* Send a note on message; oct_shift can shift octave up and down, so
   invalid note numbers will just be ignored.
   Returns any error status given by the MIDI module.
*/
int tx_noteon(int note, int velocity, int oct_shift)
{
  int status = 0; /* error status, if any */

  note = note + (12 * oct_shift);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Note on tx: %d",note);
#endif

  _swi(MIDI_TxNoteOn,_INR(0,1)|_OUT(1),note,velocity,&status);

  return status;
}

/* Send a note off message: as above invalid note numbers will be ignored.
   Returns 0; currently MIDI_TxNoteOff doesn't return an error status but if future versions do,
   any error status will be returned here.
*/
int tx_noteoff(int note, int velocity, int oct_shift)
{
  note = note + (12 * oct_shift);
  int chan = global_choices.opt_txchan - 1; /* opt_txchan is numbered 0-15 */

  /* TxNoteOff will send note off as a note on with velocity 0; switch on the relevant choice. */
  if (global_choices.opt_altnoteoff == 1) {
    _swi(MIDI_SetTxChannel,_IN(0)|_OUT(0),0,&chan);
    _swi(MIDI_TxNoteOn,_INR(0,1),note,0);
  }
  else {
    /* Send a proper note off manually. Currently hardcoded port=0 */
    int comm = 0;
    comm = 0x80; /* byte 0 high bits = command */
    comm = comm | chan; /* byte 0 low bits = chan */
    comm = comm | (note << 8); /* byte 1 is note */
    comm = comm | (velocity << 16); /* byte 2 is velocity */
    _swi(MIDI_TxCommand,_INR(0,1),comm,0); /* send immediately */
  }

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Note off tx: note=%d, vel=%d, chan=%d",note,velocity,chan);
#endif

  return 0;
}

/* Send a program change message */
void tx_progchg(int prog)
{
  _swi(MIDI_TxProgramChange,_IN(0),prog);

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Prog change tx: prog=%d",prog);
#endif
}

/* Send a control change message */
void tx_controlchg(int control, int value)
{
  _swi(MIDI_TxControlChange,_INR(0,1),control,value);

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Control change tx: control=%d, value=%d",control,value);
#endif

}

/* Transmit a song start command */
void tx_songstart(void)
{
  _swi(MIDI_TxStart,0);

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Song start tx");
#endif
}

/* Transmit a song continue command */
void tx_songcontinue(void)
{
  _swi(MIDI_TxContinue,0);

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Song continue tx");
#endif
}

/* Transmit a song stop command */
void tx_songstop(void)
{
  _swi(MIDI_TxStop,0);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Song stop tx");
#endif
}

/* Transmit a song select command. */
void tx_songsel(int num)
{
  _swi(MIDI_TxSongSelect,_IN(0),num);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Song sel tx: song=%d",num);
#endif
}

/* Transmit a tune request command. */
void tx_tunereq(void)
{
  _swi(MIDI_TxTuneRequest,0);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Tune request tx");
#endif
}

/* Transmit a system reset command. */
void tx_sysreset(void)
{
  _swi(MIDI_TxSystemReset,0);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: System reset tx");
#endif
}

/* Transmit a pitch wheel command. */
void tx_pitchwheel(int pitch)
{
  _swi(MIDI_TxPitchWheel,_IN(0),pitch);
#ifdef REPORTER_DEBUG
  /* This is continuous, so this debug output can be a bit annoying. Is it worth it? */
  report_printf("MidiMon: Pitch wheel tx: pitch=%d",pitch);
#endif
}

/* Turn timing clock messages on/off. Pass 0 to receive timing messages, nonzero to ignore. */
void ignore_timing(int option)
{
  if (option == 0) {
    _swi(MIDI_IgnoreTiming,_IN(0),0);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Timing Rx set to On.");
#endif
  }
  else {
    _swi(MIDI_IgnoreTiming,_IN(0),1);
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Timing Rx set to Off.");
#endif
  }
}

/* Set fake fast clock. Pass 0 for FFC off, 1 for on. Returns options bitmap or -1 on error. */
int fake_fast_clock(int option)
{
  int bitmap = -1;

  if (option == 0) {
    _swi(MIDI_Options,_IN(0),0);
  }
  else {
    _swi(MIDI_Options,_IN(0),1);
  }

  _swi(MIDI_Options,_IN(0)|_OUT(0),-1,&bitmap);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: FFC altered. Options bitmap is %x",bitmap);
#endif

  return bitmap;
}

/* Reset the MIDI module status by calling SWI MIDI_Init. */
void reset_midi(void)
{
  _swi(MIDI_Init,_IN(0),0);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: MIDI module status reset.");
#endif
}

/* Set the Tx channel by calling SWI MIDI_SetTxChannel. Returns new channel. */
int set_tx_channel(int device, int channel)
{
  int new_chan = -1; /* new channel number for debug */
  channel = channel + (16 * (device-1)); /* covert channel number to port number based on device */
  _swi(MIDI_SetTxChannel,_IN(0)|_OUT(0),channel,&new_chan);
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Tx channel set. device=%d channel=%d",device,new_chan);
#endif

  return new_chan;
}

/* Handle MIDI error message from the MidiEvent module.
   This is mostly just for debug for now, as many of these are common and expected. */
int midi_error(WimpMessage *message, void *handle)
{
  report_printf("MidiMon: A MIDI error has occurred. Details:");
  int err = 0;
  _kernel_oserror *oserr = NULL;
  oserr = _swix(MIDI_InqError,_OUT(0),&err);
  if (oserr != NULL) {
    report_printf("  Ironically, an error occurred while trying to get the MIDI error.");
  }
  else {
    /* each byte of err represents a different device, with LSB being dev 0. */
    int dev_errs[4];
    dev_errs[0] = err & 0xFF;
    dev_errs[1] = (err >> 8) & 0xFF;
    dev_errs[2] = (err >> 16) & 0xFF;
    dev_errs[3] = (err >> 24) & 0xFF;
    for (int i = 0; i < 4; i++) {
      switch (dev_errs[i]) {
        case 0:
             report_printf("  Device %d: No error",i);
        break;
        case 65:
             report_printf("  Device %d: Error 65: Active sensing no longer received.",i);
        break;
        case 66:
             report_printf("  Device %d: Error 66: Receive buffer is full, data lost.",i);
        break;
        case 68:
             report_printf("  Device %d: Error 68: Unrecognised data discarded.",i);
        break;
        case 88:
             report_printf("  Device %d: Error 88: USB device has been disconnected.",i);
        break;
        case 47:
             report_printf("  Device %d: Error 47: USB Device not present.",i);
        break;
        default:
             report_printf("  Device %d: Unrecognised error.",i);
        break;
      }
    }
  }

  return 1;
}

/* Handle MIDI init message from the MidiEvent module. Warning, this doesn't actually seem
   to fire for some reason. Will investigate later, but this is unused currently anyway. */
int midi_initialised(WimpMessage *message, void *handle)
{
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: The MIDI module has been initialised.");
#endif
  return 1;
}

/* Handle MIDI dying message from the MidiEvent module. */
int midi_dying(WimpMessage *message, void *handle)
{
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: MIDI module is dying");
#endif
  werr(1,"The MIDI module is no longer running. MidiMon will now exit.");
  return 1;
}

/* Handle device connect message from the MidiEvent module. */
int midi_dev_connected(WimpMessage *message, void *handle)
{
  report_printf("MidiMon: A new MIDI device has been connected");
  return 1;
}

/* Handle a MIDI device disconnected message. If a device is disconnected, set it to no device,
   as as we don't necessarily know which device was disconnected and they may get renumbered.
   The user will have to select a new device from the menu.
*/
int midi_dev_disconnected(WimpMessage *message, void *handle)
{
  report_printf("MidiMon: A MIDI device has been disconnected");
  device_num = -1; /* flag no selected device */
  update_device_display();
  return 1;
}

/* Store the product name of a device in buf with max size n. Device numbered 1-4.
   Return NULL if anythng goes wrong. */
char *get_product_name(int device)
{
  char *prod_name;
  _kernel_oserror *err;
  err = _swix(MIDI_USBInfo,_IN(0)|_OUT(2),device,&prod_name);

  if (err != NULL) {
    return NULL;
  }

  return prod_name;
}

/* Parse the command and put a printable string in buf */
void parse_command(int command, char *buf, int buf_size)
{
  /* unpack the message */
  int status = (command & 0xFF); /* byte 0: status byte */
  int size = (command >> 24 & 3); /* bits 24-25 are size of command */
  int data1 = (command >> 8 & 0xFF); /* byte 1: data byte 1 */
  int data2 = (command >> 16 & 0xFF); /* byte 2: data byte 2 */
  int channel;

  if (status >= 0x80 && status <= 0xEF) { /* channel-specific */
    channel = status & 0x0F; /* lower nibble is channel */
    status = status & 0xF0; /* higher nibble command */
    switch (status) {
      case 0x80: /* Note Off */
      snprintf(buf,buf_size,"[Note Off] Note=%d Velocity=%d",data1,data2);
      break;
      case 0x90: /* Note On */
      snprintf(buf,buf_size,"[Note On] Note=%d Velocity=%d",data1,data2);
      break;
      case 0xA0: /* Aftertouch / Key Pressure */
      snprintf(buf,buf_size,"[Aftertouch] Key=%d Pressure=%d",data1,data2);
      break;
      case 0xB0: /* Controller Change */
      snprintf(buf,buf_size,"[Controller Change] Controller=%d Value=%d",
               data1,data2);
      break;
      case 0xC0: /* Program Change */
      snprintf(buf,buf_size,"[Program Change] Program=%d",data1);
      break;
      case 0xD0: /* Channel Pressure */
      snprintf(buf,buf_size,"[Channel Pressure] Pressure=%d",data1);
      break;
      case 0xE0: /* Pitch Bend */
      snprintf(buf,buf_size,"[Pitch Bend] LSB=%x MSB=%x",data1,data2);
      break;
      default:
      snprintf(buf,buf_size,"Unknown command");
      break;
    }
  }
  else { /* Not channel-specific */
    switch (status) {
      case 0xF0:
      snprintf(buf,buf_size,"[System Exclusive] %x %x",data1,data2);
      break;
      case 0xF1:
      snprintf(buf,buf_size,"MTC Quarter Frame: %x %x",data1,data2);
      break;
      case 0xF2:
      snprintf(buf,buf_size,"[Song Position] LSB=%x MSB=%x",data1,data2);
      break;
      case 0xF3:
      snprintf(buf,buf_size,"[Song Select] Song=%d",data1);
      break;
      case 0xF5: /* Bus Select: nonstandard, vendor-specific */
      /* This may be removed if it can't be tested */
      snprintf(buf,buf_size,"[Bus Select] Bus=%d",data1);
      break;
      case 0xF6:
      snprintf(buf,buf_size,"[Tune Request]");
      break;
      case 0xF7:
      snprintf(buf,buf_size,"[System Exclusive End]");
      break;
      case 0xF8:
      snprintf(buf,buf_size,"Clock");
      break;
      case 0xFA:
      snprintf(buf,buf_size,"Start");
      break;
      case 0xFB:
      snprintf(buf,buf_size,"Continue");
      break;
      case 0xFC:
      snprintf(buf,buf_size,"Stop");
      break;
      case 0xFE:
      snprintf(buf,buf_size,"Active Sensing");
      break;
      case 0xFF:
      snprintf(buf,buf_size,"System Reset");
      break;
      default:
      snprintf(buf,buf_size,"Unknown command");
      break;
    }
  }

}


