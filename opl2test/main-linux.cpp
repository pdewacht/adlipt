#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include "OPL2.h"
#include "demotune.h"

static int interrupted = 0;
static void sigint(int) {
  interrupted = 1;
}

int main(int argc, char *argv[])
{
  struct parport_list parports = {};
  struct parport *selected = NULL;
  int i, ret = 1, caps = CAP1284_RAW;

  printf("== OPL2LPT test program ==\n\n");
  if (ieee1284_find_ports(&parports, 0) != E1284_OK) {
    fprintf(stderr, "Cannot get a list of parallel ports\n");
    goto err;
  }
  if (parports.portc == 0) {
    fprintf(stderr, "No parallel ports found\n");
    goto err;
  }
  if (parports.portc == 1) {
    selected = parports.portv[0];
    printf("One parallel port found: %s\n", selected->name);
  } else {
    printf("Several parallel ports found:\n");
    for (i = 0; i < parports.portc; i++)
      printf("%2d: %s\n", i+1, parports.portv[i]->name);
    while (1) {
      printf("Which port? "); fflush(stdout);
      scanf("%d", &i);
      if (i > 0 && i <= parports.portc) break;
    }
    selected = parports.portv[i];
  }

  if (ieee1284_open(selected, F1284_EXCL, &caps) != E1284_OK) {
    fprintf(stderr, "Cannot open parallel port\n");
    goto err;
  }
  if (ieee1284_claim(selected) != E1284_OK) {
    fprintf(stderr, "Cannot claim parallel port\n");
    goto err;
  }

  extern OPL2 opl2;
  opl2.init(selected);
  music_setup();
  printf("\n\nPress Ctrl-C to stop...");
  fflush(stdout);
  signal(SIGINT, sigint);
  while (!interrupted) {
    music_loop();
  }
  music_shutdown();
  printf("\n\n");
  ret = 0;

err:
  if (selected) ieee1284_close(selected);
  if (parports.portc) ieee1284_free_ports(&parports);
  return ret;
}
