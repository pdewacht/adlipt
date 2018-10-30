#include <stddef.h>
#include "jlm.h"
#include "resident.h"
#include "cmdline.h"
#include "cputype.h"

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


__declspec(naked) static void port_trap() {
  /*
   * eax: data
   * ebx: VM handle
   * ecx: IO type
   * edx: port
   * ebp: Client_Reg_Struct
   */
  __asm {
    test ecx, not 4
    jnz skip

    push ebx
    /* Find client CS:IP, calculate linear address */
    movzx ebx, word ptr [ebp + 0x2C]
    shl ebx, 4
    add ebx, dword ptr [ebp + 0x28]
    mov dword ptr [port_trap_ip], ebx
    /* Call emulation routine */
    call get_port_handler
    call ebx
    pop ebx
    ret

  skip:  /* VMMJmp Simulate_IO */
    int 0x20
    dd 0x1001D or 0x8000
  }
}


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
  "Usage: JLOAD JADLIPT.DLL [LPT1|LPT2|LPT3] [OPL2] [BLASTER[=220]]\r\n";

static const char not_present[] =
  "Port not present\r\n";


static int install(char *cmd_line) {
  enum mode mode;
  int i, *ports;

  puts(banner);

  /* Defaults */
  config.bios_id = 0;
  config.opl3 = 1;
  config.sb_base = 0;
  config.enable_patching = 1;
  config.cpu_type = cpu_type();

  mode = parse_command_line(cmd_line);

  if (mode != MODE_LOAD) {
    puts(usage);
    return 0;
  }

  config.lpt_port = get_lpt_port(config.bios_id + 1);
  if (!config.lpt_port) {
    puts(not_present);
    return 0;
  }

  /* hw_reset(config.lpt_port); */

  ports = collect_ports(&config);
  for (i = 0; ports[i]; i++) {
    if (Install_IO_Handler(ports[i], port_trap) != 0) {
      goto fail;
    }
  }
  return 1;

 fail:
  for (i--; i >= 0; i--) {
    Remove_IO_Handler(ports[i]);
  }
  return 0;
}

static int uninstall() {
  int i, *ports;
  ports = collect_ports(&config);
  for (i = 0; ports[i]; i++) {
    Remove_IO_Handler(ports[i]);
  }
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
