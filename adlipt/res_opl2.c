#include <stddef.h>

#pragma code_seg("RESIDENT")
#pragma data_seg("RESIDENT", "CODE")

#include "resident.h"


struct config RESIDENT config;


/* I/O port access */

static void outp(unsigned port, char value);
#pragma aux outp =                              \
  "out dx, al"                                  \
  parm [dx] [al]                                \
  modify exact [ax]

static char inp(unsigned port);
#pragma aux inp =                               \
  "in al, dx"                                   \
  parm [dx]                                     \
  modify exact [ax]

static void address_delay(unsigned port);
#pragma aux address_delay =                     \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  parm [dx]                                     \
  modify exact [ax]

static void data_delay(unsigned port);
#pragma aux data_delay =                        \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  "in al, dx"                                   \
  parm [dx]                                     \
  modify exact [ax]

#define PP_NOT_STROBE   0x1
#define PP_NOT_AUTOFD   0x2
#define PP_INIT         0x4
#define PP_NOT_SELECT   0x8
// STROBE (inverted) = address bit 0
// SELECT (inverted) = address bit 1
// INIT = when low, write
// AUTOFD (inverted) = when low, read


/* Debug output */

static void writechar(char x) {
  int COM1 = 0x03F8;
  outp(COM1, x);
}

static void writeln(void) {
  writechar('\r');
  writechar('\n');
}

static void writehex(char x) {
  x = (x & 0xf) + '0';
  if (x > '9') {
    x += 'A' - '9' - 1;
  }
  writechar(x);
}

static void write2hex(char x) {
  writehex(x >> 4);
  writehex(x);
}

static void write4hex(int x) {
  write2hex(x >> 8);
  write2hex(x);
}


/* I/O virtualization */

static char address;
static char timer_reg;

#define STATUS_LOW_BITS 0x06

static unsigned emulate_read(unsigned ax, char _WCI86FAR *next_opcode);
#pragma aux emulate_read parm [ax] [bx dx] modify exact [ax bx cx dx]

#define DO_LPT(value, flags, delay)             \
  do {                                          \
    unsigned port = config.lpt_port;            \
    int i;                                      \
    outp(port, (value));                        \
    port += 2;                                  \
    outp(port, (flags));                        \
    outp(port, (flags) ^ PP_INIT);              \
    outp(port, (flags));                        \
    if (config.enable_patching) {               \
      delay(port);                              \
    }                                           \
  } while (0)

unsigned emulate_opl2_address_io(int port, int is_write, unsigned ax, char _WCI86FAR *next_opcode) {
  if (!is_write) {
    return emulate_read(ax, next_opcode);
  }
  address = ax;
  DO_LPT(ax, PP_INIT | PP_NOT_SELECT | PP_NOT_STROBE, address_delay);
  return ax;
}

unsigned emulate_opl2_data_io(int port, int is_write, unsigned ax, char _WCI86FAR *next_opcode) {
  if (!is_write) {
    return emulate_read(ax, next_opcode);
  }
  if (address == 4) {
    timer_reg = ax;
  }
  DO_LPT(ax, PP_INIT | PP_NOT_SELECT, data_delay);
  return ax;
}

static unsigned emulate_read(unsigned ax, char _WCI86FAR *next_opcode) {
  /*
   * Emulate the timers. We let them expire instantaneously.
   * So far I haven't found any software that uses the timers for
   * anything other than a detection routine.
   */
  ax = ax & ~0xFF;
  if ((timer_reg & 0xC1) == 1) {
    ax |= 0xC0;
  }
  if ((timer_reg & 0xA2) == 2) {
    ax |= 0xA0;
  }
  ax |= STATUS_LOW_BITS;

  if (config.enable_patching) {
    /*
     * Typical Adlib routines use dummy IN AL,DX instructions to
     * provide a delay. This hurts because each of these INs generates
     * a fault and especially on slower CPUs the delay will be much
     * much longer than intended.
     *
     * We can work around this by replacing the INs with NOPs and
     * compensate by adding delay code to our OUT emulation (which
     * will not fault).
     *
     * We don't want to break Adlib detection routines though, there
     * the INs are actually meaningful. So only patch if a register is
     * selected that wouldn't be used in such a routine.
     */
    if (address >= 0x20) {
      char _WCI86FAR *opc = next_opcode - 1;
      if (*opc == 0xEC) {
        *opc = 0x90;
      }
    }
  }

  if (config.cpu_type >= 5) {
    /*
     * Because INs are used for timing, the emulation must be at least
     * as slow as the real thing. Assume that for 386 and 486 CPUs
     * that will always be the case. On Pentium systems we do an
     * explicit I/O to be sure.
     */
    (volatile) inp(config.lpt_port);
  }

  return ax;
}
