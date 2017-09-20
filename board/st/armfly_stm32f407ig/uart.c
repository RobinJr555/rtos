#include "common.h"
#include "asm/arch/base.h"
#include "asm/arch/clock.h"
#include "asm/arch/gpio.h"
#include "asm/arch/uart.h"
#include "asm/io.h"
#include "driver/clock.h"


#define UART_STATUS_TXE	(1 << 7)

#define UART_TX_ENABLE	(1 << 3)
#define UART_ENABLE		(1 << 13)

#define UART_OVER8		(1 << 15)


void early_console_putchar(char ch)
{
	struct stm32_uart_regs *uart_regs;
	uart_regs = (struct stm32_uart_regs *)USART1_BASE;

	writeb(ch, &uart_regs->dr);
	while(!(readl(&uart_regs->sr) & UART_STATUS_TXE))	/* transmit data register empty */
		;
}

unsigned int early_console_buadrate(unsigned int baudrate)
{
	struct stm32_uart_regs *uart_regs;
	uart_regs = (struct stm32_uart_regs *)USART1_BASE;

	unsigned int apb2_freq = clk_get(CLOCK_APB2);
	unsigned int oversampling_8 = (readl(&uart_regs->cr1) & UART_OVER8) >> 15;

	unsigned int div = (apb2_freq * 25)/((2 - oversampling_8)*2*baudrate);
	unsigned int div_mant = div / 100;
	unsigned int div_fraq = ((div - div_mant*100) *16 + 50)/100;
	return (div_mant << 4 | div_fraq);
}

void early_console_init(void)
{
	struct stm32_gpio_regs *gpio_regs;
	struct stm32_uart_regs *uart_regs;

	clk_setup_periph(GPIOA_BASE);		/* eanble gpio_a clock */
	clk_setup_periph(USART1_BASE);		/* enable uart1 clock */

	gpio_regs = (struct stm32_gpio_regs *)GPIOA_BASE;
	/* PA9: uart1_tx */
	clrsetbits_le32(&gpio_regs->afr[1], 0xF << (9-8)*4, GPIO_AF_UART1 << (9-8)*4);
	clrsetbits_le32(&gpio_regs->moder, 0x3 << 9*2, GPIO_MODE_AF << 9*2);
	clrsetbits_le32(&gpio_regs->otyper, 0x1 << 9, GPIO_OTYPE_PP << 9);
	clrsetbits_le32(&gpio_regs->ospeedr, 0x3 << 9*2, GPIO_SPEED_50M << 9*2);
	clrsetbits_le32(&gpio_regs->pupdr, 0x3 << 9*2, GPIO_PUPD_UP << 9*2);

	/*  PA10: uart1_rx */
	clrsetbits_le32(&gpio_regs->afr[1], 0xF << (10-8)*4, GPIO_AF_UART1 << (10-8)*4);
	clrsetbits_le32(&gpio_regs->moder, 0x3 << 10*2, GPIO_MODE_AF << 10*2);
	clrsetbits_le32(&gpio_regs->otyper, 0x1 << 10, GPIO_PUPD_UP << 10);
	clrsetbits_le32(&gpio_regs->ospeedr, 0x3 << 10*2, GPIO_SPEED_50M << 10*2);
	clrsetbits_le32(&gpio_regs->pupdr + 0x0C, 0x3 << 10*2, GPIO_OTYPE_PP << 10*2);

	uart_regs = (struct stm32_uart_regs *)USART1_BASE;
	/* uart configuration */
	writel(early_console_buadrate(115200), &uart_regs->brr);	/* 115200bps */
	setbits_le32(&uart_regs->cr1, UART_TX_ENABLE);				/* tx enable*/
	setbits_le32(&uart_regs->cr1, UART_ENABLE);					/* uart enable*/

	std_outbyte = early_console_putchar;
}

