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

#define DO_LPT(value, flags, delay)             \
  do {                                          \
    unsigned port = config.lpt_port;            \
    int i;                                      \
    outp(port, (value));                        \
    port += 2;                                  \
    outp(port, (flags));                        \
    outp(port, (flags) ^ PP_INIT);              \
    outp(port, (flags));                        \
    delay(port);                                \
  } while (0)

unsigned emulate_adlib_address_io(int port, int is_write, unsigned ax, char __far *next_opcode) {
  if (is_write) {
    address = ax;
    DO_LPT(ax, PP_INIT | PP_NOT_SELECT | PP_NOT_STROBE, address_delay);
    return ax;
  }
  else {
    char s = 0;
    char t = timer_reg;
    if ((t & 0xC1) == 1) {
      s |= 0xC0;
    }
    if ((t & 0xA2) == 2) {
      s |= 0xA0;
    }

    if (s == 0 && next_opcode != 0 && next_opcode[-1] == 0xEC) {
      next_opcode[-1] = 0x90;
    }

    return (ax & 0xFF00) | s | STATUS_LOW_BITS;
  }
}

unsigned emulate_adlib_data_io(int port, int is_write, unsigned ax) {
  if (is_write) {
    if (address == 4) {
      timer_reg = ax;
    }
    DO_LPT(ax, PP_INIT | PP_NOT_SELECT, data_delay);
  }
  return ax;
}
