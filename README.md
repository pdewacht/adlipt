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

    ADLIPT LPT1

assuming the OPL2LPT board is plugged into LPT1. It will use about
half a kilobyte of RAM. It can be loaded into high memory using the
DOS `LH` command.

Once loaded it cannot be unloaded, but it can be temporarily disabled
with the command:

    ADLIPT DISABLE

Re-enable with:

    ADLIPT ENABLE

### JEMM

Load ADLiPT using the command:

    JLOAD JADLIPT.DLL LPT1


## Requirements

- 386 CPU
- EMM386, QEMM or JEMM
    - EMM386 version 4.46 or later (tested with version 4.49 from
      MS-DOS 6.22)
    - QEMM 7.03 or later (tested with QEMM 7.5)
    - Support for other memory managers could be added if there's
      interest.
- The OPL2LPT board ;)

## Building

Compiles with OpenWatcom. Use the `build.sh` script.

## Copying

Copyright © 2017 Peter De Wachter (pdewacht@gmail.com)

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
