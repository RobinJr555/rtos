#ifndef __STM32_CLOCK_H
#define __STM32_CLOCK_H

enum clock {
	CLOCK_CORE,
	CLOCK_AHB,
	CLOCK_APB1,
	CLOCK_APB2
};


void stm32_flash_latency_cfg(int latency);

#endif
