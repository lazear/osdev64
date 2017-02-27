#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>

void pit_init(uint32_t frequency);
void pit_shutdown();
uint64_t pit_get_ticks(void);

#endif