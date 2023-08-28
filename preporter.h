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

/***
File: preporter.h
Author: Lauren Rad
Version: 1.0.1
Description: I had trouble with all the existing C libraries for Reporter, so
here are some quick and dirty wrapper functions for Reporter SWIs.
***/

#ifndef PREPORTER_H
#define PREPORTER_H

#include <stdio.h>
#include "kernel.h"

#define BUFSIZE 1000
static char _reporter_buf[BUFSIZE];

#define Error_UnknownSWI        0x1E6   // Error for try_reporter

/* Define SWI numbers - not all are implemented*/
/* Reporting SWIs */
#define Report_Text0		0x054C80
#define Report_TextS		0x054C81
#define Report_Regs		0x054C82
#define Report_Registers	0x054C83
#define Report_Where		0x054C84
#define Report_Poll		0x054C85
#define Report_Dump		0x054C86
#define Report_GetSwiRet	0x054C87
#define Report_ErrBlk		0x054C88

/* Controlling SWIs */
#define Report_Quit		0x054C8A
#define Report_Clear		0x054C8B
#define Report_Open		0x054C8C
#define Report_Close		0x054C8D
#define Report_On		0x054C8E
#define Report_Off		0x054C8F
#define Report_CmdOn		0x054C90
#define Report_CmdOff		0x054C91
#define Report_Hide		0x054C92
#define Report_Show		0x054C93
#define Report_ErrOn		0x054C94
#define Report_ErrOff		0x054C95
#define Report_TaskOn		0x054C96
#define Report_TaskOff		0x054C97
#define Report_Vdu4On		0x054C98
#define Report_Vdu4Off		0x054C99
#define Report_RmaOn		0x054C9A
#define Report_RmaOff		0x054C9B
#define Report_TimeOn		0x054C9C
#define Report_TimeOff		0x054C9D
#define Report_SrceOn		0x054C9E
#define Report_SrceOff		0x054C9F
#define Report_ObeyOn		0x054CA0
#define Report_ObeyOff		0x054CA1
#define Report_Push		0x054CA2
#define Report_Pull		0x054CA3
#define Report_Pause		0x054CA4
#define Report_Scroll		0x054CA5
#define Report_SaveOn		0x054CA6
#define Report_SaveOff		0x054CA7
#define Report_LogOn		0x054CA8
#define Report_LogOff		0x054CA9

/* Option names for use with report_opt */
typedef enum Report_Opts { On, Cmd, Time, Srce, Obey, Err, Task, Vdu4, Rma } Report_Opts;

/* each of these functions calls the SWI of the same name */
void report_text0(char *s);
void report_regs(void);
void report_where(void);
void report_poll(int reason_code);
void report_dump(int addr, int len, int width, char *text);

void report_opt(enum Report_Opts opt, int val);
void report_hide(void);
void report_show(void);
void report_clear(void);
void report_close(void);
void report_open(void);
void report_push(void);
void report_pull(void);
void report_quit(void);

/* Wrapper func to filter for Reporter not being loaded */
_kernel_oserror *try_reporter(_kernel_oserror *err);

/* this macro acts like printf for Reporter */
#define report_printf(...) \
snprintf(_reporter_buf,BUFSIZE,__VA_ARGS__); \
report_text0(_reporter_buf);

#endif
