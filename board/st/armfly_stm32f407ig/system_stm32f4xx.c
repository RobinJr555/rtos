#include "common.h"
#include "asm/io.h"
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
 * System FSMC function
 *********************************************************/
void SystemInit_ExtMemCtl(void)
{
/*
 +-------------------+--------------------+------------------+-------------------+
 +                               RAM pins assignment                             +
 +-------------------+--------------------+------------------+-------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10  |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11  |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12  |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13  |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14  |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15  |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2  |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 | PG10 <-> FSMC_NE3 |
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 | PG12 <-> FSMC_NE4 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |------------------+|
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/
	/* Enable GPIOD, GPIOE, GPIOF and GPIOG interface clock */
	setbits_le32(RCC_BASE + 0x30, 0x78);

	/* Connect PDx pins to FSMC Alternate function */
	writel(0x00cc00cc, GPIOD_BASE + 0x20);
	writel(0xcccccccc, GPIOD_BASE + 0x24);
	/* Configure PDx pins in Alternate function mode */
	writel(0xaaaa0a0a, GPIOD_BASE + 0x00);
	/* Configure PDx pins speed to 100 MHz */
	writel(0xffff0f0f, GPIOD_BASE + 0x08);
	/* Configure PDx pins Output type to push-pull */
	writel(0x00000000, GPIOD_BASE + 0x04);
	/* No pull-up, pull-down for PDx pins */
	writel(0x00000000, GPIOD_BASE + 0x0c);

	/* Connect PEx pins to FSMC Alternate function */
	writel(0xcccccccc, GPIOE_BASE + 0x20);
	writel(0xcccccccc, GPIOE_BASE + 0x24);
	/* Configure PEx pins in Alternate function mode */
	writel(0xaaaaaaaa, GPIOE_BASE + 0x00);
	/* Configure PEx pins speed to 100 MHz */
	writel(0xffffffff, GPIOE_BASE + 0x08);
	/* Configure PEx pins Output type to push-pull */
	writel(0x00000000, GPIOE_BASE + 0x04);
	/* No pull-up, pull-down for PEx pins */
	writel(0x00000000, GPIOE_BASE + 0x0c);

	/* Connect PFx pins to FSMC Alternate function */
	writel(0x00cccccc, GPIOF_BASE + 0x20);
	writel(0xcccc0000, GPIOF_BASE + 0x24);
	/* Configure PFx pins in Alternate function mode */
	writel(0xaa000aaa, GPIOF_BASE + 0x00);
	/* Configure PFx pins speed to 100 MHz */
	writel(0xff000fff, GPIOF_BASE + 0x08);
	/* Configure PFx pins Output type to push-pull */
	writel(0x00000000, GPIOF_BASE + 0x04);
	/* No pull-up, pull-down for PFx pins */
	writel(0x00000000, GPIOF_BASE + 0x0c);

	/* Connect PGx pins to FSMC Alternate function */
	writel(0x00cccccc, GPIOF_BASE + 0x20);
	writel(0x000c0cc0, GPIOF_BASE + 0x24);
	/* Configure PGx pins in Alternate function mode */
	writel(0x02280aaa, GPIOG_BASE + 0x00);
	/* Configure PGx pins speed to 100 MHz */
	writel(0x02280fff, GPIOG_BASE + 0x08);
	/* Configure PGx pins Output type to push-pull */
	writel(0x00000000, GPIOG_BASE + 0x04);
	/* No pull-up, pull-down for PGx pins */
	writel(0x00000000, GPIOG_BASE + 0x0c);

	/* Enable the FSMC interface clock */
	setbits_le32(RCC_BASE + 0x38, 0x1);

	/* Configure and enable Bank1_NOR2 */
	writel(FSMC_R_BASE + 8*(3-1), 0x00001059);
	writel(FSMC_R_BASE + 0x04 + 8*(3-1), 0x00000705);
	writel(FSMC_R_BASE + 0x104 + 8*(3-1), 0x00000705);

	/* Configure and enable Bank1_SRAM3 */
	writel(FSMC_R_BASE + 8*(3-1), 0x00001011);
	writel(FSMC_R_BASE + 0x04 + 8*(3-1), 0x00010203);
	writel(FSMC_R_BASE + 0x104 + 8*(3-1), 0x00010203);

	/* Configure and enable Bank1_SRAM4 */
	writel(FSMC_R_BASE + 8*(3-1), 0x00001011);
	writel(FSMC_R_BASE + 0x04 + 8*(3-1), 0x00000201);
	writel(FSMC_R_BASE + 0x104 + 8*(3-1), 0x00000201);
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

#ifdef CONFIG_EXT_MEM_CTL
	SystemInit_ExtMemCtl();
#endif
}
