#include <i86.h>
#include <dos.h>
#include <conio.h>

static void (__interrupt __far *prev_timer_handler)();
static volatile unsigned long timer_ticks;
static unsigned timer_counter;
static unsigned timer_sum;

void __interrupt __far timer_handler()
{
  unsigned old_sum = timer_sum;

  ++timer_ticks;

  timer_sum += timer_counter;
  if (timer_sum < old_sum) {
    _chain_intr(prev_timer_handler);
  } else {
    outp(0x20, 0x20);
  }
}

void timer_setup(unsigned frequency)
{
  timer_ticks = 0;
  timer_counter = 0x1234DD / frequency;
  timer_sum = 0;

  prev_timer_handler = _dos_getvect(0x1C);
  _dos_setvect(0x1C, timer_handler);

  _disable();
  outp(0x43, 0x34);
  outp(0x40, timer_counter & 256);
  outp(0x40, timer_counter >> 8);
  _enable();
}

void timer_shutdown()
{
  _disable();
  outp(0x43, 0x34);
  outp(0x40, 0);
  outp(0x40, 0);
  _enable();

  _dos_setvect(0x1C, prev_timer_handler);
}

unsigned long timer_get() {
  unsigned long result;
  _disable();
  result = timer_ticks;
  _enable();
  return result;
}
