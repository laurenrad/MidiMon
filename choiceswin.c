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
 * File: choiceswin.c
 * Author: Lauren Rad
 * Purpose: Wimp handlers for Choices window and associated helpers.
 */

/* Library headers */
#include <stdbool.h>

/* System headers */
#include "kernel.h"
#include "swis.h"

/* Toolbox headers */
#include "event.h"
#include "toolbox.h"
#include "gadgets.h"
#include "window.h"

/* RISC_OSLib headers for MessageTrans lookup */
#include "msgs.h"
#include "msgtrans.h"

/* MidiMon headers */
#include "choices.h"
#include "preporter.h"
#include "common.h"
#include "choiceswin.h"
#include "midi.h"

/* Globals */
static ObjectId window_id_choices;
static bool choices_opened = false;

/* Called when the choices window is shown. */
int window_choices_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  /* On the first time the choices window is shown, save ObjectId an update messages. */
  if (!choices_opened) {
    choices_opened = true;
    window_id_choices = id_block->self_id;
    load_messages_choiceswin();
  }

  /* Update gadgets with the set choices */
  refresh_gadgets(global_choices, id_block);

  return 1;
}

/* Handler for 'Save' button in Choices dialog */
int choices_save_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  store_gadgets(&global_choices, id_block);

  if(save_choices() != 0) {
    report_printf("MidiMon: Error writing out Choices file");
    exit(EXIT_FAILURE);
  }

  action_choices(&global_choices);

  return 1;
}

/* Handler for 'Set' button in Choices dialog */
int choices_set_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  store_gadgets(&global_choices, id_block);

  action_choices(&global_choices);

  return 1;
}

/* Handler for 'Default' button click in Choices window */
/* According to the Style guide, this should reset the gadgets to defaults AND
make the defaults active. */
int choices_default_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  global_choices = init_choices(); /* set active choices to defaults */

  refresh_gadgets(global_choices, id_block); /* reset the gadgets */

  return 1;
}

/* Handler for 'Cancel' button click in Choices window */
int choices_cancel_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  refresh_gadgets(global_choices, id_block); /* just set the gadgets back to whatever is stored */

  return 1;
}

/* Refresh the gadgets to reflect the Choices instance, used by a couple
handlers */
void refresh_gadgets(Choices c, IdBlock *id_block)
{
  numberrange_set_value(0,id_block->self_id,Gadget_Choices_TxChan,c.opt_txchan);
  optionbutton_set_state(0,id_block->self_id,Gadget_Choices_AltNoteOff,c.opt_altnoteoff);
  optionbutton_set_state(0,id_block->self_id,Gadget_Choices_IgnoreClock,c.opt_ignoreclock);
  optionbutton_set_state(0,id_block->self_id,Gadget_Choices_FakeFastClock,c.opt_fakefastclock);
}

/* Store the state of the gadgets to the Choices instance */
void store_gadgets(Choices *c, IdBlock *id_block)
{
  numberrange_get_value(0,id_block->self_id,Gadget_Choices_TxChan,&(c->opt_txchan));
  optionbutton_get_state(0,id_block->self_id,Gadget_Choices_AltNoteOff,&(c->opt_altnoteoff));
  optionbutton_get_state(0,id_block->self_id,Gadget_Choices_IgnoreClock,&(c->opt_ignoreclock));
  optionbutton_get_state(0,id_block->self_id,Gadget_Choices_FakeFastClock,&(c->opt_fakefastclock));
}

/* Debug: print out choices to reporter. This isn't currently called */
void debug_print_choices(Choices *const c)
{
  report_printf("Choices values:");
  report_printf("Version:\t%d",c->choices_ver);
  report_printf("Tx Channel:\t%d",c->opt_txchan);
  report_printf("Alt Note Off:\t%d",c->opt_altnoteoff);
  report_printf("Ignore Clock:\t%d",c->opt_ignoreclock);
  report_printf("Fake Fast Clock:\t%d",c->opt_fakefastclock);
}

/* Take action on choices after they're loaded or set. This only applies to choices where
   and immediate action is needed rather than being read later, like when a SWI is needed
   to set an option with the MIDI module.
*/
void action_choices(Choices *const c)
{
 int new_channel = set_tx_channel(device_num,c->opt_txchan);
 ignore_timing(c->opt_ignoreclock);
 fake_fast_clock(c->opt_fakefastclock);
}

/* Look up messages with MessageTrans */
void load_messages_choiceswin(void)
{
  /* Debug */
  _kernel_oserror *err;

  /* Load Messages file and save pointer to control block */
  msgs_init();
  msgtrans_control_block *cb;
  cb = msgs_main_control_block();

  /* Set window and gadget text */
  err = window_set_title(0,window_id_choices,msgs_lookup("Choices|1:Choices"));
  button_set_value(0,window_id_choices,Gadget_Choices_TxChanLabel,
  	     	   msgs_lookup("Choices|4:Tx Channel"));
  actionbutton_set_text(0,window_id_choices,Gadget_Choices_DefaultButton,
  	       	   msgs_lookup("Choices|12:Default"));
  actionbutton_set_text(0,window_id_choices,Gadget_Choices_SaveButton,
  	       	   msgs_lookup("Choices|14:Save"));
  actionbutton_set_text(0,window_id_choices,Gadget_Choices_CancelButton,
  	       	   msgs_lookup("Choices|16:Cancel"));
  actionbutton_set_text(0,window_id_choices,Gadget_Choices_SetButton,
  	       	   msgs_lookup("Choices|18:Set"));
  optionbutton_set_label(0,window_id_choices,Gadget_Choices_AltNoteOff,
  	       	   msgs_lookup("Choices|6:Zero Velocity Note Off"));
  optionbutton_set_label(0,window_id_choices,Gadget_Choices_IgnoreClock,
  	       	   msgs_lookup("Choices|8:Ignore Clock Messages"));
  optionbutton_set_label(0,window_id_choices,Gadget_Choices_FakeFastClock,
  	       	   msgs_lookup("Choices|10:Fake Fast Clock"));

  /* Set help text */
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_TxChan,
  	     	  	  msgs_lookup("Choices|2:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_AltNoteOff,
  	     	  	  msgs_lookup("Choices|5:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_IgnoreClock,
  	     	  	  msgs_lookup("Choices|7:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_FakeFastClock,
  	     	  	  msgs_lookup("Choices|9:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_DefaultButton,
  	     	  	  msgs_lookup("Choices|11:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_SaveButton,
  	     	  	  msgs_lookup("Choices|13:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_CancelButton,
  	     	  	  msgs_lookup("Choices|15:Unable to get help."));
  gadget_set_help_message(0,window_id_choices,Gadget_Choices_SetButton,
  	     	  	  msgs_lookup("Choices|17:Unable to get help."));

  if (err != NULL) {
    report_printf("MidiMon: err: in load_messages_choiceswin - %d %s",err->errnum,err->errmess);
  }

  msgtrans_close_file(cb); /* close Messages file */
}
