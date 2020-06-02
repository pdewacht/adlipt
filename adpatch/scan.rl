/* -*- Mode: C -*- */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "adpatch.h"


%%{
  machine AdLibScanner;
  alphtype unsigned char;
  include "patterns.rl";
  write data nofinal;
}%%


#define BUFSIZE (31*1024)


#define WARN(msg)                                               \
  printf("Warning: %s\n", msg);

#define FATAL(msg)                                              \
  WARN(msg)                                                     \
  goto fail;

#define MATCH(n)                                                \
  if (num_matches == MAX_MATCHES) goto too_many_matches;        \
  matches[num_matches].patch = &patch_data[n];                  \
  matches[num_matches].offset = fpos + (ts - buf);              \
  matches[num_matches].len = te - ts;                           \
  num_matches++;


int scan_and_copy(FILE *in, FILE *out, struct match *matches) {
  static unsigned char buf[BUFSIZE];
  int cs, act, have = 0;
  unsigned char *ts, *te = 0;
  int done = 0;
  int num_matches = 0;
  long fpos = 0;

  %% write init;

  while (!done) {
    unsigned char *p = buf + have, *pe, *eof = 0;
    int len, space = BUFSIZE - have;

    if (space == 0) {
      /* We've used up the entire buffer storing an already-parsed token
       * prefix that must be preserved. */
      fprintf(stderr, "Internal error: out of buffer space [0x%lX]\n", fpos-have);
      return -1;
    }

    len = fread(p, 1, space, in);
    pe = p + len;

    /* Check if this is the end of file. */
    if (len < space) {
      eof = pe;
      done = 1;
    }

    %% write exec;

    if (cs == AdLibScanner_error) {
      fprintf(stderr, "Internal error: scan error\n");
      return -1;
    }

    len = ts ? (ts - buf) : (pe - buf);
    fpos += len;
    if (out && fwrite(buf, 1, len, out) != len) {
      perror("Write error");
      return -1;
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

  return num_matches;

 too_many_matches:
  fprintf(stderr, "Too many matching patterns, something is wrong\n");
 fail:
  return -1;
}
