#include <conio.h>
#include <dos.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

int emm386_virtualize_io(int first, int last, int count, void __far *table, int size);
/* Interrupt list: 2F4A15BX0000 INSTALL I/O VIRTUALIZATION HANDLER */
#pragma aux emm386_virtualize_io =              \
  ".386"                                        \
  "push ds"                                     \
  "mov ax, es"                                  \
  "mov ds, ax"                                  \
  "mov ax, 0x4A15"                              \
  "shl edx, 16"                                 \
  "mov dx, bx"                                  \
  "xor bx, bx"                                  \
  "int 0x2F"                                    \
  "sbb ax, ax"                                  \
  "pop ds"                                      \
  parm [bx] [dx] [cx] [es si] [di]              \
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


static bool setup_emm386() {
  unsigned char version[2];
  int handle, err;

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

  return emm386_virtualize_io(0x388, 0x389, 2, &emm386_glue, (int)&resident_end) == 0;
}


static bool setup_qemm() {
  int handle, err, version;
  void __far *qpi;

  err = _dos_open("QEMM386$", O_RDONLY, &handle);
  if (err) {
    return false;
  }
  err = qemm_get_qpi_entry_point(handle, &qpi);
  _dos_close(handle);
  if (err) {
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
  qemm_chain = qpi_get_io_callback(&qpi);
  qpi_set_io_callback(&qpi, qemm_glue);
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


static short get_lpt_port(int i) {
  return *(short __far *)MK_FP(0x40, 6 + 2*i);
}


static void usage(void) {
  cputs("Usage: ADLIPT LPT1|LPT2|LPT3\r\n"
        "       ADLIPT DISABLE\r\n"
        "       ADLIPT ENABLE\r\n");
  exit(1);
}


static void status(struct config __far *cfg) {
  cputs("  Status: ");
  cputs(cfg->enabled ? "Enabled" : "Disabled");
  cputs("\r\n");

  cputs("  Port: LPT");
  putch('1' + cfg->bios_id);
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
      cfg = (struct config __far *)(info.signature + _fstrlen(info.signature) + 1);
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
      else if (stricmp(arg, "enable") == 0) {
        cfg->enabled = true;
      }
      else if (stricmp(arg, "disable") == 0) {
        cfg->enabled = false;
      }
      else {
        usage();
      }
    }
  }

  if (installed) {
    status(cfg);
  }
  else {
    int __far *env_seg;

    if (!cfg->lpt_port) {
      usage();
      return 1;
    }
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
    status(cfg);

    /* hook AMIS interrupt */
    amis_chain = _dos_getvect(0x2D);
    _dos_setvect(0x2D, (void (__interrupt *)()) amis_hook);

    /* free environment block */
    env_seg = MK_FP(_psp, 0x2C);
    _dos_freemem(*env_seg);
    *env_seg = 0;

    _dos_keep(0, ((char __huge *)&resident_end - (char __huge *)(_psp :> 0) + 15) / 16);
  }
  return 0;
}
