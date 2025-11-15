#ifndef __OS_KERNEL__
#define __OS_KERNEL__

#include <stdint.h>
#include "stm32f4xx.h"

void osKernelInit(void);
void osKernelLaunch(uint32_t quanta);
uint8_t osKernelAddThreads(void (*task0)(void), void (*task1)(void), void (*task2)(void));

void osThreadYield(void);

#endif
