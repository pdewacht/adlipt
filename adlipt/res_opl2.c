#pragma code_seg("RESIDENT")
#pragma data_seg("RESIDENT", "CODE")

#include "resident.h"


struct config RESIDENT config;


/* I/O port access */

extern unsigned inp(unsigned port);
extern unsigned outp(unsigned port, unsigned value);
#pragma intrinsic(inp, outp)
#pragma aux inp modify nomemory;
#pragma aux outp modify nomemory;

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

unsigned emulate_adlib_io(int port, int is_write, unsigned ax)
{
  if (is_write) {
    char value = ax;
    char c = PP_INIT | PP_NOT_SELECT;

    if ((char)port == 0x88) {
      /* Write to address register */
      c |= PP_NOT_STROBE;
      /* Remember the address */
      address = value;
    }
    else {
      /* Write to data port */
      /* Remember the timer setting */
      if (address == 4) {
        timer_reg = value;
      }
    }

    outp(config.lpt_port, value);  /* Prepare data */
    outp(config.lpt_port + 2, c);  /* Select address */
    c ^= PP_INIT;
    outp(config.lpt_port + 2, c);  /* Begin write */
    c ^= PP_INIT;
    outp(config.lpt_port + 2, c);  /* Complete write */
  }
  else {
    /* Read. */

    if ((char)port == 0x88) {
      /*
       * Emulate the timers. We let them expire instantaneously.
       * This seems good enough for the standard Adlib detection
       * routines.
       */
      char s = STATUS_LOW_BITS;
      if ((timer_reg & 0xC1) == 1) {
        s |= 0xC0;
      }
      if ((timer_reg & 0xA2) == 2) {
        s |= 0xA0;
      }
      ax = (ax & ~0xFF) | s;
    }

    /* Do a dummy I/O action for timing */
    (volatile) inp(config.lpt_port + 2);
  }

  #ifdef DEBUG
  writechar(is_write ? 'W' : 'R');
  writechar((char)port == 0x88 ? 'a' : 'd');
  write2hex(ax);
  writeln();
  #endif

  return ax;
}
