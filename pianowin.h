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
 * File: piano.h
 * Author: Lauren Rad
 * Purpose: Handlers for the piano controller window. (Header)
*/

#ifndef MIDIMON_PIANO_H
#define MIDIMON_PIANO_H

#include "toolbox.h"

typedef struct KeyUpData {
  char key_num; /* see PRM 1-158 */
  int driver_id;
  int state; /* 0 = up, 1 = down */
} KeyUpData;

typedef struct KeyUpMessage {
  int size;
  int sender;
  int my_ref;
  int your_ref;
  int action_code;
  KeyUpData key_data;
} KeyUpMessage;

int window_piano_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int slider_valuechange(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int key_pressed(WimpMessage *message, void *handle);
int key_clicked(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
int slider_snap(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
void load_messages_pianowin(void);

/* Following is constants for the keycodes on PRM page 1-158 */
/* Only alphanumeric keys included for now */
#define KEY_Q	     0x27
#define KEY_W	     0x28
#define KEY_E	     0x29
#define KEY_R	     0x2A
#define KEY_T	     0x2B
#define KEY_Y	     0x2C
#define KEY_U	     0x2D
#define KEY_I	     0x2E
#define KEY_O	     0x2F
#define KEY_P	     0x30
#define KEY_A	     0x3C
#define KEY_S	     0x3D
#define KEY_D	     0x3E
#define KEY_F	     0x3F
#define KEY_G	     0x40
#define KEY_H	     0x41
#define KEY_J	     0x42
#define KEY_K	     0x43
#define KEY_L	     0x44
#define KEY_Z	     0x4E
#define KEY_X	     0x4F
#define KEY_C	     0x50
#define KEY_V	     0x51
#define KEY_B	     0x52
#define KEY_N	     0x53
#define KEY_M	     0x54
#define KEY_1	     0x11
#define KEY_2	     0x12
#define KEY_3	     0x13
#define KEY_4	     0x14
#define KEY_5	     0x15
#define KEY_6	     0x16
#define KEY_7	     0x17
#define KEY_8	     0x18
#define KEY_9	     0x19
#define KEY_0	     0x1A
/* This also includes the mouse buttons */
#define MOUSE_SELECT 0x70
#define MOUSE_ADJUST 0x72

#endif
