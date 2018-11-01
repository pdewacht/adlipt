# ADLiPT

ADLiPT is a driver for the "Adlib on a parallel port" OPL2LPT board.
For more information, see [the thread on the Vogons forum][1].

[1]: https://www.vogons.org/viewtopic.php?f=62&t=55105

## Download

Download the driver from [the Github releases tab][2].

[2]: https://github.com/pdewacht/adlipt/releases/latest

## Usage

### EMM386 or QEMM

Load the TSR with the command:

    ADLIPT

assuming the OPL2LPT board is plugged into LPT1. It will use about a
kilobyte of RAM. It can be loaded into high memory using the DOS `LH`
command.

The TSR can be unloaded with the command:

    ADLIPT UNLOAD

### JEMM

Load ADLiPT using the command:

    JLOAD JADLIPT.DLL

### Options

- **`OPL3`** Enable OPL3 support. Use this if you have an OPL3LPT.

- **`LPT1`/`LPT2`/`LPT3`** Select printer port.

- **`BLASTER=220`** Enable Sound Blaster FM emulation. ADLiPT will
  intercept the Sound Blaster FM ports in addition to the standard
  Adlib ports. (It won't however fake enough of a Sound Blaster to
  pass installation checks, so this won't be very useful if you don't
  have a Sound Blaster.)

- **`NOPATCH`** Disable runtime patching. Without runtime patching the
  TSR will be much slower, but the I/O timing will be much more
  regular, which might help with software that's very timing
  sensitive.


## System requirements

- 386 CPU
- A compatible memory manager:
  - EMM386 version 4.46 or later (tested with version 4.49 from MS-DOS 6.22)
  - QEMM 7.03 or later (tested with QEMM versions 7.03, 7.5, 8.0, 9.01)
  - JEMM 5.78

EMS memory is _not_ required. If you don't use software that requires
EMS memory, you can load EMM386 with the `NOEMS` parameter.

## Source code

This program compiles with OpenWatcom, use the `build.sh` script.
You'll also need Python 3 and [the Ragel state machine
compiler][Ragel].

[Ragel]: https://www.colm.net/open-source/ragel/

## Copying

Copyright © 2017, 2018 Peter De Wachter (pdewacht@gmail.com)

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
