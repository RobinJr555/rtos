#include "stm32f4xx.h"

/**********************************************************
 * SystemInit()
 *********************************************************/
extern uint32_t __vector_table;
void SystemInit (void)
{
	SCB->VTOR = __vector_table;
}

/**********************************************************
 * SystemCoreClockUpdate
 *********************************************************/
void SystemCoreClockUpdate (void)
{

}
