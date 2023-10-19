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
  * Purpose: Code for interacting with the USBMIDI module (Header)
  */

#ifndef MIDIMON_MIDI_H
#define MIDIMON_MIDI_H

int device_count(void);          // return number of available MIDI devices
int clear_rx_buf(void);    // clear the rx buf
int read_rx_command(int device); // handle new incoming message
int tx_noteon(int note, int velocity, int oct_shift);   // send note on
int tx_noteoff(int note, int velocity, int oct_shift);  // send note off
void tx_progchg(int prog);       // send program change
void tx_controlchg(int control, int value);     // send control change
void tx_songstart(void);         // send song start
void tx_songcontinue(void);      // send song continue
void tx_songstop(void);          // send song stop
void tx_songsel(int num);        // send song select
void tx_tunereq(void);           // tx tune request command
void tx_sysreset(void);          // tx system reset command
void tx_pitchwheel(int pitch);   // tx pitchwheel command
void ignore_timing(int option);  // turn timing clock messages on/off
void reset_midi(void);           // reset the MIDI module status
int fake_fast_clock(int option); // set fake fast clock option
int set_tx_channel(int device, int channel);    // set tx channel
int midi_error(WimpMessage *message, void *handle);    // handle MIDI error message
int midi_dying(WimpMessage *message, void *handle);    // handle MIDI dying msg
int midi_dev_connected(WimpMessage *message, void *handle);    // handle device connect msg
int midi_dev_disconnected(WimpMessage *message, void *handle); // handle device disconnect msg
char *get_product_name(int device);     // get the product name of a device
void parse_command(int command, char *buf, int buf_size);       // printable command
int midi_incoming(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);

/*
 * USBMidi SWI numbers
 */
#define MIDI_SoundEnable 	       	0x404C0
#define MIDI_SetMode		  	0x404C1
#define MIDI_SetTxChannel	  	0x404C2
#define MIDI_SetTxActiveSensing	  	0x404C3
#define MIDI_InqSongPositionPointer	0x404C4
#define MIDI_InqBufferSize		0x404C5
#define MIDI_InqError			0x404C6
#define MIDI_RxByte			0x404C7
#define MIDI_RxCommand			0x404C8
#define MIDI_TxByte			0x404C9
#define MIDI_TxCommand			0x404CA
#define MIDI_TxNoteOff			0x404CB
#define MIDI_TxNoteOn			0x404CC
#define MIDI_TxPolyKeyPressure		0x404CD
#define MIDI_TxControlChange		0x404CE
#define MIDI_TxLocalControl		0x404CF
#define MIDI_TxAllNotesOff		0x404D0
#define MIDI_TxOmniModeOff		0x404D1
#define MIDI_TxOmniModeOn		0x404D2
#define MIDI_TxMonoModeOn		0x404D3
#define MIDI_TxPolyModeOn		0x404D4
#define MIDI_TxProgramChange		0x404D5
#define MIDI_TxChannelPressure		0x404D6
#define MIDI_TxPitchWheel		0x404D7
#define MIDI_TxSongPositionPointer	0x404D8
#define MIDI_TxSongSelect		0x404D9
#define MIDI_TxTuneRequest		0x404DA
#define MIDI_TxStart			0x404DB
#define MIDI_TxContinue			0x404DC
#define MIDI_TxStop			0x404DD
#define MIDI_TxSystemReset		0x404DE
#define MIDI_IgnoreTiming		0x404DF
#define MIDI_TxSynchSoundScheduler	0x404E0
#define MIDI_FastClock			0x404E1
#define MIDI_Init			0x404E2
#define MIDI_SetBufferSize		0x404E3
#define MIDI_Interface			0x404E4 /* DO NOT CALL */
#define MIDI_USBInfo			0x404EA
#define MIDI_Options			0x404EB

#endif
