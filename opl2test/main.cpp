#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/ppdev.h>
#include "OPL2.h"
#include "demotune.h"


static int setup(char *parport)
{
  int fd = -1;
  if ((fd = open(parport, O_RDWR)) < 0) {
    fprintf(stderr, "cannot open %s: %m\n", parport);
    goto err;
  }
  if (ioctl(fd, PPCLAIM) < 0) {
    fprintf(stderr, "cannot claim parallel port: %m\n");
    goto err;
  }

  return fd;
err:
  if (fd >= 0) close(fd);
  return -1;
}

static int interrupted = 0;
static void sigint(int) {
  interrupted = 1;
}

int main(int argc, char *argv[])
{
  printf("== OPL2LPT test program ==\n\n");
  if (argc != 2) {
    printf("First argument should be path to parallel port (eg /dev/parport0)\n");
    return 1;
  }

  int lpt_base = setup(argv[1]);
  if (lpt_base < 0) return 1;

  extern OPL2 opl2;
  opl2.init(lpt_base);
  music_setup();
  printf("\n\nPress Ctrl-C to stop...");
  fflush(stdout);
  signal(SIGINT, sigint);
  while (!interrupted) {
    music_loop();
  }
  music_shutdown();
  printf("\n\n");
  return 0;
}
