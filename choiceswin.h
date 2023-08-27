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
 * File: choiceswin.h
 * Author: Lauren Rad
 * Purpose: Wimp handlers for Choices window (header)
*/

#ifndef MIDIMON_CHOICESWIN_H
#define MIDIMON_CHOICESWIN_H

#include <stdlib.h>
#include "choices.h"
#include "event.h"

/*
 * window_choices_onshow
 * This handler is called when the Choices window is shown.
 */
int window_choices_onshow(int event_code, ToolboxEvent *event, IdBlock *id_block, void *handle);

/*
 * Make certain current choices take effect immediately.
 */
void action_choices(Choices *const c);

#endif
