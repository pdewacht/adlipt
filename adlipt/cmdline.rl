#include <stddef.h>
#include "resident.h"
#include "cmdline.h"

%%{

machine cmdline;

action mode_load      { mode = MODE_LOAD; }
action mode_unload    { mode = MODE_UNLOAD; }
action mode_status    { mode = MODE_STATUS; }
action opt_lpt        { config.bios_id = *p - '1'; }
action opt_nopatch    { config.enable_patching = 0; }
action opt_forcedelay { config.cpu_type = 100; }

# accept C NUL-terminated strings and DOS CR-terminated strings
end = (0 | 13) @{ fbreak; };
sep = " "+;

load_opt =
  ( /LPT[123]/i    @opt_lpt
  | /NOPATCH/i     @opt_nopatch
  | /FORCEDELAY/i  @opt_forcedelay
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
