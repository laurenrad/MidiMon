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

 /* This is the most rudimentary possible module to just emit a Wimp
    user message in response to a Event 17 (MIDI Event). */

#include <stdlib.h>
#include <stdio.h>
#include "swis.h"
#include "kernel.h"

#define EventV		16
#define EventMidi	17
#define EnableEvent	14
#define DisableEvent	13

/* Message definitions. Both Service_MIDI and Event_MIDI can give device connect/disconnect,
   but Event_MIDI will be providing them in this module. */
#define Message_MIDIDataReceived       	0x5A4C0 /* MIDI event has been received */
#define Message_MIDIError		0x5A4C1 /* An error has occurred in the background */
#define Message_MIDIDevConnect		0x5A4C2 /* A USB MIDI device has been connected. */
#define Message_MIDIDevDisconnect	0x5A4C3 /* A USB MIDI device has been disconnected. */
#define Message_MIDIInit		0x5A4C4 /* USB MIDI module has initialised */
#define Message_MIDIDying		0x5A4C5 /* USB MIDI module is dying */

/* Definition of the message's structure; Data is unused in all cases currently and will be 0.
   The rationale over using a reason code scheme with a single message type as with the service
   and event is this allows separate handlers to be registered which is cleaner and more convenient.
*/
typedef struct WimpMidiMessage {
  int size;
  int sender;
  int my_ref;
  int your_ref;
  int action_code;
  int data;
} WimpMidiMessage;

extern void midi_entry(void);
int midi_handler(_kernel_swi_regs *r, void *pw);

#define IGNORE(x) do { (void)(x); } while(0)

static void claim_event(void *pw)
{
  _swix(OS_Claim, _INR(0,2), EventV, midi_entry, pw);
  _swix(OS_Byte, _INR(0,1), EnableEvent, EventMidi);
}

static void release_event(void *pw)
{
  _swix(OS_Byte, _INR(0,1), DisableEvent, EventMidi);
  _swix(OS_Release, _INR(0,2), EventV, midi_entry, pw);
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

/* Handler for MIDI-related service calls */
void service_handler(int service_number, _kernel_swi_regs *r, void *pw)
{
  IGNORE(pw);

  WimpMidiMessage m;
  m.size = sizeof(m);
  m.sender = 0;
  m.my_ref = 0;
  m.your_ref = 0;
  m.data = 0;

  switch (r->r[0]) {
    case 0:
    	 m.action_code = Message_MIDIInit;
    break;
    case 1:
    	 m.action_code = Message_MIDIDying;
    break;
    default:
    	 /* If it's anything else just return without claiming svc or sending a message */
    	 r->r[0] = 1;
    	 return;
    break;
  }

  _swix(Wimp_SendMessage,_INR(0,2),17,&m,0);

  r->r[0] = 1; /* Do not claim service */
}

extern int midi_handler(_kernel_swi_regs *r, void *pw)
{
  IGNORE(pw);
  IGNORE(r);

  WimpMidiMessage m;
  m.size = sizeof(m);
  m.sender = 0; /* this is a module, so zero? */
  m.my_ref = 0; /* this should be filled in by the Wimp */
  m.your_ref = 0;
  m.data = 0;

  switch (r->r[1]) {
    case 0:
    	 m.action_code = Message_MIDIDataReceived;
    break;
    case 1:
    	 m.action_code = Message_MIDIError;
    break;
    case 10:
    	 m.action_code = Message_MIDIDevConnect;
    break;
    case 11:
    	 m.action_code = Message_MIDIDevDisconnect;
    break;
    default:
    	 /* This shouldn't happen but if it does, just return without sending. */
    	 return 1;
    break;
  }

  _swix(Wimp_SendMessage,_INR(0,2),17,&m,0);

  return 1;
}




