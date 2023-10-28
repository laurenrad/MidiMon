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
  * File: snippets.c
  * Author: Lauren Rad
  * Purpose: This is just a pastebin for any code which isn't hooked up and
  * I don't want littering the actual source, but I want to keep around for
  * documentation or future debug purposes.
  */


/* Clear the Rx Buffer. Returns 0 on success, or the error number of any
   _kernel_oserror detected. */
/*
 * drain_rx_buf
 * This is some old code that's being kept around for debug purposes,
 * in case there are any other modules that need to be implemented.
 * This will just keep reading from the buffer with RxCommand until
 # the buffer is cleared.
 */
#define RX_BUFSIZE 2044 // receive buffer size
int drain_rx_buf(int device)
{
    int buf_free, error_code, command;
    int buf_last_free; // last known buffer free space. so we can stop if clearing isn't working.
    _kernel_oserror *err;

    /*
     * Since this is called before entering the polling loop, check that
     * everything is ok. For now, at least checking here should ensure that
     * the MIDI module is loaded before startup, otherwise a SWI not known
     * error will be returned here.
     */
    err = _swix(MIDI_InqError, _OUT(0), &error_code);
    if (err != NULL) {
        return err->errnum;
    }
    if (error_code == 'B') {
        report_printf("MidiMon: Receive buffer full!");
    }

    /*
     * Prior to 0.08, USB-MIDI had a bug where MIDI_InqBufferSize returned
     * the buffer size rather than the number of unused buffer bytes. This
     * appears to have now been fixed, but if written like this it will
     * require 0.08.
     * Note that for this SWI, devices are numbered from 0 (0-3) rather than 1
     */

    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), device << 1, &buf_free);
    buf_last_free = buf_free;
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: clear_rx_buf: before: device %d buffer free %d", device, buf_free);
#endif

    int i = 0;
    while (buf_free < RX_BUFSIZE) {
        _swi(MIDI_RxCommand, _IN(0) | _OUT(0), device, &command);
        _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), device << 1, &buf_free);
        i++;
        report_printf("MidiMon: clear_rx_buf: during: device %d buffer free %d last %d",device,buf_free,buf_last_free);

        if (buf_free == buf_last_free) {
            report_printf("MidiMon: err: can't empty buffer!");
            break; //return -1;          // quit if the buffer isn't clearing to prevent hanging
        }
    }
    report_printf("ran through loop %d times trying to empty buffer",i);


    _swi(MIDI_InqBufferSize, _IN(0) | _OUT(0), device << 1, &buf_free);
    buf_last_free = buf_free;
#ifdef REPORTER_DEBUG
    report_printf("MidiMon: clear_rx_buf: after: device %d buffer free %d", device, buf_free);
#endif

    return 0;
}
