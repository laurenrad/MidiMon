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

/* Library includes */
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

/* System includes */
#include "kernel.h"
#include "swis.h"
#include "wimp.h"
#include "wimplib.h"
#include "event.h"

/* Toolbox includes */
#include "toolbox.h"
#include "iconbar.h"
#include "menu.h"
#include "ScrollList.h"
#include "saveas.h"

/* RISC_OSLib includes for MessageTrans lookup */
#include "msgs.h"
#include "msgtrans.h"

/* MidiMon includes */
#include "monitorwin.h"
#include "common.h"
#include "midi.h"
#include "preporter.h"

/* Globals */
static ObjectId window_id_main; /* Toolbox Object ID of monitor window */
static bool monitor_opened = false; /* Track if the window has been opened yet */

/* Clear all items from the ScrollList */
int clear_scrolllist(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  scrolllist_delete_items(0,window_id_main,Gadget_Monitor_ScrollList,0,-1);
  return 1;
}

/* Message handler for Midipal notification of incomming command */
int handle_incoming(WimpMessage *message, void *handle)
{
  char printbuf[MaxLine]; /* MaxLine is in common.h */

  int command;

  /* bits 24-25 are number of bytes in the command */
  while (((command = read_rx_command(device_num)) >> 24 & 3) != 0) {
    parse_command(command,printbuf,MaxLine);
    /* I'm kind of afraid to do this because I couldn't find it documented,
       but it seems that if you give index -1 it will add to the end. */
    scrolllist_add_item(ScrollList_AddItem_MakeVisible,window_id_main,
                        Gadget_Monitor_ScrollList,printbuf,NULL,NULL,-1);
  }

  return 1;
}

/* Called when the monitor window is shown. Save ObjectId, load messages, set defaults */
int window_monitor_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  if (!monitor_opened) {
    /* Store window ID */
    monitor_opened = true;
    window_id_main = id_block->self_id;
    load_messages_monitorwin();
    /* Invert ScrollList colours */
    scrolllist_set_colour(0,window_id_main,Gadget_Monitor_ScrollList,-256,0);
    /* Set device display */
    update_device_display();
  }

  return 1;
}

/* Called in response to SaveAs classes SaveAs_SaveToFile event */
int save_log_text(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  SaveAsSaveToFileEvent *e = (SaveAsSaveToFileEvent *)event;
  unsigned int size_estimate = 0; /* estimated file size */
  unsigned int item_count = 0; /* number of items in ScrollList */
  char buf[MaxLine];
  char *filename = e->filename;
  FILE *outfile;
  _kernel_oserror *err;

  /* Must provide the estimated file size. This is tough to do accurately
     because of the lazy way the text is just being stored in the ScrollList,
     but how important is this? At any rate this will err on the side of
     overestimating because it will multiply the maximum line length by
     number of lines. If proper allocation is done later this will be
     reworked. */
  scrolllist_count_items(0,window_id_main,Gadget_Monitor_ScrollList,
  	    	  	&item_count);
  size_estimate = item_count * (unsigned int) MaxLine;
  saveas_set_file_size(0,id_block->self_id,size_estimate);

  /* Since we need to build the file up, first create it with OS_File 11
     so any oserrors can be caught and dealt with appropriately. */
  err = _swix(OS_File,_INR(0,4),11,filename,0xfff,0,0);
  if (err != NULL) {
    report_printf("MidiMon: Error creating file: %d %s",err->errnum,err->errmess);
    wimp_report_error(err,0,"MidiMon",NULL,NULL,NULL);
    saveas_file_save_completed(0,id_block->self_id,filename);
  }
  else if ((outfile = fopen(filename,"w")) == NULL) {
    /* Somehow the file couldn't be opened for writing even though it was
       just created. Just notify Toolbox and throw up a dialogue for now */
    saveas_file_save_completed(0,id_block->self_id,filename);
    _kernel_oserror e = {255, "Unable to create file."};
    wimp_report_error(&e,0,"MidiMon",NULL,NULL,NULL);
  }
  else {
    /* Do the actual save by pulling each line from the ScrollList and
       printing them to the file. Stupid and inefficient, but the lengths
       I go to to avoid dealing with allocating storage! */
    for (int i = 0; i < item_count; i++) {
      err = scrolllist_get_item_text(0,window_id_main,Gadget_Monitor_ScrollList,
           	   	   	     buf,MaxLine,i,NULL);
      fprintf(outfile,"%s\n",buf);
    }

    /* Inform the toolbox the file has been saved successfully */
    saveas_file_save_completed(1,id_block->self_id,filename);

    fclose(outfile);
  }

  return 1;
}

/* Update the device name display */
void update_device_display(void)
{
  _kernel_oserror *err = NULL;
  /* Only update if the ID is known */
  if (monitor_opened) {
    if (device_num != -1) {
      char *prod_name = get_product_name(device_num+1); /* device number is 1-4 here */
      char display_name[ProdNameMaxLen];
      if (prod_name != NULL) {
        snprintf(display_name,ProdNameMaxLen,"%s",prod_name);
        err = displayfield_set_value(0,window_id_main,Gadget_Monitor_DeviceDisplay,prod_name);
      }
      else {
        report_printf("MidiMon: Unknown error getting product name");
      }
    }
    else {
      /* Load in localised "No Device" message from file. Not efficient but easy for now. */
      msgs_init();
      msgtrans_control_block *cb;
      cb = msgs_main_control_block();
      err = displayfield_set_value(0,window_id_main,Gadget_Monitor_DeviceDisplay,
      	    		     	   msgs_lookup("Monitor|6:No device selected"));
      msgtrans_close_file(cb);
    }

    if (err != NULL) {
      report_printf("MidiMon: err updating device display: %d %s",err->errnum,err->errmess);
    }
  }
}

int test_button_click(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  /* note that asctime will add a newline, but oh well */
  time_t t;
  t = time(NULL);
  struct tm *local = localtime(&t);
  scrolllist_add_item(0,window_id_main,Gadget_Monitor_ScrollList,asctime(local),NULL,NULL,-1);

  return 1;
}

/* Load Messages with MessageTrans. Note that buffer lengths may need to be adjusted in res
   file if localised.
 */
void load_messages_monitorwin(void)
{
  _kernel_oserror *err;

  /* Load Messages file with default name and save pointer to control block */
  msgs_init();
  msgtrans_control_block *cb;
  cb = msgs_main_control_block();

  /* Set gadget and window text */
  err = window_set_title(0,window_id_main,msgs_lookup("Monitor|1:Monitor")); /* window title */
  displayfield_set_value(0,window_id_main,Gadget_Monitor_DeviceDisplay,
  	       	   	 msgs_lookup("Monitor|6:No device")); /* device displayfield */
  button_set_value(0,window_id_main,Gadget_Monitor_DeviceLabel,
  	     	     	 msgs_lookup("Monitor|4:Device")); /* device label (actually a button) */

  /* Set help strings */
  /* The ScrollList help text doesn't display. This might be a ToolBox bug? */
  gadget_set_help_message(0,window_id_main,Gadget_Monitor_ScrollList,
  	     	  	  msgs_lookup("Monitor|2:Unable to get help."));
  gadget_set_help_message(0,window_id_main,Gadget_Monitor_DeviceDisplay,
  	     	  	  msgs_lookup("Monitor|5:Unable to get help."));

  if (err != NULL) {
    report_printf("MidiMon: err in load_messages_monitorwin - %d: %s",err->errnum,err->errmess);
  }

  msgtrans_close_file(cb); /* Close Messages file */
}
