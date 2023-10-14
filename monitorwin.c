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
#define MenuEntry_Monitor_Test          0x04

static ObjectId window_id_main; // Toolbox Object ID of monitor window
static bool monitor_opened = false;     // Track if the window has been opened yet

int clear_scrolllist(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int save_log_text(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int test_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int menu_setup(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
void load_messages_monitorwin(void);

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
        event_register_toolbox_handler(-1, Event_Monitor_ShowMenu, menu_setup, NULL);
    }

    return 1;
}

/*
 * menu_setup
 * Perform any needed setup for the monitor window menu.
 * Currently this removes the "Test" menu entry if built without REPORTER_DEBUG.
 */
int menu_setup(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
#ifndef REPORTER_DEBUG
    menu_remove_entry(0, id_block->self_id, MenuEntry_Monitor_Test);
#endif
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
 * add_to_monitor
 * Add an item to the monitor ScrollList.
 */
void add_to_monitor(char *buf)
{
    char printbuf[MaxLine];
    scrolllist_add_item(ScrollList_AddItem_MakeVisible, window_id_main,
                        Gadget_Monitor_ScrollList, buf, NULL, NULL, -1);
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

    int buf1, buf2, buf3, buf4;
    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), 0 << 1, &buf1);
    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), 1 << 1, &buf2);
    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), 2 << 1, &buf3);
    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), 3 << 1, &buf4);
    report_printf("Buf sizes: 0=%d 1=%d 2=%d 3=%d",buf1,buf2,buf3,buf4);

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
