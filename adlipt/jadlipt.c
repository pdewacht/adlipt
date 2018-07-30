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


__declspec(naked) static void address_io_handler() {
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
    push eax
    /* Find client CS:IP, calculate linear address */
    movzx eax, word ptr [ebp + 0x2C]
    shl eax, 4
    add eax, dword ptr [ebp + 0x28]
    /* Put on stack, restore original EAX */
    xchg eax, dword ptr [esp]
    call emulate_opl2_address_io
    ret
  skip:  /* VMMJmp Simulate_IO */
    int 0x20
    dd 0x1001D or 0x8000
  }
}


__declspec(naked) static void data_io_handler() {
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
    push eax
    /* Find client CS:IP, calculate linear address */
    movzx eax, word ptr [ebp + 0x2C]
    shl eax, 4
    add eax, dword ptr [ebp + 0x28]
    /* Put on stack, restore original EAX */
    xchg eax, dword ptr [esp]
    call emulate_opl2_data_io
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
  "Usage: JLOAD JADLIPT.DLL [LPT1|LPT2|LPT3]\r\n";

static const char not_present[] =
  "Port not present\r\n";

static int install(char *cmd_line) {
  enum mode mode;

  puts(banner);

  /* Defaults */
  config.bios_id = 0;
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
