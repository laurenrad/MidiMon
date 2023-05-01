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
 * Author: Lauren Rad
 * File: iconbar.c
 * Description: Handlers for Iconbar icon and associated menus.
*/

/* Library headers */

/* System headers */
#include "kernel.h"
#include "swis.h"

/* ToolBox headers */
#include "toolbox.h"
#include "event.h"
#include "menu.h"

/* MidiMon headers */
#include "common.h"
#include "midi.h"
#include "preporter.h"
#include "monitorwin.h"

/* Handler for default menu selection event */
int device_selection(int event_code, ToolboxEvent *event, IdBlock *id_block,
    	 	   void *handle)
{
  device_num = id_block->self_component; /* set global device num */

  /* Update menu ticks */

  for (int i = 0; i < 4; i++) {
    menu_set_tick(0,id_block->self_id,i,0); /* first, untick all */
  }
  menu_set_tick(0,id_block->self_id,
       	   	device_num,1); /* tick selected */

  /* Tell the monitor window to update its device display, if it's been opened */
  update_device_display(); /* devices are numbered 1-4 here */

  clear_rx_buf(device_num); /* clear device rx buffer */
#ifdef REPORTER_DEBUG
  report_printf("MidiMon: Device num set to %d and buffer cleared.",device_num);
#endif

  return 1;
}

/* Devices menu shown - enable/disable based on available devices.
   The mod documentation isn't clear but this is making the assumption that
   available devices are contiguous, since there is no 'official' way
   to check anything other than the number of devices currently. */
int update_devices_menu(int event_code, ToolboxEvent *event,
    	   	   	IdBlock *id_block, void *handle)
{
  const int MAX_DEVICES = 4; /* this is hardcoded into the module */
  const int PRODNAME_LENGTH = 50; /* this is set in res file */
  ObjectId menu_id = id_block->self_id;
  int devices = device_count();
  char entrystring[PRODNAME_LENGTH];
  char *prodname;

  /* Fade/unfade based on available devices.
     Obviously, this makes the assumption that the component IDs in the
     menu are set up correctly. So don't break them :3 */
  for (int component = 0; component < MAX_DEVICES; component++) {
    if (component < devices) {
      menu_set_fade(0,menu_id,component,0); /* unfade */
      prodname = get_product_name(component+1); /* devices are numbered 1-4 here */
      snprintf(entrystring,PRODNAME_LENGTH,"%d %s",component+1,prodname);
      menu_set_entry_text(0,menu_id,component,entrystring);
    }
    else {
      menu_set_fade(0,menu_id,component,1); /* fade */
    }
  }

  /* Tick the selected device if a device is set. */
  for (int i = 0; i < 4; i++) {
    menu_set_tick(0,menu_id,i,0); /* first untick all */
  }
  if (device_num != -1) {
    menu_set_tick(0,menu_id,device_num,1);
  }

  return 1;
}

/* Handle the Panic menu option in the Iconbar menu by calling reset_midi. */
int midi_panic(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle)
{
  reset_midi();
  return 1;
}
