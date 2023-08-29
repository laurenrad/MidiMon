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
 * Application: N/A
 * File: preporter.c
 * Version: 1.0.1
 * Author: Lauren Rad
 * Purpose: I had trouble with all the existing C libraries for reporter, so
 * this file contains quick and dirty wrapper functions for Reporter SWIs.
 */

#include <string.h>
#include "kernel.h"
#include "swis.h"
#include "preporter.h"

char _reporter_buf[BUFSIZE];

/* Print a string to Reporter. */
void report_text0(char *s)
{
    try_reporter(_swix(Report_Text0, _IN(0), s));
}

/* Print registers to Reporter. */
void report_regs(void)
{
    _swi(Report_Regs, 0);
}

/* Print Where info to Reporter. */
void report_where(void)
{
    _swi(Report_Where, 0);
}

/* Print WimpPoll info to Reporter. */
void report_poll(int reason_code)
{
    _swi(Report_Poll, _IN(0), reason_code);
}

/* Print memory dump to Reporter. */
void report_dump(int addr, int len, int width, char *text)
{
    _swi(Report_Dump, _INR(0, 3), addr, len, width, (int)text);
}

/* Set on/off options, including:
On, Cmd, Time, Srce, Obey, Err, Task, Vdu4, Rma
0 = off, nonzero = on
*/
void report_opt(Report_Opts opt, int val)
{
    switch (opt) {
    case On:
        if (val)
            _swi(Report_On, 0);
        else
            _swi(Report_Off, 0);
        break;
    case Cmd:
        if (val)
            _swi(Report_CmdOn, 0);
        else
            _swi(Report_CmdOff, 0);
        break;
    case Time:
        if (val)
            _swi(Report_TimeOn, 0);
        else
            _swi(Report_TimeOff, 0);
        break;
    case Srce:
        if (val)
            _swi(Report_SrceOn, 0);
        else
            _swi(Report_SrceOff, 0);
        break;
    case Obey:
        if (val)
            _swi(Report_ObeyOn, 0);
        else
            _swi(Report_ObeyOff, 0);
        break;
    case Err:
        if (val)
            _swi(Report_ErrOn, 0);
        else
            _swi(Report_ErrOff, 0);
        break;
    case Task:
        if (val)
            _swi(Report_TaskOn, 0);
        else
            _swi(Report_TaskOff, 0);
        break;
    case Vdu4:
        if (val)
            _swi(Report_Vdu4On, 0);
        else
            _swi(Report_Vdu4Off, 0);
        break;
    case Rma:
        if (val)
            _swi(Report_RmaOn, 0);
        else
            _swi(Report_RmaOff, 0);
        break;
    default:
        report_text0("report_opt: unknown option");
        break;
    }

}

/* Hide the Reporter window. */
void report_hide(void)
{
    _swi(Report_Hide, 0);
}

/* Show the Reporter window. */
void report_show(void)
{
    _swi(Report_Show, 0);
}

/* Clear Reporter. The *command has a text option but it seems to be ignored
 from SWI, at least trying in R0 since it's not documented. */
void report_clear(void)
{
    _swi(Report_Clear, 0);
}

/* Close Reporter window. This doesn't seem to work... */
void report_close(void)
{
    _swi(Report_Close, 0);
}

/* Open Reporter window. */
void report_open(void)
{
    _swi(Report_Open, 0);
}

/* Push options stack. */
void report_push(void)
{
    _swi(Report_Push, 0);
}

/* Pull options stack. */
void report_pull(void)
{
    _swi(Report_Pull, 0);
}

/* Quit Reporter. */
void report_quit(void)
{
    _swi(Report_Quit, 0);
}

/* Wrapper to ignore if Reporter isn't running */
_kernel_oserror *try_reporter(_kernel_oserror *err)
{

    if (err != NULL && (err->errnum != Error_UnknownSWI)) {
        /*
         * oop a real error
         */
        report_printf("error: %d: %s\n", err->errnum, err->errmess);
    }

    return err;
}
