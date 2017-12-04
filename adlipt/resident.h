union version {
  unsigned short word;
  struct {
    unsigned char minor;
    unsigned char major;
  };
};

_Packed struct iisp_header {
  unsigned short jump_to_start;
  void __far *next_handler;
  unsigned short signature;
  char eoi_flag;
  char jump_to_reset[2];
  char reserved[7];
};

enum emm_type {
  EMM_NONE = 0,
  EMM_EMM386,
  EMM_QEMM
};

_Packed struct config {
  char enabled;
  int lpt_port;
  int bios_id;
  int psp;
  enum emm_type emm_type;
  int emm386_virt_io_handle;
};


#define RESIDENT __based(__segname("RESIDENT"))

extern struct config RESIDENT config;

extern struct iisp_header RESIDENT amis_handler;
extern char RESIDENT amis_header[];
extern char RESIDENT amis_id;

extern char RESIDENT emm386_table[];
extern struct iisp_header RESIDENT qemm_handler;

extern char RESIDENT resident_end[];


unsigned emulate_adlib_io(int port, int is_write, unsigned ax);
#ifdef _M_I86
#pragma aux emulate_adlib_io parm [dx] [cx] [ax] value [ax]
#else
#pragma aux emulate_adlib_io parm [edx] [ecx] [eax] value [eax]
#endif
