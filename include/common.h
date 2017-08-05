#ifndef __RTOS_COMMON_H
#define __RTOS_COMMON_H

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "common/list.h"
#include "common/errno.h"
#include "common/byteorder.h"

#include "board_config.h"

#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof((arr)[0]))

#if CONFIG_ENABLE_DEBUG
#define dbg(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

#if CONFIG_RTOS_PRINTF
#define printf		rtos_printf
#endif

/**
 * std_outbyte is at the lowest level function of printf(),
 * it is used to output a character by uart.
 */
typedef void (*outbyte)(char ch);
extern outbyte std_outbyte;

/**
 * std_inbyte is at the lowest level function of scanf(),
 * it is used to get a character from uart.
 */
typedef unsigned char (*inbyte)(void);
extern inbyte std_inbyte;

/* recommand using rtos_printf() instead of printf() */
void rtos_printf(char* format, ...);

#if CONFIG_EARLY_PRINTF
void early_console_init(void);
#endif

void machine_init(void);
void device_init(void);

#endif
