union version {
  short word;
  struct {
    unsigned char minor;
    unsigned char major;
  };
};

_Packed struct config {
  char enabled;
  short lpt_port;
  short bios_id;
};

#define RESIDENT __based(__segname("RESIDENT"))

extern struct config RESIDENT config;

extern char RESIDENT amis_header[];
extern char RESIDENT amis_id;
extern void __far * RESIDENT amis_chain;
extern void RESIDENT amis_hook();

extern char RESIDENT emm386_glue[];
extern void RESIDENT qemm_glue();
extern void __far * RESIDENT qemm_chain;

extern char RESIDENT resident_end[];


unsigned emulate_adlib_io(int port, int is_write, unsigned ax);
#ifdef _M_I86
#pragma aux emulate_adlib_io parm [dx] [cx] [ax] value [ax]
#else
#pragma aux emulate_adlib_io parm [edx] [ecx] [eax] value [eax]
#endif
