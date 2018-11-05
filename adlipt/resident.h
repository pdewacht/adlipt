#ifdef _M_I86

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

#endif

_Packed struct config {
  unsigned lpt_port;
  char bios_id;
  char opl3;
  unsigned sb_base;
  char sb_fake;
  char cpu_type;
  char enable_patching;
#ifdef _M_I86
  unsigned psp;
  enum emm_type emm_type;
  int emm386_virt_io_handle;
#endif
};


#define RESIDENT __based(__segname("RESIDENT"))

extern struct config RESIDENT config;

extern struct iisp_header RESIDENT amis_handler;
extern char RESIDENT amis_header[];
extern char RESIDENT amis_id;

extern int RESIDENT emm386_table[];
extern struct iisp_header RESIDENT qemm_handler;

extern char _WCI86FAR * RESIDENT port_trap_ip;

extern char RESIDENT resident_end[];


typedef unsigned porthandler(unsigned ax);
#ifdef _M_I86
#pragma aux porthandler parm [ax] value [ax] modify exact [ax bx]
#else
#pragma aux porthandler parm [eax] value [eax] modify exact [eax ebx]
#endif

extern porthandler emulate_opl2_write_address;
extern porthandler emulate_opl2_write_data;
extern porthandler emulate_opl3_write_low_address;
extern porthandler emulate_opl3_write_high_address;
extern porthandler emulate_opl3_write_data;
extern porthandler emulate_opl2_read;
extern porthandler emulate_opl3_read;
extern porthandler emulate_ignore;
extern porthandler emulate_sbdsp_reset;
extern porthandler emulate_sbdsp_read_data;
extern porthandler emulate_sbdsp_write_data;
extern porthandler emulate_sbdsp_write_buffer;
extern porthandler emulate_sbdsp_data_avail;
extern porthandler emulate_sbmixer_data_write;

porthandler *get_port_handler(unsigned port, unsigned flags);
#ifdef _M_I86
#pragma aux get_port_handler parm [dx cx] value [bx] modify exact [bx]
#else
#pragma aux get_port_handler parm [edx ecx] value [ebx] modify exact [ebx]
#endif


extern void hw_reset(unsigned port);
