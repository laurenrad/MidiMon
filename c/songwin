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
 * File: songwin.c
 * Author: Lauren Rad
 * Purpose: Handlers for the Song Control window.
*/

#include <stdbool.h>

#include "toolbox.h"
#include "gadgets.h"
#include "window.h"
#include "event.h"
#include "msgs.h"               // RISC_OSLib
#include "msgtrans.h"           // RISC_OSLib

// MidiMon stuff
#include "common.h"
#include "midi.h"
#include "preporter.h"
#include "songwin.h"

#define Gadget_Song_Start	  	0x00    // Start button
#define Gadget_Song_Continue		0x01    // Continue button
#define Gadget_Song_Stop		0x02    // Stop button
#define Gadget_Song_SongNumLabel	0x03    // Song Select label text
#define Gadget_Song_SongNum	  	0x04    // Number range for song select
#define Gadget_Song_SongSelSend		0x05    // Song select send

static ObjectId window_id_song;   // ObjectId of this window
static bool song_opened = false;  // Track if window_id_song is known

int button_songstart(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_songcontinue(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_songstop(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_sendsongsel(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
void load_messages_songwin(void);

/*
 * window_song_onshow
 * This handler is called when the Song window is shown.
 * This performs first-time setup, such as storing the ObjectId, loading Messages,
 * and registering event handlers.
 */
int window_song_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    if (song_opened == false) {
        song_opened = true;
        window_id_song = id_block->self_id;
        load_messages_songwin();
        event_register_toolbox_handler(-1, Event_Song_Start, button_songstart, NULL);
        event_register_toolbox_handler(-1, Event_Song_Continue, button_songcontinue, NULL);
        event_register_toolbox_handler(-1, Event_Song_Stop, button_songstop, NULL);
        event_register_toolbox_handler(-1, Event_Song_SendSongSel, button_sendsongsel, NULL);
    }

    return 1;
}

/*
 * button_songstart
 * This handler is called when the Start button is clicked.
 * Sends a Song Start message in response.
 */
int button_songstart(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    tx_songstart();
    return 1;
}

/*
 * button_songcontinue
 * This handler is called when the Continue button is clicked.
 * Sends a Song Continue message in response.
 */
int button_songcontinue(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    tx_songcontinue();
    return 1;
}

/* Button handler for Stop button. Tx song stop message */
/*
 * button_songstop
 * This handler is called when the Stop button is clicked.
 * Sends a Song Stop message in response.
 */
int button_songstop(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    tx_songstop();
    return 1;
}

/*
 * button_sendsongsel
 * This handler is called when the Send button is clicked.
 * This sends a Song Select message with the song selected in the numberrange.
 */
int button_sendsongsel(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    int song_num = 0;

    numberrange_get_value(0, window_id_song, Gadget_Song_SongNum, &song_num);
    /*
     * Making the numberrange writable allows it to have invalid (positive) values.
     * There might be a standard way to handle this, but for now if it's out of range set it
     * back to 127. Setting the numberrange fails on very large values, but it still sends 127
     * so this should be ok enough for now.
     */
    if (song_num > 127) {
        numberrange_set_value(0, window_id_song, Gadget_Song_SongNum, 127);
        song_num = 127;
    }
    tx_songsel(song_num);

    return 1;
}

void load_messages_songwin(void)
{
    _kernel_oserror *err;

    /*
     * Window and gadget text
     */
    err = window_set_title(0, window_id_song, msgs_lookup("SongControl|1:err")); // window title
    button_set_value(0, window_id_song, Gadget_Song_SongNumLabel,
                     msgs_lookup("SongControl|9:Song Select"));   // song select label
    actionbutton_set_text(0, window_id_song, Gadget_Song_Start,
                          msgs_lookup("SongControl|3:Start"));    // start button
    actionbutton_set_text(0, window_id_song, Gadget_Song_Continue,
                          msgs_lookup("SongControl|5:Continue")); // continue button
    actionbutton_set_text(0, window_id_song, Gadget_Song_Stop,
                          msgs_lookup("SongControl|7:Stop"));     // stop button
    actionbutton_set_text(0, window_id_song, Gadget_Song_SongSelSend,
                          msgs_lookup("SongControl|12:Send"));    // song select send button

    /*
     * Help text
     */
    gadget_set_help_message(0, window_id_song, Gadget_Song_Start,
                            msgs_lookup("SongControl|2:Unable to get help."));  // start button
    gadget_set_help_message(0, window_id_song, Gadget_Song_Continue,
                            msgs_lookup("SongControl|4:Unable to get help."));  // continue button
    gadget_set_help_message(0, window_id_song, Gadget_Song_Stop,
                            msgs_lookup("SongControl|6:Unable to get help."));  // stop button
    gadget_set_help_message(0, window_id_song, Gadget_Song_SongSelSend,
                            msgs_lookup("SongControl|11:Unable to get help.")); // song select send
    gadget_set_help_message(0, window_id_song, Gadget_Song_SongNum,
                            msgs_lookup("SongControl|10:Unable to get help.")); // song number

    if (err != NULL) {
        report_printf("MidiMon: err in load_messages_songwin - %d: %s", err->errnum, err->errmess);
    }
}
