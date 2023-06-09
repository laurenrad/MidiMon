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
 * File: songwin.h
 * Author: Lauren Rad
 * Purpose: Handlers for the Song Control window (Header).
*/

#ifndef MIDIMON_SONGWIN_H
#define MIDIMON_SONGWIN_H

/* Functions */
int window_song_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_songstart(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_songcontinue(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_songstop(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int button_sendsongsel(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
void load_messages_songwin(void);

#endif
