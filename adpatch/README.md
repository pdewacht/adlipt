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
- No Adlib emulation slowdown
- Can work with protected mode games

Cons:

- Patches need to be written for each game by hand

## Working games

- Sierra
  - Games using the "SCI" engine. Patch `ADL.DRV`.
- id Software
  - The Commander Keen series
  - Wolfenstein 3D
  - DOOM (v1.9 tested)
  - DOOM II
- Softdisk
  - Keen Dreams
  - Dangerous Dave's Risky Rescue
  - Dave Does Nutz
- Apogee
  - Bio Menace

Games not listed almost certainly will not work (sorry).
Patches welcome :)

## Compilation

To compile a DOS executable using OpenWatcom, use the `build.sh`
script. To compile with GCC, use the `buildunix.sh` script.

You'll also need Python 3, the NASM assembler and [the Ragel state
machine compiler][Ragel].

[Ragel]: https://www.colm.net/open-source/ragel/

## Writing patches

The patches are stored in YAML files in the `patterns/` directory.
Each file can have multiple patches, I try to keep related patches
together. Generally each target needs two patches, one for the Adlib
detection routine and one for the actual Adlib I/O.

Each patch consists of a search pattern and a replacement code fragment.

### Search patterns

The search pattern is specified as an assembly fragment, but there are
certain special keywords that can be used.

**ANYBYTE, ANYWORD, ANYDWORD** can be used as a wildcard for an
arbitrary 1/2/4 byte value. For example:

    mov dx, word ptr [ANYWORD]
    out dx, al

In this pattern, the address of the variable is not important and
might differ in different builds of the game. So it's best to accept
anything.

**BEGIN/ALT/END** mark alternative code sequences. For example:

    BEGIN
      mov dx, 0x389
    ALT
      inc dx
    END

This is useful to match slight variations in a single pattern.

**BEGIN/REPEAT** marks possible repetition. For example:

    BEGIN
      in al, dx
    REPEAT

The fragment must appear at least once.

**BEGIN/OPTION** marks a fragment that might or might not appear.

    BEGIN
      nop
    OPTION

#### Problems:

ANYBYTE/ANYWORD won't work for relative offsets. So instead of `call
ANYWORD`, you'll have to write `db 0xE8, ANYBYTE, ANYBYTE`. Same for
short jumps.

Surprisingly often x86 instructions have multiple encodings. For
example, `mov ax,bx` might be encoded as `89 D8` or as `8B C3`. If
NASM picks the wrong encoding, you'll have to spell out the bytes
explicitly.

The implementation is a gross hack.

### Replacement code

The replacement code is an ordinary assembly fragment. The keyword
PORT can be used *once* to get the parallel port I/O address.

ADPATCH ships with a few standard replacements that can be `%include`d:

- `standard.s`: the standard OPL2LPT code. Expects the OPL2 register
  in AL and the data in AH.
- `standard32.s`: same, as 32-bit code.
- `compact.s`: a slower but smaller version. Will pop register and data
  (separately) from the stack.

If the replacement code is shorter than the matched fragment, it will
be padded with NOPs.
