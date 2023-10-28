# MidiMon

## Basic Info

MidiMon is an application for monitoring incoming MIDI messages, as well as
sending test messages. See the included !Help file for full details.

MidiMon is generally designed to work with any MIDI module which implements the legacy 
Acorn MIDI module interface; however, it is only tested with the USB-MIDI module by Rick
Murray[^1] v0.10 or higher, or with Peter Everett's USB-enabled MIDI Support System[^2].
If you have any other MIDI modules or drivers, please feel free to reach out to me and let
me know how this works, keeping in mind the hardware support may vary by module.

[^1]: This module can currently be obtained from [Rick Murray's
website](https://heyrick.eu/blog/index.php?diary=20230319).

[^2]: This module can currently be obtained from [Pete's website](http://www.forever.onmypc.net).

## Latest Release

The latest release is v1.50. See the CHANGELOG file for changes, and check "Releases" on the GitHub page for binary builds.

## Licensing

MidiMon is free software, distributed under the terms of the Apache License,
version 2.0. See the included LICENSE file for more information.

## Building

MidiMon uses the User Interface Toolbox and is packaged to be built with the
ROOL DDE. A binary build is available.


## Getting Help

For more information, see the included file "!Help" in the application bundle
or in this repository. As is standard, on RISC OS this file can be viewed by
clicking MENU on the app bundle, then navigating to App "!MidiMon", then
selecting Help.
