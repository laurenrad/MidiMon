# MidiMon

## Basic Info

MidiMon is an application for monitoring incoming MIDI messages, as well as
sending test messages. This documentation will go over installation as well
as the various components and features.

MidiMon is designed to work with the USB-MIDI module by Rick Murray[^1],
version 0.08 or higher. In its current state it is designed only to work with
this module and not the legacy Acorn MIDI module, as I have no way of
accessing the legacy hardware or even its documentation. However, if anyone
wants, it hypothetically could be ported over to use the legacy module with
some changes as the API for USB-MIDI is similar. 

[^1] The module can currently be obtained from [Rick Murray's
website](https://heyrick.eu/blog/index.php?diary=20230319).

## Licensing

MidiMon is free software, distributed under the terms of the Apache License,
version 2.0. See the included LICENSE file for more information.

## Building

MidiMon uses the User Interface Toolbox and is packaged to be built with the
ROOL DDE.


## Getting Help

For more information, see the included file "!Help" in the application bundle
or in this repository. As is standard, on RISC OS this file can be viewed by
clicking MENU on the app bundle, then navigating to App "!MidiMon", then
selecting Help.