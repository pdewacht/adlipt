union version {
  short word;
  struct {
    unsigned char minor;
    unsigned char major;
  };
};

_Packed struct config {
  char enabled;
  int lpt;
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

