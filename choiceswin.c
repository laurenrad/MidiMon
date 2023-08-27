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

// Library stuff
#include <stdbool.h>

// RISC OS stuff
#include "kernel.h"
#include "swis.h"
#include "event.h"
#include "toolbox.h"
#include "gadgets.h"
#include "window.h"
#include "msgs.h" // This and msgtrans are RISC_OSLib stuff
#include "msgtrans.h"

// My stuff
#include "choices.h"
#include "preporter.h"
#include "common.h"
#include "choiceswin.h"
#include "midi.h"

// Globals
static ObjectId window_id_choices;
static bool choices_opened = false;

/*
 * window_choices_onshow
 * This handler is called when the choices window is shown. This does any first-time setup
 * for the window, and makes sure the window is in step with any choices changes that
 * may have happened while it was closed.
 */
int window_choices_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    if (!choices_opened) {
        choices_opened = true;
        window_id_choices = id_block->self_id; // save ObjectId for later use
        load_messages_choiceswin(); // load messages
    }
    refresh_gadgets(global_choices, id_block); // sync gadgets with choices

    return 1;
}

/*
 * choices_save_button_click
 * This handler is called when the 'Save' button is clicked. It both saves choices to disk
 * and causes the choices to take effect
 */
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

/*
 * choices_set_button_click
 * This handler is called when the 'Set' button is clicked. It causes the choices to take
 * effect without storing them to disk.
 */
int choices_set_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    store_gadgets(&global_choices, id_block);
    action_choices(&global_choices);

    return 1;
}

/*
 * choices_default_button_click
 * This handler is called when the 'Default' button is clicked.
 * According to the RISC OS Style Guide, this should reset the gadgets to defaults AND
 * make the defaults active.
 */
int choices_default_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block,
                                 void *handle)
{
    global_choices = init_choices(); // set active choices to defaults
    refresh_gadgets(global_choices, id_block);

    return 1;
}

/*
 * choices_cancel_button_click
 * This handler is called when the 'Cancel' button is clicked.
 * This discards all changes made.
 */
int choices_cancel_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block,
                                void *handle)
{
    refresh_gadgets(global_choices, id_block); // just set the gadgets back to whatever is stored

    return 1;
}

/*
 * refresh_gadgets
 * Update gadgets to reflect what is stored in choices.
 */
void refresh_gadgets(Choices c, IdBlock *id_block)
{
    numberrange_set_value(0, id_block->self_id, Gadget_Choices_TxChan, c.opt_txchan);
    optionbutton_set_state(0, id_block->self_id, Gadget_Choices_AltNoteOff, c.opt_altnoteoff);
    optionbutton_set_state(0, id_block->self_id, Gadget_Choices_IgnoreClock, c.opt_ignoreclock);
    optionbutton_set_state(0, id_block->self_id, Gadget_Choices_FakeFastClock, c.opt_fakefastclock);
}

/*
 * store_gadgets
 * Store the state of the gadgets to the given Choices struct.
 */
void store_gadgets(Choices *c, IdBlock *id_block)
{
    numberrange_get_value(0, id_block->self_id, Gadget_Choices_TxChan, &(c->opt_txchan));
    optionbutton_get_state(0, id_block->self_id, Gadget_Choices_AltNoteOff, &(c->opt_altnoteoff));
    optionbutton_get_state(0, id_block->self_id, Gadget_Choices_IgnoreClock, &(c->opt_ignoreclock));
    optionbutton_get_state(0, id_block->self_id, Gadget_Choices_FakeFastClock,
                           &(c->opt_fakefastclock));
}

/*
 * action_choices
 * Take action on choices after they're loaded or set. This only applies to choices where
 * immediate action is needed rather than being read later, like when a SWI is needed to
 * set an option with the MIDI module.
 * This probably should be done differently.
 */
void action_choices(Choices *const c)
{
    int new_channel = set_tx_channel(device_num, c->opt_txchan);
    ignore_timing(c->opt_ignoreclock);
    fake_fast_clock(c->opt_fakefastclock);
}

/*
 * load_messages_choiceswin
 * Look up messages with MessageTrans and update gadget labels.
 */
void load_messages_choiceswin(void)
{
    _kernel_oserror *err;

    msgs_init(); // load Messages file
    msgtrans_control_block *cb;
    cb = msgs_main_control_block(); // save the pointer to the control block

    // Set window and gadget text
    err = window_set_title(0, window_id_choices, msgs_lookup("Choices|1:Choices"));
    button_set_value(0, window_id_choices, Gadget_Choices_TxChanLabel,
                     msgs_lookup("Choices|4:Tx Channel"));
    actionbutton_set_text(0, window_id_choices,Gadget_Choices_DefaultButton,
                          msgs_lookup("Choices|12:Default"));
    actionbutton_set_text(0, window_id_choices, Gadget_Choices_SaveButton,
                          msgs_lookup("Choices|14:Save"));
    actionbutton_set_text(0, window_id_choices, Gadget_Choices_CancelButton,
                          msgs_lookup("Choices|16:Cancel"));
    actionbutton_set_text(0, window_id_choices, Gadget_Choices_SetButton,
                          msgs_lookup("Choices|18:Set"));
    optionbutton_set_label(0, window_id_choices, Gadget_Choices_AltNoteOff,
                           msgs_lookup("Choices|6:Zero Velocity Note Off"));
    optionbutton_set_label(0, window_id_choices, Gadget_Choices_IgnoreClock,
                           msgs_lookup("Choices|8:Ignore Clock Messages"));
    optionbutton_set_label(0, window_id_choices, Gadget_Choices_FakeFastClock,
                           msgs_lookup("Choices|10:Fake Fast Clock"));

    // Set help text
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_TxChan,
                            msgs_lookup("Choices|2:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_AltNoteOff,
                            msgs_lookup("Choices|5:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_IgnoreClock,
                            msgs_lookup("Choices|7:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_FakeFastClock,
                            msgs_lookup("Choices|9:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_DefaultButton,
                            msgs_lookup("Choices|11:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_SaveButton,
                            msgs_lookup("Choices|13:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_CancelButton,
                            msgs_lookup("Choices|15:Unable to get help."));
    gadget_set_help_message(0, window_id_choices, Gadget_Choices_SetButton,
                            msgs_lookup("Choices|17:Unable to get help."));

    if (err != NULL) {
        report_printf("MidiMon: err: in load_messages_choiceswin - %d %s",err->errnum,err->errmess);
    }

    msgtrans_close_file(cb); // close Messages file
}
