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
 * File: main.c
 * Author: Lauren Rad
 * Purpose: Main Wimp code for !Midimon.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wimp.h"
#include "toolbox.h"
#include "saveas.h"
#include "iconbar.h"
#include "event.h"
#include "menu.h"
#include "wimplib.h"
#include "kernel.h"
#include "swis.h"

/* RISC_OSLib headers */
#include "msgs.h"
#include "msgtrans.h"
#include "werr.h"

/* MidiMon headers */

#include "preporter.h"
#include "common.h"
#include "midi.h"
#include "monitorwin.h"
#include "choiceswin.h"
#include "pianowin.h"
#include "messageswin.h"
#include "songwin.h"
#include "iconbar.h"

#define WimpVersion	310

/* Globals */
Choices global_choices;
int device_num = -1; /* device number, numbered 0-3 */

/* Static Globals */

static WimpPollBlock poll_block;
static MessagesFD messages;
static IdBlock id_block;
static int quit = 0;

/* Functions */
void register_handlers(void);
int tbox_error_handler(int event_code, ToolboxEvent *event,
    	       	       IdBlock *id_block, void *handle);

/***
Event handler for quit events from the iconbar (as Toolbox event 1)
***/
int quit_event(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  quit = 1;
  return 1;
}

/***
Message handler for Wimp Quit or PreQuit messages
***/
int quit_message(WimpMessage *message, void *handle)
{
  quit = 1;
  return 1;
}

/* Show help file */
int show_help(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  system("Filer_Run <MidiMon$Dir>.!Help"); /* Open help file */

  return 1;
}

/* Toolbox Error handler */
int tbox_error_handler(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  ToolboxErrorEvent *e = (ToolboxErrorEvent *)event;

#ifdef REPORTER_DEBUG
  report_printf("MidiMon: A toolbox error occurred: %d %s",e->errnum,e->errmess);
#endif

  return 1;
}

/* Register event and message handlers */
void register_handlers(void)
{
  /* Generic Toolbox events */
  event_register_toolbox_handler(-1,1,quit_event,NULL);
  event_register_toolbox_handler(-1,SaveAs_SaveToFile,save_log_text,NULL);
  event_register_toolbox_handler(-1,Toolbox_Error,tbox_error_handler,NULL);

  /* Toolbox events - window shown */
  event_register_toolbox_handler(-1,Event_Windows_ShowChoices,window_choices_onshow,NULL);
  event_register_toolbox_handler(-1,Event_Windows_ShowMonitor,window_monitor_onshow,NULL);
  event_register_toolbox_handler(-1,Event_Windows_ShowPiano,window_piano_onshow,NULL);
  event_register_toolbox_handler(-1,Event_Windows_ShowMessages,window_messages_onshow,NULL);
  event_register_toolbox_handler(-1,Event_Windows_ShowSong,window_song_onshow,NULL);

  /* Toolbox events - Send Messages window */
  event_register_toolbox_handler(-1,Event_Msg_SendProgChg,button_progchgsend,NULL);
  event_register_toolbox_handler(-1,Event_Msg_TxCntrlChg,button_txcntrlchg,NULL);
  event_register_toolbox_handler(-1,Event_Msg_TxTuneReq,button_txtunereq,NULL);
  event_register_toolbox_handler(-1,Event_Msg_TxSysReset,button_txsysreset,NULL);
  event_register_toolbox_handler(-1,StringSet_ValueChanged,stringset_programchg,NULL);

  /* Toolbox events - Song Control window */
  event_register_toolbox_handler(-1,Event_Song_Start,button_songstart,NULL);
  event_register_toolbox_handler(-1,Event_Song_Continue,button_songcontinue,NULL);
  event_register_toolbox_handler(-1,Event_Song_Stop,button_songstop,NULL);
  event_register_toolbox_handler(-1,Event_Song_SendSongSel,button_sendsongsel,NULL);

  /* Toolbox events - Monitor window */
  event_register_toolbox_handler(-1,Event_Monitor_ClearLog,clear_scrolllist,NULL);
  event_register_toolbox_handler(-1,Event_Monitor_Test,test_button_click,NULL);

  /* Toolbox events - Choices window */
  event_register_toolbox_handler(-1,Event_Choices_Set,choices_set_button_click,NULL);
  event_register_toolbox_handler(-1,Event_Choices_Save,choices_save_button_click,NULL);
  event_register_toolbox_handler(-1,Event_Choices_Default,choices_default_button_click,NULL);
  event_register_toolbox_handler(-1,Event_Choices_Cancel,choices_cancel_button_click,NULL);

  /* Toolbox events - Piano window */
  event_register_toolbox_handler(-1,Slider_ValueChanged,slider_valuechange,NULL);

  /* Toolbox events - Iconbar */
  event_register_toolbox_handler(-1,Event_Iconbar_ShowHelp,show_help,NULL);
  event_register_toolbox_handler(-1,Event_Iconbar_DeviceSelect,device_selection,NULL);
  event_register_toolbox_handler(-1,Event_Iconbar_Panic,midi_panic,NULL);
  /* Since the device menu isn't created manually, this is probably easier than using the generic
  Menu_AboutToBeShown event, I think.*/
  event_register_toolbox_handler(-1,Event_Iconbar_ShowDevMenu,update_devices_menu,NULL);

  /* Wimp events */
  event_register_wimp_handler(-1,Wimp_EMouseClick,key_clicked,0);
  event_register_wimp_handler(-1,Wimp_EMouseClick,slider_snap,0);

  /* Wimp messages */
  event_register_message_handler(Wimp_MQuit,quit_message,0);
  event_register_message_handler(Wimp_MPreQuit,quit_message,0);
  event_register_message_handler(Message_KeyEvent,key_pressed,0);
  event_register_message_handler(Message_MIDIDataReceived,handle_incoming,0);
  event_register_message_handler(Message_MIDIError,midi_error,0);
  event_register_message_handler(Message_MIDIInit,midi_initialised,0);
  event_register_message_handler(Message_MIDIDying,midi_dying,0);
  event_register_message_handler(Message_MIDIDevConnect,midi_dev_connected,0);
  event_register_message_handler(Message_MIDIDevDisconnect,midi_dev_disconnected,0);
}

int main(void)
{
  int wimp_messages = 0; /* Receive all Wimp messages */
  int toolbox_events = 0; /* Receive all Toolbox events */
  int event_code; /* For event_poll */
  _kernel_oserror *err;

  /* Initialise the Toolbox */
  toolbox_initialise(0, WimpVersion, &wimp_messages, &toolbox_events, "<Midimon$Dir>", &messages, &id_block, 0, 0, 0);

  /* Initialise the event library */
  event_initialise(&id_block);

  /* Set poll mask. Currently masking out:
     - Null_Reason_Code
     - Pointer_Leaving_Window
     - Pointer_Entering_Window
     - Lose_Caret
     - Gain_Caret */
  event_set_mask(0x1831);

  register_handlers(); /* Register event and message handlers */

  if(load_choices() != 0) {
    /* Something went seriously wrong -- currently this would mean
    Choices$Path isn't set properly */
    exit(EXIT_FAILURE); /* TBD cleanup? */
  }

  /* Before entering poll loop, empty the MIDI Rx buffer so new rx messages
     are triggered, as they only trigger on an empty buf.
     Also check that a MIDI module is actually loaded. Originally, this was
     the only place where MIDI SWIs would be called other than in response
     to events, so it doesn't quit, but that's less true with the
     device scanning. But it may still need to improve its handling of
     this in the event the MIDI module dies while the app is running. */
  err = _swix(MIDI_USBInfo,_IN(0),0);
  if (err != NULL && err->errnum == Error_SWINotKnown) {
    werr(1,"No MIDI module loaded!");
  }
  else {
    /* Initial device setup.
     This should be able to use the new features of USB-MIDI 0.08 to
     get the actual device names, let me check */
     int device_count;
     char *dev_name; /* Pointer to device name string, eg "USB8" */
     char *prod_name; /* Pointer to product name string */
     _swi(MIDI_USBInfo,_IN(0)|_OUT(0),0,&device_count);
     report_printf("MidiMon: MIDI Devices Connected: %d",device_count);
     if (device_count > 0) {
       device_num = 0; /* Default to the 1st device. */
       /* Beware! MIDI_USBInfo numbers 1-4, not 0-3. */
       for (int i = 1; i <= device_count; i++) {
         report_printf("  Device %d: %s",i,get_product_name(i));

        /*clear_rx_buf(i-1);*/ /* Clear the buffer for the device */
       }
       clear_rx_buf(1); /* hardcode to 1 for now due to my own hardware
       	     	    	   issues */
     }
  }

  /* Make choices take effect - this calls some MIDI SWIs so it needs to happen after the previous
     check. */
  action_choices(&global_choices);

  /* Begin poll loop */
  while (!quit)
  {
    event_poll(&event_code, &poll_block, 0);
  }

  exit(EXIT_SUCCESS);

}
