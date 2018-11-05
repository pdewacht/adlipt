#include <stddef.h>
#include "resident.h"
#include "cmdline.h"

%%{

machine cmdline;

action mode_load       { mode = MODE_LOAD; }
action mode_unload     { mode = MODE_UNLOAD; }
action mode_status     { mode = MODE_STATUS; }
action opt_opl3        { config.opl3 = *p == '3'; }
action opt_lpt         { config.bios_id = *p - '1'; }
action opt_nopatch     { config.enable_patching = 0; }
action opt_forcedelay  { config.cpu_type = 100; }
action opt_sb          { config.sb_base = 0x220; }
action opt_sb_port     { config.sb_base = 0x200 + 16 * (p[-1] - '0'); }
action opt_sb_fake     { config.sb_fake = 1; }

# accept C NUL-terminated strings and DOS CR-terminated strings
end = (0 | 13) @{ fbreak; };
sep = " "+;

load_opt =
  ( /OPL[23]/i         @opt_opl3 . /LPT/i?
  | /LPT[123]/i        @opt_lpt
  | /NOPATCH/i         @opt_nopatch
  | /FORCEDELAY/i      @opt_forcedelay
  | (/FAKE/i           @opt_sb_fake)? .
     /BLASTER/i        @opt_sb
    (/=2[0-8]0/        @opt_sb_port)?
  );

load   = (load_opt . (sep . load_opt)*)?  %mode_load;
unload = /UNLOAD/i                        @mode_unload;
status = /STATUS/i                        @mode_status;

main := sep? . (load | unload | status) . sep? . end;

}%%


enum mode parse_command_line (char _WCI86FAR *p)
{
  enum mode mode;
  int cs;
  %%write data;
  %%write init;
  %%write exec noend;
  if (cs < cmdline_first_final) {
    mode = MODE_USAGE;
  }
  return mode;
}


#define MAX_PORTS 16

int *collect_ports(struct config _WCI86FAR *cfg) {
  static int buf[MAX_PORTS + 1];
  int *p = buf;
  int sb_base, i;
  *p++ = 0x388;
  *p++ = 0x389;
  if (cfg->opl3) {
    *p++ = 0x38A;
    *p++ = 0x38B;
  }
  if ((sb_base = cfg->sb_base)) {
    *p++ = sb_base + 0x8;
    *p++ = sb_base + 0x9;
    if (cfg->sb_fake) {
      *p++ = sb_base + 0x4;
      *p++ = sb_base + 0x5;
      *p++ = sb_base + 0x6;
      *p++ = sb_base + 0xA;
      *p++ = sb_base + 0xC;
      *p++ = sb_base + 0xE;
    }
    if (cfg->opl3) {
      for (i = 0; i < 4; i++) {
        *p++ = sb_base + i;
      }
    }
  }
  *p++ = 0;
  return buf;
}
