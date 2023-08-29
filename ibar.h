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
 * File: ibar.h
 * Description: Header for Iconbar icon and associated menus.
*/

#ifndef MIDIMON_IBAR_H
#define MIDIMON_IBAR_H

int device_selection(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int update_devices_menu(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);
int midi_panic(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);

#endif
