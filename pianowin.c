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

#include <stdbool.h>

#include "wimp.h"
#include "wimplib.h"
#include "kernel.h"
#include "toolbox.h"
#include "gadgets.h"
#include "window.h"
#include "event.h"
#include "msgs.h"               // RISC_OSLib
#include "msgtrans.h"           // RISC_OSLib

// MidiMon stuff
#include "common.h"
#include "pianowin.h"
#include "pianoconst.h"
#include "preporter.h"
#include "midi.h"

/*
 * These structs correspond to the data coming from the MIDIEvent helper module.
 */
typedef struct KeyEventData {
    char key_num;               // see PRM 1-158
    int driver_id;
    int state;                  // 0 = up, 1 = down
} KeyEventData;

typedef struct PollWordData {
    int nonzero;
    int key_count;
    int midi_count;
} PollWordData;

#define KEY_COUNT   24
#define BASE_NOTE   60          // Note number of the lowest C on the piano
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
#define Gadget_Piano_TypeNotes		0x04    // on horizontal toolbar
#define Gadget_Piano_Vel		0x05    // on horizontal toolbar
#define Gadget_Piano_Oct		0x01    // on horizontal toolbar
#define Gadget_Piano_VelLabel		0x03    // on horizontal toolbar
#define Gadget_Piano_OctLabel		0x02    // on horizontal toolbar
#define Gadget_Piano_PitchBend		0x00    // on vertical toolbar
#define Gadget_Piano_PitchLabel		0x01    // on vertical toolbar

static ObjectId window_id_piano;        // piano window's ObjectId
static ObjectId tbar_id_h;              // horizontal toolbar's ObjectId
static ObjectId tbar_id_v;              // vertical toolbar's ObjectId
static bool piano_opened = false;       // Track if we know the window ID yet
static int keys_pressed[KEY_COUNT];     // keep track of what keys are down.

int slider_valuechange(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int key_pressed(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
int key_clicked(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
int slider_snap(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
void load_messages_pianowin(void);
int hotkeys_enabled(void);
int has_caret(void);
int get_velocity(void);
int get_octave(void);

/*
 * window_piano_onshow
 * This handler is called when the piano window is shown.
 * This performs first-time setup including saving the ObjectId, updating messages,
 * and registering event handlers.
 * This one also needs to get the toolbar IDs as well, as this window has toolbars.
 */
int window_piano_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    if (!piano_opened) {
        piano_opened = true;
        window_id_piano = id_block->self_id;

        window_get_tool_bars(0x09, window_id_piano, &tbar_id_h, NULL, NULL, &tbar_id_v);
        load_messages_pianowin();

        event_register_toolbox_handler(-1, Slider_ValueChanged, slider_valuechange, NULL);
        event_register_wimp_handler(-1, Wimp_EMouseClick, key_clicked, 0);
        event_register_wimp_handler(-1, Wimp_EMouseClick, slider_snap, 0);
    }

    return 1;
}

/*
 * slider_valuechange
 * This handler is called on any Slider_ValueChanged event.
 * This filters for the pitch bend slider, and then sends pitch bend on value change.
 */
int slider_valuechange(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    if (id_block->self_component == Gadget_Piano_PitchBend) {
        int val;
        slider_get_value(0, tbar_id_v, Gadget_Piano_PitchBend, &val);
        tx_pitchwheel(val);
    } else {
        return 0;               // this wasn't for the pitch bend, so return unhandled
    }

    return 1;
}

/*
 * slider_snap
 * Another handler for Wimp_EMouseClick; this one doesn't claim the event so that key_clicked
 * can still handle. Is this safe? I'm not sure but it seems to work, and this is the less
 * important of the two so it's the one that doesn't claim. The purpose of this is to make
 * the pitch bend slider snap back to the center, as these controls usually do.
 * Currently, the mechanism for this is to release the slider and adjust click on it to reset.
 * This isn't ideal but while the slider is dragged, an adjust click won't be recognised,
 * so this is the best I can do for now.
 */
int slider_snap(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
    WimpMouseClickEvent *m = (WimpMouseClickEvent *) event;

    if (piano_opened && id_block->parent_id == window_id_piano     // filter for pitch bend
        && id_block->self_component == Gadget_Piano_PitchBend) {   // then for adjust clicks
        if (m->buttons == 1) {
            int current_val;
            slider_get_value(0, id_block->self_id, Gadget_Piano_PitchBend, &current_val);
            /*
             * Only snap if it isn't already centered, or else nasty flickering will happen.
             * Ok, nasty flickering still happens if you drag, but this at least eliminates it when
             * the mouse is still.
             */
            if (current_val != 8192) {  // 8192 = middle value
                slider_set_value(0, id_block->self_id, Gadget_Piano_PitchBend, 8192);
                tx_pitchwheel(8192); // transmit this to end bending
            }
        }
    }

    return 0;
}


/*
 * int key_pressed
 * This is a handler for a nonzero pollword; to keep it simple the pollword is a single
 * word which the module will set depending on the event. Then, if it's a key event
 * this handler will call a SWI to get info from the key buffer.
 * The way all this works is nonstandard but it allows the program to respond to key up events
 * and ignore key repeat.
 * The main disadvantage currently is that this is tied to a QWERTY layout and doesn't know
 * necessarily handle the caret properly.
 */
int key_pressed(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
    WimpPollWordNonZeroEvent *e = (WimpPollWordNonZeroEvent *)event;
    PollWordData *pword = (PollWordData *)(e->poll_word);
    ComponentId component = 0;
    int note = 0;
    int octave = get_octave();
    int velocity = get_velocity();
    int key_num, state;

    if (pword->key_count > 0) {
        _swix(MIDIEvent_GetKeypress,_OUT(0)|_OUT(2),&key_num,&state);

        if (key_num == -1) {
            report_printf("Couldn't get keypress!");
            return 1;
        }

        report_printf("Got keypress: %d (%d)",key_num, state);

        if (piano_opened == false) {
            return 1;  // only do something if the window has been opened yet
        }

        if (!hotkeys_enabled() && !has_caret()) {
            return 1;  // also don't do anything if hotkeys are off and we don't have the caret
        }

        /*
        * Then, check for the special case of a mouse up event.
        * This should be more elegant.
        */
        if (key_num == MOUSE_SELECT || key_num == MOUSE_ADJUST) {
            if (state == 0) {
                for (int i = 0; i < KEY_COUNT; i++) {
                    if (keys_pressed[i] == 1) {
                        keys_pressed[i] = 0;
                        tx_noteoff(60 + i, velocity, octave); // offset from base note
                    }
                }
            }
            return 1; // done and handled
        }

        /*
        * Otherwise, check the key number and set the note accordingly
        */
        switch (key_num) {
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
            return 1;               // unhandled key, just return
            break;
        }

        if (state == 0) {         // key up, clear bit 21 (selected bit) for the given button
            button_set_flags(0, window_id_piano, component, 0x200000, 0x0);
            tx_noteoff(note, velocity, octave);     // send note off message for this note
        } else {                    // key down, set bit 21 (selected bit) for the given button
            button_set_flags(0, window_id_piano, component, 0x200000, 0x200000);
            tx_noteon(note, velocity, octave);      // send note on message for this note
        }

        return 1;
    }

    /* If there was no key event, see if there are any MIDI events.
       In practice, this shouldn't happen as midi_incoming was registered after
       and has higher priority, but it's best practice to leave it here */
    return 0;
}

/*
 * This handler is called on key clicks.
 * This watches for mouse down clicks on the piano keys and responds.
 * It also attempts to grab the caret if the window doesn't have it already.
 * Warning: In order to make the code more succinct, this makes the assumption
 * that the components have sequential IDs, which should be true but careful
 * mucking around in the resource file then.
 */
int key_clicked(int event_code, WimpPollBlock * event, IdBlock * id_block, void *handle)
{
    ObjectId window = id_block->self_id;
    ComponentId component = id_block->self_component;
    int window_handle_piano = -1;
    int note = 0;
    int octave = get_octave();
    int velocity = get_velocity();

    if ((piano_opened == true) && (window == window_id_piano)) {        // filter for piano win
        window_get_wimp_handle(0, window_id_piano, &window_handle_piano);
        wimp_set_caret_position(window_handle_piano, -1, 0, 0, -1, -1); // attempt to gain caret

        if (component < KEY_COUNT && component >= 0) {  // filter for piano key buttons
            note = component + BASE_NOTE;
            keys_pressed[component] = 1;        // mark key as pressed for mouseup
            tx_noteon(note, velocity, octave);  //send note on message
        }
    }

    return 1;
}

/* load_messages_pianowin
 * Loads Messages with MessageTrans. This one is a bit different because all the translatable
 * gadgets are on toolbars.
 */
void load_messages_pianowin(void)
{
    _kernel_oserror *err;
    ObjectId toolbar_id_h, toolbar_id_v;

    /*
     * Get ObjectIds of the toolbars
     */
    window_get_tool_bars(0, window_id_piano, &toolbar_id_h, NULL, NULL, &toolbar_id_v);

    /*
     * Set gadget and window text
     */
    err = window_set_title(0, window_id_piano, msgs_lookup("Piano|1:err")); // window title
    button_set_value(0, toolbar_id_h, Gadget_Piano_VelLabel,
                     msgs_lookup("Piano|53:err")); // piano velocity label
    button_set_value(0, toolbar_id_h, Gadget_Piano_OctLabel,
                     msgs_lookup("Piano|57:err")); // piano octave label
    optionbutton_set_label(0, toolbar_id_h, Gadget_Piano_TypeNotes,
                     msgs_lookup("Piano|51:err")); // type notes option

    /*
     * Set help text
     */
    gadget_set_help_message(0, toolbar_id_h, Gadget_Piano_TypeNotes,
                            msgs_lookup("Piano|50:Unable to get help."));
    gadget_set_help_message(0, toolbar_id_h, Gadget_Piano_Vel,
                            msgs_lookup("Piano|54:Unable to get help."));
    gadget_set_help_message(0, toolbar_id_h, Gadget_Piano_Oct,
                            msgs_lookup("Piano|55:Unable to get help."));

    if (err != NULL) {
        report_printf("MidiMon: err in load_messages_pianowin - %d: %s", err->errnum, err->errmess);
    }
}

/*
 * hotkeys_enabled
 * Return 1 if hotkeys are enabled, 0 if disabled.
 */
int hotkeys_enabled(void)
{
    int state = 0;

    optionbutton_get_state(0, tbar_id_h, Gadget_Piano_TypeNotes, &state);

    return state;
}

/*
 * get_velocity
 * Returns the velocity from the velocity slider.
 */
int get_velocity(void)
{
    int velocity;
    numberrange_get_value(0, tbar_id_h, Gadget_Piano_Vel, &velocity);
    return velocity;
}

/*
 * get_octave
 * Returns the octave from the octave slider.
 */
int get_octave(void)
{
    int octave;
    numberrange_get_value(0, tbar_id_h, Gadget_Piano_Oct, &octave);
    return octave;
}

/*
 * has_caret
 * Returns 1 if piano window has the caret, 0 otherwise.
 */
int has_caret(void)
{
    _kernel_oserror *err = NULL;
    WimpGetCaretPositionBlock b;       // in wimp.h
    int window_handle_piano;    // Wimp handle of piano window

    if (piano_opened) {
        err = wimp_get_caret_position(&b);
        window_get_wimp_handle(0, window_id_piano, &window_handle_piano);
        if (err != NULL) {
            report_printf("an error occurred in has_caret: %d %s", err->errnum, err->errmess);
        } else {
            if (b.window_handle == window_handle_piano) {
                return 1;
            }
        }
    }

    return 0;
}
