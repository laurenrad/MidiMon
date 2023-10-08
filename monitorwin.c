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
 * File: monitor.c
 * Author: Lauren Rad
 * Purpose: Handlers for the Monitor window.
 */

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "kernel.h"
#include "swis.h"
#include "wimp.h"
#include "wimplib.h"
#include "event.h"
#include "toolbox.h"
#include "menu.h"
#include "ScrollList.h"
#include "saveas.h"
#include "msgs.h"               // RISC_OSLib
#include "msgtrans.h"           // RISC_OSLib

// MidiMon stuff
#include "monitorwin.h"
#include "common.h"
#include "midi.h"
#include "preporter.h"

/* Gadgets: Monitor window gadgets */
#define Gadget_Monitor_ScrollList       0x00
#define Gadget_Monitor_DeviceDisplay	0x04
#define Gadget_Monitor_DeviceLabel	0x05
#define MenuEntry_Monitor_Save		0x02
#define MenuEntry_Monitor_Selection	0x05

static ObjectId window_id_main; // Toolbox Object ID of monitor window
static bool monitor_opened = false;     // Track if the window has been opened yet

int clear_scrolllist(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int save_log_text(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int test_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
void load_messages_monitorwin(void);

/*
 * These structs correspond to the data coming from the MIDIEvent helper module.
 */
typedef struct MIDIEventData {
    int event;
} MIDIEventData;

typedef struct PollWordData {
    int nonzero;
    int key_count;
    int midi_count;
} PollWordData;

/*
 * window_monitor_onshow
 * This handler is called when the Monitor window is shown.
 * Performs first-time setup including storing the ObjectId, loading messages,
 * and more.
 */
int window_monitor_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    if (!monitor_opened) {
        monitor_opened = true;
        window_id_main = id_block->self_id;
        load_messages_monitorwin();
        // Invert ScrollList colours
        scrolllist_set_colour(0, window_id_main, Gadget_Monitor_ScrollList, -256, 0);
        update_device_display();
        event_register_toolbox_handler(-1, Event_Monitor_ClearLog, clear_scrolllist, NULL);
        event_register_toolbox_handler(-1, Event_Monitor_Test, test_button_click, NULL);
        event_register_toolbox_handler(-1, SaveAs_SaveToFile, save_log_text, NULL);
    }

    return 1;
}

/*
 * clear_scrolllist
 * This handler is called when the Clear entry is selected from the Monitor menu.
 * Clears all entries in the Monitor.
 */
int clear_scrolllist(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    scrolllist_delete_items(0, window_id_main, Gadget_Monitor_ScrollList, 0, -1); // -1 = end
    return 1;
}

/*
 * midi_incoming
 * Handle incoming MIDI notifications from the helper module.
 */
#define MIDI_DataReceived   0
#define MIDI_Error          1
#define MIDI_Connect        10
#define MIDI_Disconnect     11
#define MIDI_Initialised    20
#define MIDI_Dying          21
int midi_incoming(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle)
{
    WimpPollWordNonZeroEvent *e = (WimpPollWordNonZeroEvent *)event;
    PollWordData *pword = (PollWordData *)(e->poll_word);
    char printbuf[MaxLine];
    int command;
    int midi_event;

    if (pword->midi_count > 0) {
        _swi(MIDIEvent_GetMIDIEvent,_OUT(0),&midi_event);
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Received MIDI event %d",midi_event);
#endif

        switch (midi_event) {
            case MIDI_DataReceived:
            while (((command = read_rx_command(device_num)) >> 24 & 3) != 0) {
                parse_command(command, printbuf, MaxLine);
                scrolllist_add_item(ScrollList_AddItem_MakeVisible, window_id_main,
                                    Gadget_Monitor_ScrollList, printbuf, NULL, NULL, -1);
            }
            break;
            /* The rest of these aren't fully implemented yet. */
            case MIDI_Error:
            report_printf("MidiMon: A MIDI error has occurred");
            break;
            case MIDI_Connect:
            report_printf("MidiMon: A MIDI device has been connected");
            break;
            case MIDI_Disconnect:
            report_printf("MidiMon: A MIDI device has been disconnected");
            break;
            case MIDI_Initialised:
            report_printf("MidiMon: MIDI Module has been initialised");
            break;
            case MIDI_Dying:
            report_printf("MidiMon: MIDI Module is dying");
            break;
            default:
            report_printf("MidiMon: Unknown event received from MIDIEvent: %d",midi_event);
            break;
        }
        return 1;
    }

    return 0; // if there was no MIDI event, see if there are any KeyEvents
}

/*
 * save_log_text
 * This handler is called in response to SaveToFile events.
 * Depending on which menu entry this was called from, this will either
 * save / copy the entire log contents, or just the selected entry.
 */
int save_log_text(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    SaveAsSaveToFileEvent *e = (SaveAsSaveToFileEvent *) event;
    unsigned int size_estimate = 0;     // estimated file size
    unsigned int item_count = 0;        // number of items in ScrollList
    int selected = 0;           // currently selected item if applicable
    char buf[MaxLine];
    char *filename = e->filename;
    FILE *outfile;
    _kernel_oserror *err;

    report_printf("parent was %d", id_block->parent_component);

    /*
     * First, when estimating size check if we are doing the whole thing or
     * if it's just the selection
     */
    if (id_block->parent_component == MenuEntry_Monitor_Save) {
        /*
         * Must provide the estimated file size. This is tough to do accurately
         * * because of the lazy way the text is just being stored in the ScrollList,
         * * but how important is this? At any rate this will err on the side of
         * * overestimating because it will multiply the maximum line length by
         * * number of lines. If proper allocation is done later this will be
         * * reworked.
         */
        scrolllist_count_items(0, window_id_main, Gadget_Monitor_ScrollList, &item_count);
        size_estimate = item_count * (unsigned int)MaxLine;
        saveas_set_file_size(0, id_block->self_id, size_estimate);
    } else if (id_block->parent_component == MenuEntry_Monitor_Selection) {
        err = scrolllist_get_selected(0, window_id_main, Gadget_Monitor_ScrollList, 0, &selected);
        err = scrolllist_get_item_text(0, window_id_main, Gadget_Monitor_ScrollList,
                                       buf, MaxLine, selected, NULL);
        if (err != NULL) {
            report_printf("err: %d; %s\n", err->errnum, err->errmess);
            size_estimate = strlen(buf);
            report_printf("size estimate: %d", size_estimate);
            saveas_set_file_size(0, id_block->self_id, size_estimate);
        }
    } else {
        return 0; // quit without handling if this was called from somewhere else
    }

    /*
     * Since we need to build the file up, first create it with OS_File 11
     * so any oserrors can be caught and dealt with appropriately.
     */
    err = _swix(OS_File, _INR(0, 4), 11, filename, 0xfff, 0, 0);
    if (err != NULL) {
        report_printf("MidiMon: Error creating file: %d %s", err->errnum, err->errmess);
        wimp_report_error(err, 0, "MidiMon", NULL, NULL, NULL);
        saveas_file_save_completed(0, id_block->self_id, filename);
    } else if ((outfile = fopen(filename, "w")) == NULL) {
        /*
         * Somehow the file couldn't be opened for writing even though it was
         * just created. Just notify Toolbox and throw up a dialogue for now
         */
        saveas_file_save_completed(0, id_block->self_id, filename);
        _kernel_oserror e = { 255, "Unable to create file." };
        wimp_report_error(&e, 0, "MidiMon", NULL, NULL, NULL);
    } else {
        if (id_block->parent_component == MenuEntry_Monitor_Save) {
            /*
             * Do the actual save by pulling each line from the ScrollList and
             * * printing them to the file. Stupid and inefficient, but again, this avoids
             * * having to deal with the storage allocation manually!
             */
            for (int i = 0; i < item_count; i++) {
                err = scrolllist_get_item_text(0, window_id_main, Gadget_Monitor_ScrollList,
                                               buf, MaxLine, selected, NULL);
                fprintf(outfile, "%s", buf);
            }
        } else {
            err = scrolllist_get_item_text(0, window_id_main, Gadget_Monitor_ScrollList,
                                           buf, MaxLine, selected, NULL);
            fprintf(outfile, "%s", buf);
        }

        saveas_file_save_completed(1, id_block->self_id, filename);

        fclose(outfile);
    }

    return 1;
}

/*
 * update_device_display
 * Updates the device name display.
 * This can be called from elsewhere before the window is opened, so only
 * do this if the window's object ID is known.
 */
void update_device_display(void)
{
    _kernel_oserror *err = NULL;

    if (monitor_opened) {
        if (device_num != -1) {
            char *prod_name = get_product_name(device_num + 1); /* device number is 1-4 here */
            char display_name[ProdNameMaxLen];
            if (prod_name != NULL) {
                snprintf(display_name, ProdNameMaxLen, "%s", prod_name);
                err =
                    displayfield_set_value(0, window_id_main, Gadget_Monitor_DeviceDisplay,
                                           prod_name);
            } else {
                report_printf("MidiMon: Unknown error getting product name");
            }
        } else {
            /* Load in localised "No Device" message. */
            err = displayfield_set_value(0, window_id_main, Gadget_Monitor_DeviceDisplay,
                                         msgs_lookup("Monitor|6:No device selected"));
        }

        if (err != NULL) {
            report_printf("MidiMon: err updating device display: %d %s", err->errnum, err->errmess);
        }
    }
}

/*
 * test_button_click
 * This is actually for a menu item now, but this allows a debug option to
 * add things to the scrolllist without needing actual MIDI messages.
 */
int test_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
    time_t t;
    t = time(NULL);
    struct tm *local = localtime(&t);
    scrolllist_add_item(0, window_id_main, Gadget_Monitor_ScrollList, asctime(local), NULL, NULL,
                        -1);

    //return 1;
    clear_rx_buf(0);
    clear_rx_buf(1);
    clear_rx_buf(2);
    clear_rx_buf(3);

    return 1;
}

/*
 * load_messages_monitorwin
 * Load Messages with MessageTrans.
 */
void load_messages_monitorwin(void)
{
    _kernel_oserror *err;

    /*
     * Set gadget and window text
     */
    err = window_set_title(0, window_id_main, msgs_lookup("Monitor|1:Monitor")); // window title
    displayfield_set_value(0, window_id_main, Gadget_Monitor_DeviceDisplay,
                           msgs_lookup("Monitor|6:No device")); // device displayfield
    button_set_value(0, window_id_main, Gadget_Monitor_DeviceLabel,
                     msgs_lookup("Monitor|4:Device")); // device label (actually a button)

    /*
     * Set help strings
     * The ScrollList help text doesn't actually show for some reason though.
     */
    gadget_set_help_message(0, window_id_main, Gadget_Monitor_ScrollList,
                            msgs_lookup("Monitor|2:Unable to get help."));
    gadget_set_help_message(0, window_id_main, Gadget_Monitor_DeviceDisplay,
                            msgs_lookup("Monitor|5:Unable to get help."));

    if (err != NULL) {
        report_printf("MidiMon: err in load_messages_monitorwin - %d: %s", err->errnum,
                      err->errmess);
    }
}
