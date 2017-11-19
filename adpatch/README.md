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
