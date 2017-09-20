#include "common.h"
#include "asm/arch/base.h"
#include "driver/clock.h"


/*----------------------------------------------------------
 * Define clocks
 *--------------------------------------------------------*/
#define  SYSTEM_CLOCK    ( CONFIG_SYS_CLK_FREQ )

/*----------------------------------------------------------
 * Externals
 *--------------------------------------------------------*/
extern uint32_t __Vectors;

/*----------------------------------------------------------
 * System Core Clock Variable
 *--------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;

/**********************************************************
 * System Core Clock update function
 *********************************************************/
void SystemCoreClockUpdate (uint32_t system_clock)
{
	SystemCoreClock = system_clock;
	clk_update(system_clock);
}

/**********************************************************
 * System initialization function
 *********************************************************/
void SystemInit (void)
{
	SCB->VTOR = (uint32_t) &__Vectors;

#if defined (__FPU_USED) && (__FPU_USED == 1U)
	SCB->CPACR |= ((3U << 10U*2U) |			/* set CP10 Full Access */
					(3U << 11U*2U));		/* set CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
	SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

	SystemCoreClockUpdate(SYSTEM_CLOCK);
}
