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

 /* This is a helper module for MidiMon, to allow it to be notified of incoming
    key and MIDI events and service calls.*/


#include <stdlib.h>
#include <stdio.h>
#include "swis.h"
#include "kernel.h"

#define EventV		16
#define EventMidi	17
#define EnableEvent	14
#define DisableEvent	13

/* This is the newly rewritten module that combines key and MIDI events in a way that is,
 * hopefully, "legal" (unlike the last version). This doesn't currently buffer keypresses
 * which it needs to be reworked to do but this was a quick fix.
 */

typedef struct KeyEventData {
  int key_num; /* see PRM 1-158 */
  int driver_id;
  int state; /* 0 = up, 1 = down */
} KeyEventData;

typedef struct MidiEventData {
    int event;
} MidiEventData;

#define ErrorBadSWI ((_kernel_oserror *) -1)

static int pollword; // for now: 0=clear, 1=keyevent, 2=midievent
static KeyEventData key = {0,0,0};
static MidiEventData midi;

extern void event_entry(void);
void service_handler(int service_number, _kernel_swi_regs *r, void *pw);
int event_handler(_kernel_swi_regs *r, void *pw);

#define IGNORE(x) do { (void)(x); } while(0)

static void claim_event(void *pw)
{
  _swix(OS_Claim, _INR(0,2), EventV, event_entry, pw);
  _swix(OS_Byte, _INR(0,1), EnableEvent, EventMidi);
}

static void release_event(void *pw)
{
  _swix(OS_Byte, _INR(0,1), DisableEvent, EventMidi);
  _swix(OS_Release, _INR(0,2), EventV, event_entry, pw);
}

_kernel_oserror *midievent_init(char *cmd_tail, int podule_base, void *pw)
{
  IGNORE(cmd_tail);
  IGNORE(podule_base);
  claim_event(pw);

  return NULL;
}

_kernel_oserror *midievent_final(int fatal, int podule, void *pw)
{
  IGNORE(fatal);
  IGNORE(podule);
  release_event(pw);
  return NULL;
}

_kernel_oserror *midievent_swi(int swi_offset, _kernel_swi_regs *r, void *pw)
{
    IGNORE(pw);
    switch (swi_offset) {
        case 0: // MIDIEvent_GetPollWord &5A4C0
            r->r[0] = (int)&pollword;
        break;
        case 1: // MIDIEvent_ClearPollWord &5A4C0
            pollword = 0;
        break;
        case 2: // MIDIEvent_GetKeypress &5A4C2
            r->r[0] = key.key_num;
            r->r[1] = key.driver_id;
            r->r[2] = key.state;
        break;
        case 3: // MIDIEvent_GetMIDIEvent &5A4C3
            r->r[0] = midi.event;
        default:
            return ErrorBadSWI;
        break;
    }

    return NULL;
}


/* Handler for MIDI-related service calls */
/* The numbers this puts in midi.event are nonstandard, but some remapping had to be
   done to notify of MIDI service calls and events in the same status word. */
void service_handler(int service_number, _kernel_swi_regs *r, void *pw)
{
  IGNORE(pw);

  switch (r->r[0]) {
    case 0:
        midi.event = 20; // module initialised
    break;
    case 1:
        midi.event = 21; // module dying
    break;
    default:
        midi.event = -1; // Unknown
    break;
  }

  r->r[0] = 1; /* Do not claim service */
}

int event_handler(_kernel_swi_regs *r, void *pw)
{
  IGNORE(pw);
  IGNORE(r);
  KeyEventData k;
  switch (r->r[0]) {
      case 11: // Key event
        /* Store key info from regs to be sent on request */
        key.state = r->r[1];
        key.key_num = r->r[2];
        key.driver_id = r->r[3];
        pollword = 1;
      break;
      case 17: // MIDI event
        /* Here are the relevant events that can happen here:
         * 0:  MIDI Data Received
         * 1:  MIDI Error
         * 10: MIDI Device Connect
         * 11: MIDI Device Disconnect
         */
        midi.event = r->r[1];
        pollword = 2;
      break;
      default:
      break;
  }
  return 1;
}
