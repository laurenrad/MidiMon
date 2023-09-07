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
 * File: monitor.h
 * Author: Lauren Rad
 * Purpose: Main header for the Monitor window.
 */

#ifndef MIDIMON_MONITOR_H
#define MIDIMON_MONITOR_H

int window_monitor_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int midi_incoming(int event_code, WimpPollBlock *event, IdBlock *id_block, void *handle);
void update_device_display(void);

#endif
