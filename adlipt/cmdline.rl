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
action opt_blaster_def { config.sb_base = 0x220; }
action opt_blaster     { config.sb_base = 0x200 + 16 * (p[-1] - '0'); }
action opt_nopatch     { config.enable_patching = 0; }
action opt_forcedelay  { config.cpu_type = 100; }

# accept C NUL-terminated strings and DOS CR-terminated strings
end = (0 | 13) @{ fbreak; };
sep = " "+;

load_opt =
  ( /OPL[23]/i         @opt_opl3 . /LPT/i?
  | /LPT[123]/i        @opt_lpt
  | /BLASTER/i         @opt_blaster_def
  | /BLASTER=2[0-8]0/i @opt_blaster
  | /NOPATCH/i         @opt_nopatch
  | /FORCEDELAY/i      @opt_forcedelay
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


#define MAX_PORTS 10

int *collect_ports(struct config _WCI86FAR *cfg) {
  static int buf[MAX_PORTS + 1];
  int *p = buf;
  int i;
  *p++ = 0x388;
  *p++ = 0x389;
  if (cfg->opl3) {
    *p++ = 0x38A;
    *p++ = 0x38B;
  }
  if (cfg->sb_base) {
    *p++ = cfg->sb_base + 8;
    *p++ = cfg->sb_base + 9;
    if (cfg->opl3) {
      for (i = 0; i < 4; i++) {
        *p++ = cfg->sb_base + i;
      }
    }
  }
  *p++ = 0;
  return buf;
}
