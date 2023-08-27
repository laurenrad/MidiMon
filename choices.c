/*
 * Copyright 2023 Lauren Rad
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
 * File: choices.c
 * Author: Lauren Rad
 * Purpose: Non-Wimp functions for saving/loading the Choices file.
 */

#include <string.h>
#include <stdlib.h>

/* Choicey headers */
#include "choices.h"
#include "preporter.h"
#include "common.h"

/*
 * Set up initial choices.
 * These defaults are found in choices.h.
 */
Choices init_choices(void)
{
    Choices c;
    c.choices_ver = DEFAULTS_CHOICES_VER;
    c.opt_txchan = DEFAULTS_OPT_TXCHAN;
    c.opt_altnoteoff = DEFAULTS_OPT_ALTNOTEOFF;
    c.opt_ignoreclock = DEFAULTS_OPT_IGNORECLOCK;
    c.opt_fakefastclock = DEFAULTS_OPT_FAKEFASTCLOCK;

    return c;
}

int save_choices(void)
{
    char choices_write_path[MAX_PATHNAME]; // value of env var Choices$Write
    FILE *choices_file;

    /*
     * Multiple calls to getenv will clobber the pointers, so these need to be
     * copied somewhere safe. And while we're at it, make sure they were set.
     */
    if (getenv("Choices$Write") != NULL) {
        snprintf(choices_write_path,MAX_PATHNAME,"%s.",getenv("Choices$Write"));
    } else {
        report_printf("MidiMon: Err: Choices$Write not set."); // TBD: real error
        return 1;
    }

    report_printf("MidiMon: Choices$Write is: %s",choices_write_path);
    strncat(choices_write_path,LEAFNAME,MAX_PATHNAME - strlen(choices_write_path));

    choices_file = fopen(choices_write_path,"wb");
    if (choices_file == NULL) {
        report_printf("MidiMon: Err: Unable to open Choices file for writing");
        return 2;
    }

    if (fwrite(&global_choices, sizeof(global_choices), 1, choices_file) < 1) {
        report_printf("MidiMon: Error writing to choices file");
    } else {
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Choices file written.");
#endif
    }

    fclose(choices_file);

    return 0;
}

/* Do all the dirty work of loading choices, creating defaults, etc */
int load_choices(void)
{
    /*
     * Multiple calls to getenv will clobber the pointers, so these need to be
     * copied somewhere safe. And while we're at it, make sure they were set.
     */
    char choices_path_env[MAX_PATHNAME];

    if (getenv("Choices$Path") != NULL) {
        snprintf(choices_path_env,MAX_PATHNAME,"%s",getenv("Choices$Path"));
    } else {
        report_printf("MidiMon: Err: Choices$Path not set."); // TBD real error
        return 1;
    }

#ifdef REPORTER_DEBUG
    report_printf("MidiMon: Choices$Path is: %s",choices_path_env);
#endif
    FILE *choices_file = openin_choices(choices_path_env);
    if (choices_file == NULL) {
        /*
         * And if this didn't work, just generate the defaults in our instance.
         * No need to save it as it's just the defaults, that will be handled if
         * something changes.
         */
        global_choices = init_choices();
        report_printf("MidiMon: Choices file not found. Using defaults.");
    } else {
        if(fread(&global_choices,sizeof(global_choices),1,choices_file) < 1) {
            report_printf("MidiMon: Err: Opened Choices file but wasn't able to read it.");
            global_choices = init_choices(); /* just throw it out */
        } else {
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Successfully read Choices file");
#endif
        }

        fclose(choices_file);
    }

    return 0;
}

/*
 * Unlike Choices$Write, Choices$Path can contain multiple locations.
 * This function keeps trying them and returns first successful, or returns NULL.
 * I'm not sure if I'm interpreting the Acorn style guide correctly on this, but it
 * seems sensible.
 */
FILE *openin_choices(char *choices_path_env)
{
    char *current_path = strtok(choices_path_env,",");
    FILE *choices_file;
    char fullname[MAX_PATHNAME];

    while (current_path != NULL) {
        snprintf(fullname,MAX_PATHNAME,"%s%s",current_path,LEAFNAME);
        choices_file = fopen(fullname,"rb");
        if (choices_file != NULL) {
#ifdef REPORTER_DEBUG
        report_printf("MidiMon: Selected choices path %s",fullname);
#endif
        return choices_file;
        }
        current_path = strtok(NULL,",");
    }

    return NULL;
}
