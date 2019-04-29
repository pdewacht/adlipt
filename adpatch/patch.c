/* -*- Mode: C -*- */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "adpatch.h"
#include "patch.inc"


static int16_t get16(unsigned char *buf, int ofs) {
  return buf[ofs] | ((uint16_t)buf[ofs + 1] << 8);
}

static void put16(unsigned char *buf, int ofs, int16_t val) {
  buf[ofs] = val;
  buf[ofs + 1] = val >> 8;
}

static int32_t get32(unsigned char *buf, int ofs) {
  return buf[ofs] |
    ((uint16_t)buf[ofs + 1] << 8) |
    ((uint32_t)buf[ofs + 2] << 16) |
    ((uint32_t)buf[ofs + 3] << 24);
}

static void put32(unsigned char *buf, int ofs, int32_t val) {
  buf[ofs] = val;
  buf[ofs + 1] = (int)val >> 8;
  buf[ofs + 2] = val >> 16;
  buf[ofs + 3] = val >> 24;
}


static int apply_patch(FILE *out, struct match m, long *symbols) {
  unsigned char buf[500];
  int i;

  if (m.len > sizeof(buf)) {
    fprintf(stderr, "Internal error: %s: match too long\n", m.patch->name);
    return -1;
  }
  if (m.len < m.patch->code_len) {
    fprintf(stderr, "Internal error: %s: patch too long\n", m.patch->name);
    return -1;
  }

  memset(buf, 0x90, sizeof(buf));
  memcpy(buf, m.patch->code, m.patch->code_len);

  for (i = 0; i < MAX_RELOCATIONS; i++) {
    int s = m.patch->rel[i].symbol;
    int o = m.patch->rel[i].offset;
    long v = symbols[s];

    if (!s) {
      break;
    } else if (!v) {
      /* symbol not present. can't apply patch */
      return 0;
    }

    switch (m.patch->rel[i].type) {
    case R_386_PC16:
      v -= m.offset + o;
      if (v != (int16_t)v) {
        return 0;
      }
      /* FALLTHRU */
    case R_386_16:
      put16(buf, o, get16(buf, o) + v);
      break;
    case R_386_PC32:
      v -= m.offset + o;
      /* FALLTHRU */
    case R_386_32:
      put32(buf, o, get32(buf, o) + v);
      break;
    }
  }

  if (fseek(out, m.offset, SEEK_SET) != 0) {
    perror("Seek error");
    return -1;
  }

  if (fwrite(buf, 1, m.len, out) != m.len) {
    perror("Write error");
    return -1;
  }

  return 1;
}

int apply_patches(FILE *in, FILE *out, int lpt_port) {
  struct match matches[MAX_MATCHES];
  long symbols[SYMBOL_COUNT + 1] = { 0 };
  int num_matches;
  int applied_patches;
  int m;

  num_matches = scan_and_copy(in, out, matches);
  if (num_matches < 0) {
    return -1;
  }

  symbols[1] = lpt_port;
  for (m = 0; m < num_matches; m++) {
    int i;
    for (i = 0; i < MAX_EXPORTS; i++) {
      struct export e = matches[m].patch->export[i];
      if (e.symbol) {
        symbols[e.symbol] = matches[m].offset + e.offset;
      }
    }
  }

  applied_patches = 0;
  for (m = 0; m < num_matches; m++) {
    int r = apply_patch(out, matches[m], symbols);
    if (r < 0) {
      return r;
    }
    if (r > 0) {
      printf("Applied patch: %s\n", matches[m].patch->name);
      applied_patches++;
    }
  }

  switch (applied_patches) {
  case 0:
    printf("Sorry, no matches found.\n");
    break;
  case 1:
    printf("1 patch applied.\n");
    break;
  default:
    printf("%d patches applied.\n", applied_patches);
    break;
  }

  return applied_patches;
}
