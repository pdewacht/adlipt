#ifdef _M_I86

char cpu_type(void);
#pragma aux cpu_type =                                                  \
  ".586"                                                                \
  "cli"                                                                 \
                                                                        \
  "pushf"                   /* push original FLAGS */                   \
  "pop     ax"              /* get original FLAGS */                    \
  "mov     cx, ax"          /* save original FLAGS */                   \
  "and     ax, 0fffh"       /* clear bits 12-15 in FLAGS */             \
  "push    ax"              /* save new FLAGS value on stack */         \
  "popf"                    /* replace current FLAGS value */           \
  "pushf"                   /* get new FLAGS */                         \
  "pop     ax"              /* store new FLAGS in AX */                 \
  "and     ax, 0f000h"      /* if bits 12-15 are set, then */           \
  "cmp     ax, 0f000h"      /*   processor is an 8086/8088 */           \
  "mov     al, 0"           /* turn on 8086/8088 flag */                \
  "je      end_cpu_type"    /* jump if processor is 8086/8088 */        \
                                                                        \
  "or      cx, 0f000h"      /* try to set bits 12-15 */                 \
  "push    cx"              /* save new FLAGS value on stack */         \
  "popf"                    /* replace current FLAGS value */           \
  "pushf"                   /* get new FLAGS */                         \
  "pop     ax"              /* store new FLAGS in AX */                 \
  "and     ax, 0f000h"      /* if bits 12-15 are clear */               \
  "mov     al, 2"           /* processor=80286, turn on 80286 flag */   \
  "jz      end_cpu_type"    /* if no bits set, processor is 80286 */    \
                                                                        \
  "pushfd"                  /* push original EFLAGS */                  \
  "pop     eax"             /* get original EFLAGS */                   \
  "mov     ecx, eax"        /* save original EFLAGS */                  \
  "xor     eax, 40000h"     /* flip AC bit in EFLAGS */                 \
  "push    eax"             /* save new EFLAGS value on stack */        \
  "popfd"                   /* replace current EFLAGS value */          \
  "pushfd"                  /* get new EFLAGS */                        \
  "pop     eax"             /* store new EFLAGS in EAX */               \
  "xor     eax, ecx"        /* can't toggle AC bit, processor=80386 */  \
  "mov     al, 3"           /* turn on 80386 processor flag */          \
  "jz      end_cpu_type"    /* jump if 80386 processor */               \
  "push    ecx"                                                         \
  "popfd"                   /* restore AC bit in EFLAGS first */        \
                                                                        \
  "mov     eax, ecx"        /* get original EFLAGS */                   \
  "xor     eax, 200000h"    /* flip ID bit in EFLAGS */                 \
  "push    eax"             /* save new EFLAGS value on stack */        \
  "popfd"                   /* replace current EFLAGS value */          \
  "pushfd"                  /* get new EFLAGS */                        \
  "pop     eax"             /* store new EFLAGS in EAX */               \
  "xor     eax, ecx"        /* can't toggle ID bit, */                  \
  "mov     al, 4"           /* turn on 80486 processor flag */          \
  "je      end_cpu_type"    /* processor=80486 */                       \
                                                                        \
  "xor     eax, eax"        /* set up for CPUID instruction */          \
  "cpuid"                   /* get and save vendor ID */                \
  "cmp     eax, 1"          /* make sure 1 is valid input for CPUID */  \
  "mov     al, 4"           /* turn on 80486 processor flag */          \
  "jb      end_cpu_type"    /* if not, jump to end */                   \
  "mov     eax, 1"                                                      \
  "cpuid"                   /* get family/model/stepping/features */    \
  "shr     ax, 8"           /* isolate family */                        \
  "and     ax, 0fh"                                                     \
                                                                        \
  "end_cpu_type:"                                                       \
  "sti"                                                                 \
  value [al] modify [ax bx cx dx]

#else

char cpu_type(void);
#pragma aux cpu_type =                                                  \
  ".586"                                                                \
  "cli"                                                                 \
                                                                        \
  "pushfd"                  /* push original EFLAGS */                  \
  "pop     eax"             /* get original EFLAGS */                   \
  "mov     ecx, eax"        /* save original EFLAGS */                  \
  "xor     eax, 40000h"     /* flip AC bit in EFLAGS */                 \
  "push    eax"             /* save new EFLAGS value on stack */        \
  "popfd"                   /* replace current EFLAGS value */          \
  "pushfd"                  /* get new EFLAGS */                        \
  "pop     eax"             /* store new EFLAGS in EAX */               \
  "xor     eax, ecx"        /* can't toggle AC bit, processor=80386 */  \
  "mov     al, 3"           /* turn on 80386 processor flag */          \
  "jz      end_cpu_type"    /* jump if 80386 processor */               \
  "push    ecx"                                                         \
  "popfd"                   /* restore AC bit in EFLAGS first */        \
                                                                        \
  "mov     eax, ecx"        /* get original EFLAGS */                   \
  "xor     eax, 200000h"    /* flip ID bit in EFLAGS */                 \
  "push    eax"             /* save new EFLAGS value on stack */        \
  "popfd"                   /* replace current EFLAGS value */          \
  "pushfd"                  /* get new EFLAGS */                        \
  "pop     eax"             /* store new EFLAGS in EAX */               \
  "xor     eax, ecx"        /* can't toggle ID bit, */                  \
  "mov     al, 4"           /* turn on 80486 processor flag */          \
  "je      end_cpu_type"    /* processor=80486 */                       \
                                                                        \
  "xor     eax, eax"        /* set up for CPUID instruction */          \
  "cpuid"                   /* get and save vendor ID */                \
  "cmp     eax, 1"          /* make sure 1 is valid input for CPUID */  \
  "mov     al, 4"           /* turn on 80486 processor flag */          \
  "jb      end_cpu_type"    /* if not, jump to end */                   \
  "mov     eax, 1"                                                      \
  "cpuid"                   /* get family/model/stepping/features */    \
  "shr     eax, 8"          /* isolate family */                        \
  "and     eax, 0fh"                                                    \
                                                                        \
  "end_cpu_type:"                                                       \
  "sti"                                                                 \
  value [al] modify [eax ebx ecx edx]

#endif
