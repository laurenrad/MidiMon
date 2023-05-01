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

 /* Send Wimp messages in response to the lower-level Key events.
    This allows for reaction to key release, as well as ignoring
    key repeat.
 */

#include <stdlib.h>
#include <stdio.h>
#include "swis.h"
#include "kernel.h"

#define EventV		16
#define EventMouse	10
#define EventKey	11
#define EnableEvent	14
#define DisableEvent	13

/* The message event code number. This is now allocated and registered with ROOL. */
#define Message_KeyEvent  0x5A500 /* Key up/down events */

typedef struct KeyMsgData {
  char key_num; /* see PRM 1-158 */
  int driver_id;
  int state; /* 0 = up, 1 = down */
} KeyMsgData;

typedef struct KeyMsg {
  int size;
  int sender;
  int my_ref;
  int your_ref;
  int action_code;
  KeyMsgData data;
} KeyMsg;

extern void keyevent_entry(void);
int keyevent_handler(_kernel_swi_regs *r, void *pw);

#define IGNORE(x) do { (void)(x); } while(0)

static void claim_event(void *pw)
{
  _swix(OS_Claim, _INR(0,2), EventV, keyevent_entry, pw);
  _swix(OS_Byte, _INR(0,1), EnableEvent, EventKey);
}

static void release_event(void *pw)
{
  _swix(OS_Byte, _INR(0,1), DisableEvent, EventKey);
  _swix(OS_Release, _INR(0,2), EventV, keyevent_entry, pw);
}

_kernel_oserror *keyevent_init(char *cmd_tail, int podule_base, void *pw)
{
  IGNORE(cmd_tail);
  IGNORE(podule_base);
  claim_event(pw);

  return NULL;
}

_kernel_oserror *keyevent_final(int fatal, int podule, void *pw)
{
  IGNORE(fatal);
  IGNORE(podule);
  release_event(pw);
  return NULL;
}

extern int keyevent_handler(_kernel_swi_regs *r, void *pw)
{
  IGNORE(pw);

  KeyMsgData d;

  d.state = r->r[1];
  d.key_num = r->r[2];
  d.driver_id = r->r[3];

  KeyMsg m;
  m.size = sizeof(m);
  m.sender = 0; /* this is a module, so zero? */
  m.my_ref = 0; /* this should be filled in by the Wimp */
  m.your_ref = 0;
  m.action_code = Message_KeyEvent; /* also made up, this should be allocated */
  m.data = d;

  _swix(Wimp_SendMessage,_INR(0,2),17,&m,0);

  return 1;
}




