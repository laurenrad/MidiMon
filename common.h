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
 * Author: Lauren Rad
 * File: common.h
 * Description: Main shared header for Midimon
*/

#ifndef MIDIMON_COMMON_H
#define MIDIMON_COMMON_H

#include "choices.h"

extern Choices global_choices;
extern int device_num;

/*
 * Constants for event numbers.
 * These are defined in the res file, so they are all collected in one place
 * here to watch out for any clashes.
 */

/* Events - Window show */
#define Event_Windows_ShowMonitor	0xF10   // Main window is shown
#define Event_Windows_ShowSong		0xF11   // Song Control window is shown
#define Event_Windows_ShowChoices	0xF12   // Choices window is shown
#define Event_Windows_ShowPiano		0xF13   // Piano window is shown
#define Event_Windows_ShowMessages	0xF14   // Messages window is shown

/* Events - Send Messages window */
#define Event_Msg_SendProgChg	   	0xFA1   // Tx Program Change
#define Event_Msg_TxCntrlChg		0xFA2   // Tx Control Change
#define Event_Msg_TxTuneReq		0xFA3   // Tx Tune Request
#define Event_Msg_TxSysReset		0xFA4   // Tx System Reset

/* Events - Monitor window */
#define Event_Monitor_ClearLog		0xFB0   // Clear the monitor log
#define Event_Monitor_Test		0xFBF   // Debug - monitor test button

/* Events - Song Control window */
#define Event_Song_Start 	  	0xFC0   // Song start button
#define Event_Song_Continue		0xFC1   // Song continue button
#define Event_Song_Stop			0xFC2   // Song stop button
#define Event_Song_SendSongSel		0xFC3   // Send song select button

/* Events - Choices window */
#define Event_Choices_Set		0xFD0   // Choices Set button
#define Event_Choices_Default		0xFD1   // Choices Default button
#define Event_Choices_Save		0xFD2   // Choices Save button
#define Event_Choices_Cancel		0xFD3   // Choices Cancel button

/* Events - Iconbar */
#define Event_Iconbar_ShowDevMenu	0xFE0   // Iconbar menu: show devices menu
#define Event_Iconbar_DeviceSelect	0xFE1   // Device menu selection
#define Event_Iconbar_ShowHelp		0xFE2   // Iconbar menu: Help...
#define Event_Iconbar_Panic		0xFE3   // Reset MIDI module status

/*
 * Messages - allocated and registered with ROOL.
 */
#define Message_MIDIDataReceived	0x5A4C0 // MIDI data received
#define Message_MIDIError		0x5A4C1 // MIDI error
#define Message_MIDIDevConnect		0x5A4C2 // A USB MIDI device was connected
#define Message_MIDIDevDisconnect	0x5A4C3 // A USB MIDI device was disconnected
#define Message_MIDIInit		0x5A4C4 // USB MIDI module has initialised
#define Message_MIDIDying		0x5A4C5 // USB MIDI module is dying
#define Message_KeyEvent		0x5A500 // Key up events

/*
 * Errors
 */
#define Error_SWINotKnown		0x1E6

/* Misc other constants */
#define MaxLine		  		256     // maximum list line len
#define ProdNameMaxLen			50      // maximum product name len

#endif
