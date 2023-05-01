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
 * File: piano.c
 * Author: Lauren Rad
 * Purpose: Handlers for the piano controller window.
*/

/* Library includes */
#include <stdbool.h>

/* System headers */
#include "wimp.h"
#include "wimplib.h"
#include "kernel.h"

/* Toolbox headers */
#include "toolbox.h"
#include "gadgets.h"
#include "window.h"

/* RISC_OSLib headers for MessageTrans lookup */
#include "msgs.h"
#include "msgtrans.h"

/* MidiMon headers */
#include "common.h"
#include "pianowin.h"
#include "preporter.h"
#include "midi.h"

/* Local helper functions */
int hotkeys_enabled(void); /* returns 1 if hotkeys are enabled, 0 if disabled */
int has_caret(void); /* returns 1 if window has the caret, 0 otherwise */
int get_velocity(void); /* returns the set velocity */
int get_octave(void); /* returns the set octave shift */

/* Globals */
#define KEY_COUNT   24
#define BASE_NOTE   60 /* Note number of the lowest C on the piano */
static ObjectId window_id_piano; /* piano window's ObjectId */
static ObjectId tbar_id_h; /* horizontal toolbar's ObjectId */
static ObjectId tbar_id_v; /* vertical toolbar's ObjectId */
static bool piano_opened = false; /* Track if we know the window ID yet */
static int keys_pressed[KEY_COUNT]; /* keep track of what keys are down.
       	   		     	    this is hardcoded to the interface
       	   		     	    design, so watch out! */

/* Called when the piano window is shown. Save ObjectId and update messages. */
int window_piano_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  if (!piano_opened) {
     piano_opened = true;
     window_id_piano = id_block->self_id;

     /* Get the toolbar ids too. This can be retrieved from the window ID but it's done here
     	for convenience.*/
     window_get_tool_bars(0x09,window_id_piano,&tbar_id_h,NULL,NULL,&tbar_id_v);
     load_messages_pianowin();
  }

  return 1;
}

/* Handler for the ToolBox's Slider_ValueChanged event. This will fire for any instance of
   this event, but we'll filter for the pitch bend slider. This will allow it to both send pitch
   bend on value change and snap back to the center when released, as these controls typically do.*/
int slider_valuechange(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  if (id_block->self_component == Gadget_Piano_PitchBend) {
    int val;
    slider_get_value(0,window_id_piano,Gadget_Piano_PitchBend,&val);
    tx_pitchwheel(val);

    /*slider_set_value(0,window_id_piano,Gadget_Piano_PitchBend,8192);*/ /* 8192 is center value */
    /*report_printf("snapped slider");*/

  }

  return 1;
}

/* Handle KeyEvent messages. This is nonstandard but it allows a couple
   important things for this program:
   	     - Responding to key up events
   	     - Ignoring key repeat
   The disadvantages are that it is tied to the original QWERTY layout and
   it doesn't know or care about the caret; hopefully this can be improved
   later.
*/
int key_pressed(WimpMessage *message, void *handle)
{
  /* Only do something if the window has been opened yet */
  if (piano_opened == false) {
    return 1;
  }

  /* Also don't do anything if hotkeys are not enabled and we don't have the caret */
  if (!hotkeys_enabled() && !has_caret()) {
    return 1;
  }

  KeyUpData d = ((KeyUpMessage *)message)->key_data;

  ComponentId component = 0;
  int note = 0;
  int octave = get_octave();
  int velocity = get_velocity();

  /* Then, check for the special case of a mouse up event.
     This should be more elegant. */
  if (d.key_num == MOUSE_SELECT || d.key_num == MOUSE_ADJUST) {
    if (d.state == 0) {
      for (int i = 0; i < KEY_COUNT; i++) {
        if(keys_pressed[i] == 1) {
          keys_pressed[i] = 0;
          tx_noteoff(60+i,velocity,octave); /* offset from base note */
        }
      }
    }
    return 1; /* done here */
  }

  switch (d.key_num) {
    case KEY_Q:
    	 component = Gadget_Keys_C1;
    	 note = 60;
    break;
    case KEY_2:
    	 component = Gadget_Keys_Db1;
    	 note = 61;
    break;
    case KEY_W:
    	 component = Gadget_Keys_D1;
    	 note = 62;
    break;
    case KEY_3:
    	 component = Gadget_Keys_Eb1;
    	 note = 63;
    break;
    case KEY_E:
    	 component = Gadget_Keys_E1;
    	 note = 64;
    break;
    case KEY_R:
    	 component = Gadget_Keys_F1;
    	 note = 65;
    break;
    case KEY_5:
    	 component = Gadget_Keys_Gb1;
    	 note = 66;
    break;
    case KEY_T:
    	 component = Gadget_Keys_G1;
    	 note = 67;
    break;
    case KEY_6:
    	 component = Gadget_Keys_Ab1;
    	 note = 68;
    break;
    case KEY_Y:
    	 component = Gadget_Keys_A1;
    	 note = 69;
    break;
    case KEY_7:
    	 component = Gadget_Keys_Bb1;
    	 note = 70;
    break;
    case KEY_U:
    	 component = Gadget_Keys_B1;
    	 note = 71;
    break;
    case KEY_Z:
    	 component = Gadget_Keys_C2;
    	 note = 72;
    break;
    case KEY_S:
    	 component = Gadget_Keys_Db2;
    	 note = 73;
    break;
    case KEY_X:
    	 component = Gadget_Keys_D2;
    	 note = 74;
    break;
    case KEY_D:
    	 component = Gadget_Keys_Eb2;
    	 note = 75;
    break;
    case KEY_C:
    	 component = Gadget_Keys_E2;
    	 note = 76;
    break;
    case KEY_V:
    	 component = Gadget_Keys_F2;
    	 note = 77;
    break;
    case KEY_G:
    	 component = Gadget_Keys_Gb2;
    	 note = 78;
    break;
    case KEY_B:
    	 component = Gadget_Keys_G2;
    	 note = 79;
    break;
    case KEY_H:
    	 component = Gadget_Keys_Ab2;
    	 note = 80;
    break;
    case KEY_N:
    	 component = Gadget_Keys_A2;
    	 note = 81;
    break;
    case KEY_J:
    	 component = Gadget_Keys_Bb2;
    	 note = 82;
    break;
    case KEY_M:
    	 component = Gadget_Keys_B2;
    	 note = 83;
    break;
    default:
    	 return 1; /* unhandled key, just return */
    break;
  }

  if (d.state == 0) {
    /* key up */
    /* Clear bit 21 (selected bit) for the given button */
    button_set_flags(0,window_id_piano,component,0x200000,0x0);
    tx_noteoff(note,velocity,octave); /* send note off message */
  }
  else {
    /* key down */
    /* Set bit 21 (selected bit) for the given button */
    button_set_flags(0,window_id_piano,component,0x200000,0x200000);
    tx_noteon(note,velocity,octave); /* send note on message */
  }

  return 1;
}

/* Handle key clicks (mouse down only) on the piano keys.
   Once again, the big assumption here is the component IDs of the
   piano keys, so watch out, future me!

   Additionally, attempt to grab the caret if the window doesn't have it already.
*/
int key_clicked(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
  ObjectId window = id_block->self_id;
  ComponentId component = id_block->self_component;
  int window_handle_piano =-1;

  int note = 0;
  int octave = get_octave();
  int velocity = get_velocity();

  /* Only pay attention if the click involves the piano window, since
     this is actually a general handler for all Wimp button clicks */
  if ((piano_opened == true) && (window == window_id_piano)) {
    /* Attempt to gain caret */
    window_get_wimp_handle(0,window_id_piano,&window_handle_piano);
    wimp_set_caret_position(window_handle_piano,-1,0,0,-1,-1);

    if (component < KEY_COUNT && component > 0) { /* only look at piano key buttons */
      note = component + BASE_NOTE;
      keys_pressed[component] = 1; /* mark key as pressed for mouseup */
      tx_noteon(note,velocity,octave); /* send note on message */
    }
  }

  return 1;
}

/* Another handler for Wimp_EMouseClick; this one doesn't claim the event so that key_clicked
   can still handle. Is this safe? I'm not sure but it seems to work, and this is the less
   important of the two so it's the one that doesn't claim. The purpose of this is to make
   the pitch bend slider snap back to the center, as these controls usually do.

   Currently, the mechanism for this is to release the slider and adjust click on it to reset.
   This isn't ideal but while the slider is dragged, an adjust click won't be recognised,
   so this is the best I can do for now. Maybe a better solution later?
*/
int slider_snap(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
  WimpMouseClickEvent *m = (WimpMouseClickEvent *)event;

  /* Filter for the piano window's pitch bend, then filter for Adjust clicks */
  if (piano_opened && id_block->parent_id == window_id_piano
      && id_block->self_component == Gadget_Piano_PitchBend) {
    if (m->buttons == 1) {
      int current_val;
      slider_get_value(0,id_block->self_id,Gadget_Piano_PitchBend,&current_val);
      /* Only snap if it isn't already centered, or else nasty flickering will happen.
      	 Ok, nasty flickering still happens if you drag, but this at least eliminates it when
      	 the mouse is still. */
      if (current_val != 8192) {
        slider_set_value(0,id_block->self_id,Gadget_Piano_PitchBend,8192); /* 8192 = middle value */
      }
    }
  }

  return 0;
}

/* Load Messages with MessageTrans. This one is a bit different because all the gadgets are on
   toolbars. */
void load_messages_pianowin(void)
{
  /* Debug */
  _kernel_oserror *err;

  /* Load Messages file and save pointer to control block */
  msgs_init();
  msgtrans_control_block *cb;
  cb = msgs_main_control_block();

  /* Get ObjectIds of the toolbars */
  ObjectId toolbar_id_h, toolbar_id_v;
  window_get_tool_bars(0,window_id_piano,&toolbar_id_h,NULL,NULL,&toolbar_id_v);

  /* Set gadget and window text */
  err = window_set_title(0,window_id_piano,msgs_lookup("Piano|1:err")); /* window title */
  button_set_value(0,toolbar_id_h,Gadget_Piano_VelLabel,
  	     	   msgs_lookup("Piano|53:err")); /* piano velocity label */
  button_set_value(0,toolbar_id_h,Gadget_Piano_OctLabel,
  	     	   msgs_lookup("Piano|57:err")); /* piano octave label */
  optionbutton_set_label(0,toolbar_id_h,Gadget_Piano_TypeNotes,
  	       	   msgs_lookup("Piano|51:err")); /* type notes option */

  /* Set help text */
  gadget_set_help_message(0,toolbar_id_h,Gadget_Piano_TypeNotes,
  	     	  	  	msgs_lookup("Piano|50:Unable to get help."));
  gadget_set_help_message(0,toolbar_id_h,Gadget_Piano_Vel,
  	     	  	  msgs_lookup("Piano|54:Unable to get help."));
  gadget_set_help_message(0,toolbar_id_h,Gadget_Piano_Oct,
  	     	  	  msgs_lookup("Piano|55:Unable to get help."));

  if (err != NULL) {
    report_printf("MidiMon: err in load_messages_pianowin - %d: %s",err->errnum,err->errmess);
  }

  msgtrans_close_file(cb); /* close Messages file */
}

/* Local helper functions */

/* Return 1 if hotkeys are enabled, 0 if disabled */
int hotkeys_enabled(void)
{
  int state = 0;

  optionbutton_get_state(0,tbar_id_h,Gadget_Piano_TypeNotes,&state);

  return state;
}

/* Returns the velocity. If anything goes wrong, returns max velocity. */
int get_velocity(void)
{
  int velocity = 127;
  numberrange_get_value(0,tbar_id_h,Gadget_Piano_Vel,&velocity);
  return velocity;
}

/* Returns the octave. If anything goes wrong, returns 0. */
int get_octave(void)
{
  int octave = 0;
  numberrange_get_value(0,tbar_id_h,Gadget_Piano_Oct,&octave);
  return octave;
}

/* Returns 1 if piano window has the caret , 0 otherwise. */
int has_caret(void)
{
  _kernel_oserror *err = NULL;
  if (piano_opened) {
    WimpGetCaretPositionBlock *b; /* in wimp.h */
    int window_handle_piano =-1; /* Wimp handle of piano window */
    err = wimp_get_caret_position(b);
    window_get_wimp_handle(0,window_id_piano,&window_handle_piano);
    if (err != NULL) {
      report_printf("an error occurred in has_caret: %d %s",err->errnum,err->errmess);
    }
    else {
      if (b->window_handle == window_handle_piano) {
        return 1;
      }
    }
  }

  return 0;
}
