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
 * File: messageswin.c
 * Author: Lauren Rad
 * Purpose: Handlers for the messages window.
*/


#include <stdbool.h>
#include <stdlib.h>

#include "toolbox.h"
#include "gadgets.h"
#include "window.h"
#include "wimp.h"
#include "msgs.h" // RISC_OSLib
#include "msgtrans.h" //RISC_OSLib

#include "messageswin.h"
#include "common.h"
#include "midi.h"
#include "preporter.h"

/* Globals */
static ObjectId window_id_messages = -1;
static bool messages_opened = false; /* Track if we know the window ID yet */

/*
 * window_messages_onshow
 * This handler is called when the Send Messages window is shown.
 * This does any first-time setup that may be needed and saves its ObjectId.
 */
int window_messages_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  if (messages_opened == false) {
    messages_opened = true;
    window_id_messages = id_block->self_id;
    load_messages_messageswin();
  }

  return 1;
}

/* Handle StringSet_ValueChanged events for the Program StringSet. */
int stringset_programchg(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  if (id_block->self_id == window_id_messages && id_block->self_component == Gadget_Msg_ProgChgStr) {
    int index;
    /* The event only gives the string, not the index, so call stringset_get_selected for that */
    stringset_get_selected(1,window_id_messages,Gadget_Msg_ProgChgStr,&index);

    /* Set the number range based on the StringSet. This will be read when sending the message. */
    numberrange_set_value(0,window_id_messages,Gadget_Msg_ProgChgNum,index);
  }

  return 1;
}

/* Handle the Send button for Program Change messages.
   If this is called, we can safely assume that the window ID is known,
   so no need to check.
*/
int button_progchgsend(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  int prog = 0;

  numberrange_get_value(0,window_id_messages,Gadget_Msg_ProgChgNum,&prog);

  tx_progchg(prog);

  return 1;
}

/* Handle the Send button for Control Change messages.
   If this is called, we can safely assume that the window ID is known, so no need to check.
*/
int button_txcntrlchg(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  int control = 0; /* Controller number */
  int value = 0; /* Value to send */

  numberrange_get_value(0,window_id_messages,Gadget_Msg_CtrlChgCntlr,&control);
  numberrange_get_value(0,window_id_messages,Gadget_Msg_CtrlChgVal,&value);

  tx_controlchg(control, value);

  return 1;
}

/* Button handler for the Send button for tune request command. */
int button_txtunereq(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  tx_tunereq();
  return 1;
}

/* Button handler for the Send button for system reset command. */
int button_txsysreset(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  tx_sysreset();
  return 1;
}

/* Load Messages with MessageTrans. This currently does not handle the GM patch names.
   There are some duplicate strings in the autogen'd Messages file which will not be used
   here and should probably be removed later.
*/
void load_messages_messageswin(void)
{
  /* Debug */
  _kernel_oserror *err;

  /* Load Messages file and save pointer to control block */
  msgs_init(); /* msgs_readfile(s) just ignores parameter and calls msgs_init()... */
  msgtrans_control_block *cb;
  cb = msgs_main_control_block();

  /* Window and gadget text */
  err = window_set_title(0,window_id_messages,msgs_lookup("Messages|1:err")); /* window title */
  actionbutton_set_text(0,window_id_messages,Gadget_Msg_TuneReq,
  	       	   	msgs_lookup("Messages|8:err")); /* Tune Req send button */
  actionbutton_set_text(0,window_id_messages,Gadget_Msg_SysReset,
  	       	   	msgs_lookup("Messages|8:err")); /* Sys Reset send button */
  actionbutton_set_text(0,window_id_messages,Gadget_Msg_ProgChgSend,
  	       	   	msgs_lookup("Messages|8:err")); /* Prog Chg send button */
  actionbutton_set_text(0,window_id_messages,Gadget_Msg_CtrlChgSend,
  	       	   	msgs_lookup("Messages|8:err")); /* ctrl chg send button */
  button_set_value(0,window_id_messages,Gadget_Msg_CntlrLabel,
  	     	     	msgs_lookup("Messages|20:err")); /* label: "Controller" */
  button_set_value(0,window_id_messages,Gadget_Msg_ValueLabel,
  	     	     	msgs_lookup("Messages|22:err")); /* label: "Value" */
  button_set_value(0,window_id_messages,Gadget_Msg_TuneReqLabel,
  	     	     	msgs_lookup("Messages|10:err")); /* label: "Tune Request" */
  button_set_value(0,window_id_messages,Gadget_Msg_SysResetLabel,
  	     	     	msgs_lookup("Messages|29:err")); /* label: "System Reset" */
  /* Here are the buttons I've used to replace the unchangable label box text. This still isn't
     ideal as the spacing on the label box will be static, but it will do for now. */
  button_set_value(0,window_id_messages,Gadget_Msg_CtrlChgLblBox,
  	     	   msgs_lookup("Messages|18:err")); /* label: "Control Change" */
  button_set_value(0,window_id_messages,Gadget_Msg_ProgChgLblBox,
  	     	   msgs_lookup("Messages|24:err")); /* label: "Program Change" */
  button_set_value(0,window_id_messages,Gadget_Msg_MiscLblBox,
  	     	   msgs_lookup("Messages|26:err")); /* label: "Misc" */
  /* TODO: how to handle default string set text? */
  stringset_set_available(0,window_id_messages,Gadget_Msg_ProgChgStr,
  	    		  msgs_lookup("Messages|31:Unable to read Messages file."));

  /* Help text */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_ProgChgStr,
  	     	  	  msgs_lookup("Messages|2:Unable to get help.")); /* prog string set */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_ProgChgSend,
  	     	  	  msgs_lookup("Messages|7:Unable to get help.")); /* prog chg send */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_CtrlChgSend,
  	     	  	  msgs_lookup("Messages|13:Unable to get help.")); /* ctrl chg send */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_TuneReq,
  	     	  	  msgs_lookup("Messages|11:Unable to get help.")); /* tune req send */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_SysReset,
  	     	  	  msgs_lookup("Messages|30:Unable to get help.")); /* sys reset send */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_CtrlChgCntlr,
  	     	  	  msgs_lookup("Messages|15:Unable to get help.")); /* controller num */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_CtrlChgVal,
  	     	  	  msgs_lookup("Messages|16:Unable to get help.")); /* controller val */
  gadget_set_help_message(0,window_id_messages,Gadget_Msg_ProgChgNum,
  	     	  	  msgs_lookup("Messages|27:Unable to get help.")); /* program num */

  if (err != NULL) {
    report_printf("MidiMon: err in load_messages_messagewin - %d: %s",err->errnum,err->errmess);
  }

  msgtrans_close_file(cb); /* close Messages file */
}
