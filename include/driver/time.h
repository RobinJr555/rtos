#ifndef __DRIVER_TIME_H
#define __DRIVER_TIME_H

#include "cmsis_os.h"


#define get_timer()		osKernelSysTick()
#define mdelay(x)		osDelay(x)

/*
 * for loop need at least 2 assembly codes: add and compare, so we use
 * CONFIG_SYS_CLK_FREQ / 2MHz times for 1us
 */
static inline void udelay(uint32_t time)
{
	while (time--) {
		for (uint8_t i = 0; i < CONFIG_SYS_CLK_FREQ / 2000000; i++);
	}
}

#endif
