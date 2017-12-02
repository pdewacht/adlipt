#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __WATCOMC__
  #include <io.h>
#endif
#include "patch.h"


#define STR(x) #x
#define XSTR(x) STR(x)


#ifdef __WATCOMC__
  #define DEFAULT_PORT "LPT1"
#else
  #define DEFAULT_PORT "0x378"
#endif


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
    fprintf(stderr, "Error: Rename failed: %s\n", strerror(errno));
    exit(1);
  }

  in = fopen(bak_filename, "rb");
  if (!in) {
    fprintf(stderr, "Error: Can't open input file for reading: %s\n", strerror(errno));
    goto rollback;
  }
  setvbuf(in, 0, _IONBF, 0);

  out = fopen(orig_filename, "wb");
  if (!out) {
    fprintf(stderr, "Error: Can't open output file for writing: %s\n", strerror(errno));
    goto rollback;
  }
  setvbuf(out, 0, _IONBF, 0);

  if (!apply_patches(in, out, &i)) {
    goto rollback;
  }
  if (i == 0) {
    fprintf(stderr, "Sorry, no matches found.\n");
    goto rollback;
  } else if (i == 1) {
    fprintf(stderr, "1 patch applied.\n");
  } else {
    fprintf(stderr, "%d patches applied.\n", i);
  }

  fclose(out);
  fclose(in);
  return;

 rollback:
  fprintf(stderr, "Patch failed.\n");
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

  in = fopen(src_filename, "rb");
  if (!in) {
    fprintf(stderr, "Error: Can't open input file for reading: %s\n", strerror(errno));
    exit(1);
  }
  setvbuf(in, 0, _IONBF, 0);

  out = fopen(dst_filename, "wb");
  if (!out) {
    fprintf(stderr, "Error: Can't open output file for writing: %s\n", strerror(errno));
    exit(1);
  }
  setvbuf(out, 0, _IONBF, 0);

  if (!apply_patches(in, out, 0)) {
    exit(1);
  }

  fclose(out);
  fclose(in);
}

static void usage() {
  fprintf(stderr,
          "ADPATCH " XSTR(VERSION) "\n"
          "\n"
          "Usage: adpatch [OPTIONS...] -i FILE\n"
          "       adpatch [OPTIONS...] INPUT-FILE OUTPUT-FILE\n"
          "\n"
          "Options:\n"
          "  -i            Patch in-place\n"
          "  -a            Ask before applying a patch\n"
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
  bool inplace = false;
  int opt;

  while ((opt = getopt(argc, argv, "ip:a")) != -1) {
    switch (opt) {
    case 'i':
      inplace = true;
      break;
    case 'p':
      patch_port = parse_port(optarg);
      break;
    case 'a':
      patch_ask = true;
      break;
    default:
      usage();
    }
  }

  if (!patch_port) {
    patch_port = parse_port(DEFAULT_PORT);
  }

  if (inplace) {
    if (optind + 1 != argc) {
      usage();
    }
    in_place_mode(argv[optind]);
  } else {
    if (optind + 2 != argc) {
      usage();
    }
    copy_mode(argv[optind], argv[optind + 1]);
  }
  return 0;
}
