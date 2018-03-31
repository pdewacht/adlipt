#include <conio.h>
#include <dos.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cputype.h"
#include "resident.h"


#define STR(x) #x
#define XSTR(x) STR(x)


int amis_install_check(char amis_id, struct amis_info *info);
#pragma aux amis_install_check =                \
  "xchg al, ah"                                 \
  "int 0x2D"                                    \
  "cbw"                                         \
  "mov word ptr [si], di"                       \
  "mov word ptr 2[si], dx"                      \
  "mov word ptr 4[si], cx"                      \
  parm [ax] [si]                                \
  value [ax]                                    \
  modify [bx cx dx di]

struct amis_info {
  char __far *signature;
  union version version;
};


int ioctl_read(int handle, char __near *buf, int nbytes);
#pragma aux ioctl_read =                        \
  "mov ax, 0x4402"                              \
  "int 0x21"                                    \
  "sbb dx, dx"                                  \
  parm [bx] [dx] [cx]                           \
  value [dx]                                    \
  modify [ax bx cx dx si di]

static int emm386_get_version(int handle, char __near *version_buf) {
  /* Interrupt list: 214402SF02 GET MEMORY MANAGER VERSION */
  version_buf[0] = 2;
  return ioctl_read(handle, version_buf, 2);
}

int emm386_virtualize_io(int first, int last, int count, void __far *table, int size, int *out_handle);
/* Interrupt list: 2F4A15BX0000 INSTALL I/O VIRTUALIZATION HANDLER */
#pragma aux emm386_virtualize_io =              \
  ".386"                                        \
  "push ax"                                     \
  "push ds"                                     \
  "mov ax, es"                                  \
  "mov ds, ax"                                  \
  "mov ax, 0x4A15"                              \
  "shl edx, 16"                                 \
  "mov dx, bx"                                  \
  "xor bx, bx"                                  \
  "stc"                                         \
  "int 0x2F"                                    \
  "pop ds"                                      \
  "pop bx"                                      \
  "mov [bx], ax"                                \
  "sbb ax, ax"                                  \
  parm [bx] [dx] [cx] [es si] [di] [ax]         \
  value [ax]                                    \
  modify [ax bx cx dx si di]


int emm386_unvirtualize_io(int handle);
#pragma aux emm386_unvirtualize_io =            \
  "mov ax, 0x4A15"                              \
  "mov bx, 1"                                   \
  "stc"                                         \
  "int 0x2F"                                    \
  "sbb ax, ax"                                  \
  parm [si]                                     \
  value [ax]                                    \
  modify [ax bx cx dx si di]


int qemm_get_qpi_entry_point(int handle, void __far **qpi_entry);
#pragma aux qemm_get_qpi_entry_point =          \
  "mov ax, 0x4402"                              \
  "mov cx, 4"                                   \
  "int 0x21"                                    \
  "sbb ax, ax"                                  \
  parm [bx] [dx]                                \
  modify [bx cx dx si di]


int qpi_get_version(void __far **qpi_entry);
#pragma aux qpi_get_version =                   \
  "mov ah, 3"                                   \
  "call dword ptr [si]"                         \
  parm [si]                                     \
  value [bx]                                    \
  modify [ax]


void __far *qpi_get_io_callback(void __far **qpi_entry);
#pragma aux qpi_get_io_callback =               \
  "mov ax, 0x1A06"                              \
  "call dword ptr [si]"                         \
  parm [si]                                     \
  value [es di]                                 \
  modify [ax]


void qpi_set_io_callback(void __far **qpi_entry, void __far *callback);
#pragma aux qpi_set_io_callback =               \
  "mov ax, 0x1A07"                              \
  "call dword ptr [si]"                         \
  parm [si] [es di]                             \
  modify [ax   dx]


char qpi_get_port_trap(void __far **qpi_entry, int port);
#pragma aux qpi_get_port_trap =                 \
  "mov ax, 0x1A08"                              \
  "call dword ptr [si]"                         \
  parm [si] [dx]                                \
  value [bl]                                    \
  modify [ax]


void qpi_set_port_trap(void __far **qpi_entry, int port);
#pragma aux qpi_set_port_trap =                 \
  "mov ax, 0x1A09"                              \
  "call dword ptr [si]"                         \
  parm [si] [dx]                                \
  modify [ax]


void qpi_clear_port_trap(void __far **qpi_entry, int port);
#pragma aux qpi_clear_port_trap =               \
  "mov ax, 0x1A0A"                              \
  "call dword ptr [si]"                         \
  parm [si] [dx]                                \
  modify [ax]


void qpi_clear_port_trap(void __far **qpi_entry, int port);
#pragma aux qpi_clear_port_trap =               \
  "mov ax, 0x1A0A"                              \
  "call dword ptr [si]"                         \
  parm [si] [dx]                                \
  modify [ax]


static bool amis_unhook(struct iisp_header __far *handler, unsigned our_seg) {
  for (;;) {
    struct iisp_header __far *next_handler;
    if (handler->jump_to_start != 0x10EB
        || handler->signature != 0x424B
        || handler->jump_to_reset[0] != 0xEB) {
      return false;
    }
    next_handler = handler->next_handler;
    if (FP_SEG(next_handler) == our_seg) {
      handler->next_handler = next_handler->next_handler;
      return true;
    }
    handler = next_handler;
  }
}


static bool setup_emm386() {
  unsigned char version[2];
  int handle, err, v;

  err = _dos_open("EMMXXXX0", O_RDONLY, &handle);
  if (err) {
    err = _dos_open("EMMQXXX0", O_RDONLY, &handle);
  }
  if (err) {
    return false;
  }
  err = emm386_get_version(handle, &version);
  _dos_close(handle);
  if (err || !(version[0] == 4 && version[1] >= 46)) {
    return false;
  }

  err = emm386_virtualize_io(0x388, 0x389, 2, &emm386_table, (int)&resident_end, &v);
  if (err) {
    cputs("EMM386 I/O virtualization failed\r\n");
    exit(1);
  }
  config.emm_type = EMM_EMM386;
  config.emm386_virt_io_handle = v;
  return true;
}


static bool shutdown_emm386(struct config __far *cfg) {
  int err;
  err = emm386_unvirtualize_io(cfg->emm386_virt_io_handle);
  if (err) {
    return false;
  }
  cfg->emm_type = EMM_NONE;
  return true;
}


static void __far *get_qpi_entry_point() {
  int handle, err;
  void __far *qpi;
  err = _dos_open("QEMM386$", O_RDONLY, &handle);
  if (err) {
    return 0;
  }
  err = qemm_get_qpi_entry_point(handle, &qpi);
  _dos_close(handle);
  if (err) {
    return 0;
  }
  return qpi;
}


static bool setup_qemm() {
  void __far *qpi;
  int version;

  qpi = get_qpi_entry_point();
  if (!qpi) {
    return false;
  }
  version = qpi_get_version(&qpi);
  if (version < 0x0703) {
    return false;
  }

  if (qpi_get_port_trap(&qpi, 0x388) || qpi_get_port_trap(&qpi, 0x389)) {
    cputs("Some other program is already intercepting Adlib I/O\r\n");
    exit(1);
  }
  qpi_set_port_trap(&qpi, 0x388);
  qpi_set_port_trap(&qpi, 0x389);

  qemm_handler.next_handler = qpi_get_io_callback(&qpi);
  qpi_set_io_callback(&qpi, &qemm_handler);

  config.emm_type = EMM_QEMM;
  return true;
}


static bool shutdown_qemm(struct config __far *cfg) {
  void __far *qpi;
  struct iisp_header __far *callback;

  qpi = get_qpi_entry_point();
  if (!qpi) {
    return false;
  }

  qpi_clear_port_trap(&qpi, 0x388);
  qpi_clear_port_trap(&qpi, 0x389);

  callback = qpi_get_io_callback(&qpi);
  if (FP_SEG(callback) == FP_SEG(cfg)) {
    qpi_set_io_callback(&qpi, callback->next_handler);
  } else {
    if (!amis_unhook(callback, FP_SEG(cfg))) {
      return false;
    }
  }
  cfg->emm_type = EMM_NONE;
  return true;
}


static void check_jemm(char bios_id) {
  unsigned char buf[6] = { 0 };
  int handle, err;

  err = _dos_open("EMMXXXX0", O_RDONLY, &handle);
  if (err) {
    err = _dos_open("EMMQXXX0", O_RDONLY, &handle);
  }
  if (err) {
    return;
  }
  err = ioctl_read(handle, &buf, 6);
  _dos_close(handle);
  if (err || !(buf[0] == 0x28 && buf[1] == 0)) {
    return;
  }

  cputs("Detected JEMM memory manager. Use this command instead:\r\n"
        "    JLOAD JADLIPT.DLL LPT");
  putch('1' + bios_id);
  cputs("\r\n");
  exit(1);
}


static bool uninstall(struct config __far *cfg) {
  struct iisp_header __far *current_amis_handler;

  if (cfg->emm_type == EMM_EMM386 && !shutdown_emm386(cfg)) {
    return false;
  }
  else if (cfg->emm_type == EMM_QEMM && !shutdown_qemm(cfg)) {
    return false;
  }

  current_amis_handler = (struct iisp_header __far *) _dos_getvect(0x2D);
  if (FP_SEG(current_amis_handler) == FP_SEG(cfg)) {
    _dos_setvect(0x2D, current_amis_handler->next_handler);
  } else {
    if (!amis_unhook(current_amis_handler, FP_SEG(cfg))) {
      return false;
    }
  }

  _dos_freemem(cfg->psp);
  return true;
}


static short get_lpt_port(int i) {
  return *(short __far *)MK_FP(0x40, 6 + 2*i);
}


static void usage(void) {
  cputs("Usage: ADLIPT LPT1|LPT2|LPT3\r\n"
        "       ADLIPT UNLOAD\r\n");
  exit(1);
}


static void status(struct config __far *cfg) {
  cputs("  Status: loaded\r\n");
  cputs("  Port: LPT");
  putch('1' + cfg->bios_id);
  cputs("\r\n");

  cputs("  Detected CPU: ");
  cputs(cfg->cpu_type == 3 ? "386" :
        cfg->cpu_type == 4 ? "486" :
        "Pentium or later");
  cputs("\r\n");

  cputs("  Patching: ");
  cputs(cfg->emm_type == EMM_QEMM ? "not supported with QEMM" :
        cfg->enable_patching ? "enabled" :
        "disabled");
  cputs("\r\n");
}


int main(void) {
  bool installed = false;
  bool found_unused_amis_id = false;
  int unused_amis_id = -1;
  struct config __far *cfg = &config;
  int i;

  cputs("ADLiPT " XSTR(VERSION_MAJOR) "." XSTR(VERSION_MINOR)
        "  github.com/pdewacht/adlipt\r\n\r\n");

  config.cpu_type = cpu_type();
  config.enable_patching = true /* (config.cpu_type == 3) */;

  if (config.cpu_type < 3) {
    cputs("This TSR requires a 386 or later CPU.\r\n");
    return 1;
  }

  /* Check if the TSR is already in memory */
  /* Also look for an unused AMIS multiplex id */
  for (i = 0; i < 256; ++i) {
    struct amis_info info;
    int result = amis_install_check(i, &info);
    if (result == 0 && !found_unused_amis_id) {
      found_unused_amis_id = true;
      amis_id = i;
    }
    else if (result == -1 && _fmemcmp(info.signature, amis_header, 16) == 0) {
      if (info.version.word != (VERSION_MAJOR * 256 + VERSION_MINOR)) {
        cputs("Error: A different version of ADLIPT is already loaded.\r\n");
        exit(1);
      }
      installed = true;
      cfg = MK_FP(FP_SEG(info.signature),
                  *(short __far *)(info.signature + _fstrlen(info.signature) + 1));
      break;
    }
  }

  /* Parse the command line */
  {
    char cmdline[127];
    int cmdlen;
    char *arg;

    cmdlen = *(char __far *)MK_FP(_psp, 0x80);
    _fmemcpy(cmdline, MK_FP(_psp, 0x81), 127);
    cmdline[cmdlen] = 0;

    for (arg = strtok(cmdline, " "); arg; arg = strtok(NULL, " ")) {
      if (strnicmp(arg, "lpt", 3) == 0 && (i = arg[3] - '0') >= 1 && i <= 3) {
        int port = get_lpt_port(i);
        if (!port) {
          cputs("Error: LPT");
          putch('0' + i);
          cputs(" is not present.\r\n");
          exit(1);
        }
        cfg->lpt_port = port;
        cfg->bios_id = i - 1;
      }
      else if (stricmp(arg, "patch") == 0) {
        cfg->enable_patching = true;
      }
      else if (stricmp(arg, "nopatch") == 0) {
        cfg->enable_patching = false;
      }
      else if (stricmp(arg, "forcedelay") == 0) {
        cfg->cpu_type = 100;
      }
      else if (stricmp(arg, "noforcedelay") == 0) {
        cfg->cpu_type = config.cpu_type;
      }
      else if (stricmp(arg, "unload") == 0) {
        if (!installed) {
          cputs("ADLiPT is not loaded.\r\n");
          exit(1);
        } else if (uninstall(cfg)) {
          cputs("ADLiPT is now unloaded from memory.\r\n");
          exit(0);
        } else {
          cputs("Could not unload ADLiPT.\r\n");
          exit(1);
        }
      } else {
        usage();
      }
    }
  }

  if (!installed) {
    if (!cfg->lpt_port) {
      usage();
      return 1;
    }
    cfg->psp = _psp;

    if (!found_unused_amis_id) {
      cputs("Error: No unused AMIS multiplex id found\n");
      return 1;
    }

    check_jemm(cfg->bios_id);
    if (!setup_qemm() && !setup_emm386()) {
      cputs("Error: no supported memory manager found\r\n"
            "Requires EMM386 4.46+, QEMM 7.03+ or JEMM\r\n");
      return 1;
    }

    /* hook AMIS interrupt */
    amis_handler.next_handler = _dos_getvect(0x2D);
    _dos_setvect(0x2D, (void (__interrupt *)()) &amis_handler);
  }

  if (cfg->emm_type != EMM_EMM386) {
    cfg->enable_patching = false;
  }

  status(cfg);

  if (!installed) {
    /* free environment block */
    int __far *env_seg = MK_FP(_psp, 0x2C);
    _dos_freemem(*env_seg);
    *env_seg = 0;

    _dos_keep(0, ((char __huge *)&resident_end - (char __huge *)(_psp :> 0) + 15) / 16);
  }
  return 0;
}
