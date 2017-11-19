/* -*- Mode: C -*- */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "patch.h"

#define BUFSIZE (16*1024)

int patch_port;
bool patch_ask;

static int applied_patches;

%%{
  machine AdlibScanner;
  alphtype unsigned char;
  include "patterns.rl";
  write data nofinal;
}%%

static bool yes() {
  int c = getchar();
  bool r = (c == 'y' || c == 'Y');
  while (c != '\n' && c != EOF) {
    c = getchar();
  }
  return r;
}

static bool patch(const char *name,
                  unsigned char *buf,
                  unsigned char *buf_end,
                  const char *patch,
                  int patch_len,
                  int port_idx)
{
  if (!patch_ask) {
    fprintf(stderr, "Applying patch: %s\n", name);
  } else {
    fprintf(stderr, "Apply patch: %s? ", name);
    if (!yes()) {
      return true;
    }
  }
  if (buf_end - buf < patch_len) {
    fprintf(stderr, "Internal error: patch too long\n");
    return false;
  }
  memcpy(buf, patch, patch_len);
  memset(buf + patch_len, 0x90, buf_end - buf - patch_len);
  if (port_idx >= 0) {
    buf[port_idx] = patch_port & 0xFF;
    buf[port_idx + 1] = (patch_port >> 8) & 0xFF;
  }
  applied_patches += 1;
  return true;
}

bool apply_patches(FILE *in, FILE *out, int *out_applied_patches) {
  static unsigned char buf[BUFSIZE];
  int cs, act, have = 0;
  unsigned char *ts, *te = 0;
  int done = 0;

  applied_patches = 0;

  %% write init;

  while (!done) {
    unsigned char *p = buf + have, *pe, *eof = 0;
    int len, space = BUFSIZE - have;

    if (space == 0) {
      /* We've used up the entire buffer storing an already-parsed token
       * prefix that must be preserved. */
      fprintf(stderr, "Internal error: out of buffer space\n");
      return false;
    }

    len = fread(p, 1, space, in);
    pe = p + len;

    /* Check if this is the end of file. */
    if (len < space) {
      eof = pe;
      done = 1;
    }

    %% write exec;

    if (cs == AdlibScanner_error) {
      fprintf(stderr, "Internal error: parse error\n");
      return false;
    }

    len = ts ? (ts - buf) : (pe - buf);
    if (fwrite(buf, 1, len, out) != len) {
      fprintf(stderr, "Write error\n");
      return false;
    }

    if (ts == 0) {
      have = 0;
    } else {
      /* There is a prefix to preserve, shift it over. */
      have = pe - ts;
      memmove(buf, ts, have);
      te = buf + (te - ts);
      ts = buf;
    }
  }

  if (out_applied_patches) {
    *out_applied_patches = applied_patches;
  }
  return true;
}
