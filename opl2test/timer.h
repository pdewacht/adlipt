#ifndef TIMER_H_
#define TIMER_H_

void timer_setup(unsigned frequency);
void timer_shutdown();
unsigned long timer_get();

void hlt(void);
#ifndef __linux__
#pragma aux hlt = "hlt";
#endif

#endif
