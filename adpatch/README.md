# ADPATCH

ADPATCH can patch games to directly support the OPL2LPT. After
patching, configure the game for an Adlib sound card (in case this
isn't auto-detected).

Caveats:

- Patching can break sound card support (including support for
  non-Adlib cards).
- Patched games might only work on the computer where the patch was
  done. This is because the patch hardcodes the parallel port I/O
  address, which I believe might vary from computer to computer.

Pros compared to the ADLiPT TSR:

- No need for 386 CPU
- Can work with protected mode games

Cons:

- Patches need to be written for each game by hand

## Compressed EXEs

DOS software often shipped with compressed EXEs. Such EXEs need to be
unpacked before they can be patched. ADPATCH will detect the more
common compression schemes and warn about them ("This file might be
compressed"). If you see such a warning, you should try decompressing
it and running ADPATCH again. I recommend [Ben Castricum's
UNP](http://unp.bencastricum.nl/) decompressor, it will handle pretty
much all formats.

## Working games

A list of tested games is maintained
[here](https://github.com/pdewacht/adlipt/wiki/ADPATCH-Tested-Games).

## Compilation

To compile a DOS executable using OpenWatcom, use the `build.sh`
script. To compile with GCC, use the `buildunix.sh` script.

You'll also need Python 3, the NASM assembler and [the Ragel state
machine compiler][Ragel].

[Ragel]: https://www.colm.net/open-source/ragel/

## Writing patches

See [the article here](https://github.com/pdewacht/adlipt/wiki/Making-ADPATCH-patches).
