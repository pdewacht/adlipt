#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __WATCOMC__
  #include <io.h>
#endif
#include "adpatch.h"


#define STR(x) #x
#define XSTR(x) STR(x)


#ifdef __WATCOMC__
  #define DEFAULT_PORT "LPT1"
#else
  #define DEFAULT_PORT "0x378"
#endif


static int patch_port;


static void in_place_mode(const char *orig_filename) {
  static const char bak_ext[] = ".bak";
  FILE *in = 0, *out = 0;
  char *bak_filename;
  int i, ext;

  if (access(orig_filename, R_OK) != 0) {
    fprintf(stderr, "Error: Can't read file: %s\n", orig_filename);
    exit(1);
  }

  bak_filename = malloc(strlen(orig_filename) + strlen(bak_ext) + 1);
  if (!bak_filename) { exit(1); }
  strcpy(bak_filename, orig_filename);
  for (ext = i = strlen(bak_filename); i >= 0; i--) {
    if (bak_filename[i] == '/' || bak_filename[i] == '\\') {
      break;
    } else if (bak_filename[i] == '.') {
      ext = i;
    }
  }
  strcpy(bak_filename + ext, bak_ext);
  printf("ADPATCH " XSTR(VERSION) "\n");
  printf("Patching %s\n", orig_filename);
  printf("Writing backup copy to %s\n", bak_filename);

  if (access(bak_filename, F_OK) == 0) {
    fprintf(stderr, "Error: Backup file already exists, not overwriting.\n");
    exit(1);
  }

  if (rename(orig_filename, bak_filename) != 0) {
    perror("Error: Rename failed");
    exit(1);
  }

  in = fopen(bak_filename, "rb");
  if (!in) {
    perror("Error: Can't open input file for reading");
    goto rollback;
  }
  setvbuf(in, 0, _IONBF, 0);

  out = fopen(orig_filename, "wb");
  if (!out) {
    perror("Error: Can't open output file for writing");
    goto rollback;
  }
  setvbuf(out, 0, _IONBF, 0);

  i = apply_patches(in, out, patch_port);
  if (i <= 0) {
    goto rollback;
  }

  fclose(out);
  fclose(in);
  return;

 rollback:
  if (out) {
    fclose(out);
  }
  if (in) {
    fclose(in);
  }
  remove(orig_filename);
  rename(bak_filename, orig_filename);
  exit(1);
}

static void copy_mode(const char *src_filename, const char *dst_filename) {
  FILE *in, *out;
  int r;

  in = fopen(src_filename, "rb");
  if (!in) {
    perror("Error: Can't open input file for reading");
    exit(1);
  }
  setvbuf(in, 0, _IONBF, 0);

  out = fopen(dst_filename, "wb");
  if (!out) {
    perror("Error: Can't open output file for writing");
    exit(1);
  }
  setvbuf(out, 0, _IONBF, 0);

  r = apply_patches(in, out, patch_port);

  fclose(out);
  fclose(in);

  if (r < 0) {
    unlink(dst_filename);
    exit(1);
  }
}

static void usage() {
  fprintf(stderr,
          "ADPATCH " XSTR(VERSION) "\n"
          "\n"
          "Usage: adpatch [OPTIONS...] FILE\n"
          "       adpatch [OPTIONS...] -o OUTPUT-FILE INPUT-FILE\n"
          "\n"
          "Options:\n"
          "  -p PORT       Set OPL2LPT port (default: " DEFAULT_PORT ")\n"
          );
  exit(1);
}

static short get_lpt_port(int i) {
  #ifdef __WATCOMC__
    return *(short __far *)(0x40 :> (6 + 2*i));
  #else
    return 0;
  #endif
}

static short parse_port(const char *arg) {
  int i, port;
  if (strncasecmp(arg, "lpt", 3) == 0 && (i = arg[3] - '0') >= 1 && i <= 3) {
    port = get_lpt_port(i);
  } else {
    port = strtol(arg, 0, 16);
  }
  if (port == 0) {
    fprintf(stderr, "Error: invalid port %s\n", arg);
    exit(1);
  }
  return port;
}

int main(int argc, char *argv[]) {
  char *src = NULL;
  char *dest = NULL;
  int opt;

  while ((opt = getopt(argc, argv, "ip:o:")) != -1) {
    switch (opt) {
    case 'i':
      /* Ignored for backward compatibility*/
      break;
    case 'p':
      patch_port = parse_port(optarg);
      break;
    case 'o':
      dest = optarg;
      break;
    default:
      usage();
    }
  }

  if (!patch_port) {
    patch_port = parse_port(DEFAULT_PORT);
  }

  if (optind + 1 != argc) {
    usage();
  }
  src = argv[optind];

  if (!dest) {
    in_place_mode(src);
  } else {
    copy_mode(src, dest);
  }
  return 0;
}
