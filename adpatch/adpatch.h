#define MAX_MATCHES 8
#define MAX_RELOCATIONS 2
#define MAX_EXPORTS 2

enum relocation_type {
  R_386_32,
  R_386_PC32,
  R_386_16,
  R_386_PC16
};

struct relocation {
  long offset;
  int symbol;
  enum relocation_type type;
};

struct export {
  int symbol;
  int offset;
};

struct patch {
  const char *name;
  const char *code;
  int code_len;
  struct relocation rel[MAX_RELOCATIONS];
  struct export export[MAX_EXPORTS];
};

struct match {
  const struct patch *patch;
  long offset;
  int len;
};


int apply_patches(FILE *in, FILE *out, int lpt_port);
int scan_and_copy(FILE *in, FILE *out, struct match *matches);

extern const struct patch patch_data[];
