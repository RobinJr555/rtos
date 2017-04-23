#ifndef CMSIS_RTX_H
#define CMSIS_RTX_H

#include <sys_config.h>

//
// <h>Thread Configuration
// =======================
//
//   <o>Number of concurrent running threads <0-250>
//   <i> Defines max. number of threads that will run at the same time.
//       counting "main", but not counting "osTimerThread"
//   <i> Default: 6
extern unsigned char        __StackTop[];
#define INITIAL_SP          (__StackTop)


// <h>Thread Configuration
// =======================
//
//   <o>Number of concurrent running threads <0-250>
//   <i> Defines max. number of threads that will run at the same time.
//       counting "main", but not counting "osTimerThread"
//   <i> Default: 6
#define OS_TASKCNT          14

//   <o>Scheduler (+ interrupts) stack size [bytes] <64-4096:8><#/4>
#define OS_MAINSTKSIZE      256

// </h>
// <h>SysTick Timer Configuration
// ==============================
//
//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Defines the timer clock value.
//   <i> Default: 6000000  (6MHz)

#define OS_CLOCK            CONFIG_SYS_CLK_FREQ


#endif
