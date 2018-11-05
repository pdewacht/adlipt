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


/* I/O dispatching */

static porthandler * const adlib_table[4][2] = {
  { emulate_opl2_read, emulate_opl2_write_address },
  { emulate_opl2_read, emulate_opl2_write_data },
  { emulate_opl3_read, emulate_opl3_write_high_address },
  { emulate_opl3_read, emulate_opl3_write_data }
};

static porthandler * const sb_table[16][2] = {
  { emulate_opl3_read, emulate_opl3_write_low_address },
  { emulate_opl3_read, emulate_opl3_write_data },
  { emulate_opl3_read, emulate_opl3_write_high_address },
  { emulate_opl3_read, emulate_opl3_write_data },
  { emulate_ignore, emulate_ignore },
  { emulate_sbdsp_read_data, emulate_sbmixer_data_write },
  { emulate_ignore, emulate_sbdsp_reset },
  { NULL, NULL },
  { emulate_opl2_read, emulate_opl2_write_address },
  { emulate_opl2_read, emulate_opl2_write_data },
  { emulate_sbdsp_read_data, emulate_ignore },
  { NULL, NULL },
  { emulate_sbdsp_write_buffer, emulate_sbdsp_write_data },
  { NULL, NULL },
  { emulate_sbdsp_data_avail, emulate_ignore },
  { NULL, NULL }
};

porthandler *get_port_handler(unsigned port, unsigned flags) {
  porthandler * const (*table)[2];
  int base;
  if ((base = (port & ~0x3)) == 0x388) {
    table = adlib_table;
  }
  else if (config.sb_base && (base = (port & ~0xF)) == config.sb_base) {
    table = sb_table;
  }
  else {
    return 0;
  }
  return table[port & ~base][(flags >> 2) & 1];
}


/* I/O virtualization */

#define STATUS_OPL2_LOW_BITS 0x06
#define STATUS_OPL3_LOW_BITS 0x00

#define DELAY_OPL2_ADDRESS 6
#define DELAY_OPL2_DATA 35
#define DELAY_OPL3 6

char _WCI86FAR * RESIDENT port_trap_ip;

static short address;
static char timer_reg;


#define WRITE_LPT(value, flags)                 \
  do {                                          \
    unsigned port = lpt_port;                   \
    outp(port, (value));                        \
    port += 2;                                  \
    outp(port, (flags));                        \
    outp(port, (flags) ^ PP_INIT);              \
    outp(port, (flags));                        \
  } while (0)


#ifdef _M_I86
#pragma aux delay parm [dx] [bx] modify exact [ax bx]
#pragma aux cond_delay parm [dx] [bx] modify exact [ax bx]
#else
#pragma aux delay parm [edx] [ebx] modify exact [eax ebx]
#pragma aux cond_delay parm [edx] [ebx] modify exact [eax ebx]
#endif

static void delay(unsigned port, char cnt) {
  do {
    (volatile) inp(port);
  } while (--cnt);
}

static void cond_delay(unsigned port, char cnt) {
  if (config.enable_patching) {
    delay(port, cnt);
  }
}


unsigned emulate_opl2_write_address(unsigned ax) {
  unsigned lpt_port = config.lpt_port;
  address = ax & 0xFF;
  WRITE_LPT(ax, PP_INIT | PP_NOT_SELECT | PP_NOT_STROBE);
  cond_delay(lpt_port, DELAY_OPL2_ADDRESS);
  return ax;
}

unsigned emulate_opl2_write_data(unsigned ax) {
  static const char delay_cnt[2] = { DELAY_OPL2_DATA, DELAY_OPL3 };
  unsigned lpt_port = config.lpt_port;
  if (address == 4) {
    timer_reg = ax;
  }
  WRITE_LPT(ax, PP_INIT | PP_NOT_SELECT);
  cond_delay(lpt_port, delay_cnt[config.opl3]);
  return ax;
}

unsigned emulate_opl3_write_low_address(unsigned ax) {
  if (!config.opl3) {
    return ax;
  }
  return emulate_opl2_write_address(ax);
}

unsigned emulate_opl3_write_high_address(unsigned ax) {
  unsigned lpt_port = config.lpt_port;
  if (!config.opl3) {
    return ax;
  }
  address = 0x100 | (ax & 0xFF);
  WRITE_LPT(ax, PP_INIT | PP_NOT_STROBE);
  cond_delay(lpt_port, DELAY_OPL3);
  return ax;
}

unsigned emulate_opl3_write_data(unsigned ax) {
  if (!config.opl3) {
    return ax;
  }
  return emulate_opl2_write_data(ax);
}

unsigned emulate_opl2_read(unsigned ax) {
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
  if (config.opl3) {
    ax |= STATUS_OPL3_LOW_BITS;
  } else {
    ax |= STATUS_OPL2_LOW_BITS;
  }

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
    if ((address & 0xFF) >= 0x20) {
      char _WCI86FAR *opc = port_trap_ip - 1;
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

unsigned emulate_opl3_read(unsigned ax) {
  if (!config.opl3) {
    return 0;
  }
  return emulate_opl2_read(ax);
}


#pragma code_seg("CODE")

void hw_reset(unsigned lpt_port) {
  int i;
  for (i = 0; i < 256; i++) {
    /* Clear register i */
    WRITE_LPT(i, PP_INIT | PP_NOT_SELECT | PP_NOT_STROBE);
    delay(lpt_port, DELAY_OPL2_ADDRESS);
    WRITE_LPT(0, PP_INIT | PP_NOT_SELECT);
    delay(lpt_port, DELAY_OPL2_DATA);
    /* Clear register 0x100+i -- harmless on OPL2LPT */
    WRITE_LPT(i, PP_INIT | PP_NOT_STROBE);
    delay(lpt_port, DELAY_OPL2_ADDRESS);
    WRITE_LPT(0, PP_INIT | PP_NOT_SELECT);
    delay(lpt_port, DELAY_OPL2_DATA);
  }
}

#pragma code_seg("RESIDENT")


/* Sound Blaster... */

enum sb_write_state {
  SB_IDLE,
  SB_IDENT,
};

static enum sb_write_state sb_write_state = SB_IDLE;
static char sb_read_buf[2] = { 0xAA, 0xAA };

unsigned emulate_ignore(unsigned ax) {
#ifdef DEBUG
  writechar('I');writechar('G');writechar('N');writeln();
#endif
  return ax;
}

unsigned emulate_sbdsp_reset(unsigned ax) {
#ifdef DEBUG
  writechar('R');writechar('S');writechar('T');writeln();
#endif
  sb_write_state = SB_IDLE;
  sb_read_buf[0] = sb_read_buf[1] = 0xAA;
  return ax;
}

unsigned emulate_sbdsp_read_data(unsigned ax) {
  /* Output first byte, then repeat second byte */
  char result = sb_read_buf[0];
#ifdef DEBUG
  writechar('R');writechar('D');writechar(' ');write2hex(result);writeln();
#endif
  sb_read_buf[0] = sb_read_buf[1];
  return (ax & ~0xFF) | result;
}

void int0f();
#ifdef _M_I86
#pragma aux int0f = "int 0x0f" modify exact []
#endif

unsigned emulate_sbdsp_write_data(unsigned ax) {
#ifdef DEBUG
  writechar('W');writechar('R');writechar(' ');write2hex(ax);writeln();
#endif
  switch (sb_write_state) {
  case SB_IDENT: {
    char reply = ~ax;
    sb_read_buf[0] = sb_read_buf[1] = reply;
    sb_write_state = SB_IDLE;
    break;
  }
  default:
    switch ((char)ax) {
    case 0xE0:  /* DSP Identification */
      /* Other commands where we just want to skip a byte: */
    case 0x10:  /* Direct DAC 8-bit */
    case 0x38:  /* MIDI Write Poll */
    case 0x40:  /* Set Time Constant */
      sb_write_state = SB_IDENT;
      break;
    case 0xE1:  /* DSP Version */
      if (config.opl3) {
        sb_read_buf[0] = 3;
        sb_read_buf[1] = 2;
      } else {
        sb_read_buf[0] = 1;
        sb_read_buf[1] = 5;
      }
      break;
    case 0xF2:
      // This is crazy...
      int0f();
      break;
    default:
      /* 0x04 DSP Status */
      /* 0xF1 DSP Auxiliary Status */
      sb_read_buf[0] = sb_read_buf[1] = 0;
      break;
    }
  }
  return ax;
}

unsigned emulate_sbdsp_write_buffer(unsigned ax) {
#ifdef DEBUG
  writechar('W');writechar('R');writechar('?');writechar(' ');
#endif
  return ax & ~0xFF;
}

unsigned emulate_sbdsp_data_avail(unsigned ax) {
#ifdef DEBUG
  writechar('R');writechar('D');writechar('?');writechar(' ');
#endif
  return ax | 0xFF;
}

unsigned emulate_sbmixer_data_write(unsigned ax) {
#ifdef DEBUG
  writechar('M');writechar('I');writechar('X');write2hex(ax);writeln();
#endif
  // Just reuse the DSP read buffer here, we don't care
  // Miles just wants to write & read back
  sb_read_buf[0] = ax;
  sb_read_buf[1] = ax;
  return ax;
}
