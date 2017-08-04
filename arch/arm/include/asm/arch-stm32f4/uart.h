#ifndef __STM32_UART_H
#define __STM32_UART_H


struct stm32_uart_regs {
	uint32_t sr;		/* UART status */
	uint32_t dr;		/* UART data */
	uint32_t brr;		/* UART baund rate */
	uint32_t cr1;		/* UART control 1 */
	uint32_t cr2;		/* UART control 2 */
	uint32_t cr3;		/* UART control 3 */
	uint32_t gtpr;		/* UART guard time and prescaler */
};

#endif /* __STM32_UART_H */
