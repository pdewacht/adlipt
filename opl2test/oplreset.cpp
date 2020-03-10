#include <string.h>
#include <i86.h>
#include "OPL2.h"

int main(int argc, char *argv[])
{
  int lpt = 1;
  if (argc > 1) {
    if (strnicmp(argv[1], "lpt2", 4)) {
      lpt = 2;
    } else if (strnicmp(argv[1], "lpt3", 4)) {
      lpt = 3;
    }
  }
  short port = *(short __far *)MK_FP(0x40, 6 + 2*lpt);
  OPL2().init(port);
  return 0;
}
