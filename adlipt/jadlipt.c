#include "jlm.h"
#include "resident.h"

#define STR(x) #x
#define XSTR(x) STR(x)

struct vxd_desc_block ddb = {
  0,  /* Next */
  0,  /* Version */
  UNDEFINED_DEVICE_ID,  /* Req_Device_Number */
  VERSION_MAJOR,  /* Dev_Major_Version */
  VERSION_MINOR,  /* Dev_Minor_Version */
  0,  /* Flags */
  "ADLIPT",  /* Name */
  UNDEFINED_INIT_ORDER,  /* Init_Order */
  0,  /* Control_Proc */
  0,  /* V86_API_Proc */
  0,  /* PM_API_Proc */
  0,  /* V86_API_CSIP */
  0,  /* PM_API_CSIP */
  0,  /* Reference_Data */
  0,  /* Service_Table_Ptr */
  0,  /* Service_Table_Size */
  0,  /* Win32_Service_Table */
  0,  /* Prev */
  sizeof(ddb),  /* Size */
};


__declspec(naked) static void address_io_handler() {
  // eax: data
  // ebx: VM handle
  // ecx: IO type
  // edx: port
  // ebp: Client_Reg_Struct
  __asm {
    test ecx, not 4
    jz emulate_adlib_address_io
    // VMMJmp Simulate_IO
    int 0x20
    dd 0x1001D or 0x8000
  }
}


__declspec(naked) static void data_io_handler() {
  // eax: data
  // ebx: VM handle
  // ecx: IO type
  // edx: port
  // ebp: Client_Reg_Struct
  __asm {
    test ecx, not 4
    jz emulate_adlib_data_io
    // VMMJmp Simulate_IO
    int 0x20
    dd 0x1001D or 0x8000
  }
}


int bswap(int);
#pragma aux bswap =                             \
  "rol ax, 8"                                   \
  "rol eax, 16"                                 \
  "rol ax, 8"                                   \
  parm [eax] value [eax]

static void puts(const char *str) {
  struct cb_s *hVM = Get_Cur_VM_Handle();
  struct Client_Reg_Struc *pcl = hVM->CB_Client_Pointer;

  Begin_Nest_Exec(pcl);
  for (; *str; str++) {
    pcl->Client_EAX = 0x0200;
    pcl->Client_EDX = *str;
    Exec_Int(pcl, 0x21);
  }
  End_Nest_Exec(pcl);
}

static short get_lpt_port(int i) {
  return *(short *)(0x406 + 2*i);
}

static const char banner[] =
  "ADLiPT " XSTR(VERSION_MAJOR) "." XSTR(VERSION_MINOR)
  "  github.com/pdewacht/adlipt\r\n";

static const char usage[] =
  "Usage: JLOAD JADLIPT.DLL LPT1|LPT2|LPT3\r\n";

static const char not_present[] =
  "Port not present\r\n";

static int install(char *cmd_line) {
  unsigned param;
  short port;
  int res;

  puts(banner);

  while (*cmd_line == ' ') {
    cmd_line++;
  }
  param = *(unsigned *)cmd_line;
  param = (bswap(param) & ~0x20202000) - 'LPT1';
  if (param > 2) {
    puts(usage);
    return 0;
  }

  port = get_lpt_port(param + 1);
  if (!port) {
    puts(not_present);
    return 0;
  }

  config.lpt_port = port;
  config.bios_id = param;

  if (Install_IO_Handler(0x388, address_io_handler) != 0) {
    goto fail1;
  }
  if (Install_IO_Handler(0x389, data_io_handler) != 0) {
    goto fail2;
  }
  return 1;

 fail2:
  Remove_IO_Handler(0x388);
 fail1:
  return 0;
}

static int uninstall() {
  Remove_IO_Handler(0x388);
  Remove_IO_Handler(0x389);
  return 1;
}

int __stdcall DllMain(int module, int reason, struct jlcomm *jlcomm) {
  if (reason == DLL_PROCESS_ATTACH) {
    return install(jlcomm->cmd_line);
  }
  if (reason == DLL_PROCESS_DETACH) {
    return uninstall();
  }
  return 0;
}
