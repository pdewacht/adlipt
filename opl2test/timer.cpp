#include <unistd.h>
#include <sys/time.h>

static unsigned frequency;

void timer_setup(unsigned f)
{
  frequency = f;
}

void timer_shutdown()
{
}

unsigned long timer_get() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * frequency + tv.tv_usec * frequency / 1000000;
}

void hlt() {
  usleep(1000000/frequency/10);
}
