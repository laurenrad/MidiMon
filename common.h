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

/* Events */

/* Events - Window show events */
#define Event_Windows_ShowMonitor	0xF10 /* Main window is shown */
#define Event_Windows_ShowSong		0xF11 /* Song Control window is shown */
#define Event_Windows_ShowChoices	0xF12 /* Choices window is shown */
#define Event_Windows_ShowPiano		0xF13 /* Piano window is shown */
#define Event_Windows_ShowMessages	0xF14 /* Messages window is shown */

/* Events - Send Messages window */
#define Event_Msg_SendProgChg	   	0xFA1 /* Tx Program Change */
#define Event_Msg_TxCntrlChg		0xFA2 /* Tx Control Change */
#define Event_Msg_TxTuneReq		0xFA3 /* Tx Tune Request */
#define Event_Msg_TxSysReset		0xFA4 /* Tx System Reset */

/* Events - Monitor window */
#define Event_Monitor_ClearLog		0xFB0 /* Clear the monitor log */
#define Event_Monitor_Test		0xFBF /* Debug - monitor test button */

/* Events - Song Control window */
#define Event_Song_Start 	  	0xFC0  /* Song start button */
#define Event_Song_Continue		0xFC1  /* Song continue button */
#define Event_Song_Stop			0xFC2  /* Song stop button */
#define Event_Song_SendSongSel		0xFC3  /* Send song select button */

/* Events - Choices window */
#define Event_Choices_Set		0xFD0 /* Choices Set button */
#define Event_Choices_Default		0xFD1 /* Choices Default button */
#define Event_Choices_Save		0xFD2 /* Choices Save button */
#define Event_Choices_Cancel		0xFD3 /* Choices Cancel button */

/* Events - Iconbar */
#define Event_Iconbar_ShowDevMenu	0xFE0  /* Iconbar menu: show devices menu */
#define Event_Iconbar_DeviceSelect	0xFE1 /* Device menu selection */
#define Event_Iconbar_ShowHelp		0xFE2 /* Iconbar menu: Help... */
#define Event_Iconbar_Panic		0xFE3 /* Reset MIDI module status */

/* Messages */
/* These are now allocated and registered with ROOL. */
#define Message_MIDIDataReceived	0x5A4C0 /* MIDI data received */
#define Message_MIDIError		0x5A4C1 /* MIDI error */
#define Message_MIDIDevConnect		0x5A4C2 /* A USB MIDI device was connected */
#define Message_MIDIDevDisconnect	0x5A4C3 /* A USB MIDI device was disconnected */
#define Message_MIDIInit		0x5A4C4 /* USB MIDI module has initialised */
#define Message_MIDIDying		0x5A4C5 /* USB MIDI module is dying */
#define Message_KeyEvent		0x5A500 /* Key up events */

/* Errors */
#define Error_SWINotKnown		0x1E6

/* Gadget component IDs - note that most labels will actually be buttons as
   label text can't be changed at runtime. */

/* Gadgets: Choices window gadgets */
#define Gadget_Choices_TxChan		0x00 /* tx channel number range */
#define Gadget_Choices_TxChanLabel	0x01 /* label: "Tx Channel" (actually a button) */
#define Gadget_Choices_AltNoteOff	0x02 /* alt note off option button */
#define Gadget_Choices_IgnoreClock	0x03 /* ignore clock option button */
#define Gadget_Choices_FakeFastClock	0x04 /* fake fast clock option button */
#define Gadget_Choices_DefaultButton	0x05 /* default action button */
#define Gadget_Choices_SaveButton	0x06 /* save action button */
#define Gadget_Choices_CancelButton	0x07 /* cancel action button */
#define Gadget_Choices_SetButton	0x08 /* set action button */

/* Gadgets: Monitor window gadgets */
#define Gadget_Monitor_ScrollList       0x00
#define Gadget_Monitor_DeviceDisplay	0x04
#define Gadget_Monitor_DeviceLabel	0x05

/* Gadgets: Piano window gadgets */
#define Gadget_Keys_C1			0x00
#define Gadget_Keys_Db1			0x01
#define Gadget_Keys_D1			0x02
#define Gadget_Keys_Eb1			0x03
#define Gadget_Keys_E1			0x04
#define Gadget_Keys_F1			0x05
#define Gadget_Keys_Gb1			0x06
#define Gadget_Keys_G1			0x07
#define Gadget_Keys_Ab1			0x08
#define Gadget_Keys_A1			0x09
#define Gadget_Keys_Bb1			0x0A
#define Gadget_Keys_B1			0x0B
#define Gadget_Keys_C2			0x0C
#define Gadget_Keys_Db2			0x0D
#define Gadget_Keys_D2			0x0E
#define Gadget_Keys_Eb2			0x0F
#define Gadget_Keys_E2			0x10
#define Gadget_Keys_F2			0x11
#define Gadget_Keys_Gb2			0x12
#define Gadget_Keys_G2			0x13
#define Gadget_Keys_Ab2			0x14
#define Gadget_Keys_A2			0x15
#define Gadget_Keys_Bb2			0x16
#define Gadget_Keys_B2			0x17

/* Gadgets - Piano (Horizontal Toolbar) */
#define Gadget_Piano_TypeNotes		0x04
#define Gadget_Piano_Vel		0x05
#define Gadget_Piano_Oct		0x01
#define Gadget_Piano_VelLabel		0x03
#define Gadget_Piano_OctLabel		0x02
/* Gadgets - Piano (Vertical Toolbar) */
#define Gadget_Piano_PitchBend		0x00
#define Gadget_Piano_PitchLabel		0x01

/* Gadgets: Send Messages window */
#define Gadget_Msg_ProgChgStr		0x02 /* StringSet: GM program names */
#define Gadget_Msg_ProgChgSend		0x07
#define Gadget_Msg_ProgChgNum		0x13
#define Gadget_Msg_CntlrLabel		0x0A /* Label: "Controller" (actually a button) */
#define Gadget_Msg_ValueLabel		0x10 /* Label: "Value" (actually a button) */
#define Gadget_Msg_CtrlChgSend		0x0B /* control change send button */
#define Gadget_Msg_CtrlChgCntlr		0x0C
#define Gadget_Msg_CtrlChgVal		0x0D
#define Gadget_Msg_CtrlChgLblBox	0x18 /* Control change box label (actually a button) */
#define Gadget_Msg_ProgChgLblBox	0x19 /* Program change box label (actually a button) */
#define Gadget_Msg_TuneReq		0x09 /* Send tune request button */
#define Gadget_Msg_SysReset		0x15 /* Send system reset button */
#define Gadget_Msg_TuneReqLabel		0x16 /* Label: "Tune Request" (actually a button) */
#define Gadget_Msg_SysResetLabel	0x17 /* Label: "System Reset" (actually a button) */
#define Gadget_Msg_MiscLblBox		0x1A /* Label: "Misc" (actually a button) */

/* Gadgets: Song Control window */
#define Gadget_Song_Start	  	0x00 /* Start button */
#define Gadget_Song_Continue		0x01 /* Continue button */
#define Gadget_Song_Stop		0x02 /* Stop button */
#define Gadget_Song_SongNumLabel	0x03 /* Song Select label text */
#define Gadget_Song_SongNum	  	0x04 /* Number range for song select */
#define Gadget_Song_SongSelSend		0x05 /* Song select send button */

/* Misc other constants */
#define MaxLine		  		256 /* maximum list line len */
#define ProdNameMaxLen			50  /* maximum product name len */

#endif
