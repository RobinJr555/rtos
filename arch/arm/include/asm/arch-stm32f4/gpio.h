#ifndef __STM32_GPIO_H
#define __STM32_GPIO_H

enum stm32_gpio_mode {
	GPIO_MODE_IN = 0,
	GPIO_MODE_OUT,
	GPIO_MODE_AF,
	GPIO_MODE_AN
};
enum stm32_gpio_otype {
	GPIO_OTYPE_PP = 0,
	GPIO_OTYPE_OD
};

enum stm32_gpio_speed {
	GPIO_SPEED_2M = 0,
	GPIO_SPEED_25M,
	GPIO_SPEED_50M,
	GPIO_SPEED_100M
};

enum stm32_gpio_pupd {
	GPIO_PUPD_NO = 0,
	GPIO_PUPD_UP,
	GPIO_PUPD_DOWN
};

enum stm32_gpio_af {
	GPIO_AF0 = 0,
	GPIO_AF1,
	GPIO_AF2,
	GPIO_AF3,
	GPIO_AF4,
	GPIO_AF5,
	GPIO_AF6,
	GPIO_AF7,
	GPIO_AF8,
	GPIO_AF9,
	GPIO_AF10,
	GPIO_AF11,
	GPIO_AF12,
	GPIO_AF13,
	GPIO_AF14,
	GPIO_AF15
};

#define GPIO_AF_TIM1	GPIO_AF1
#define GPIO_AF_TIM2	GPIO_AF1

#define GPIO_AF_TIM3	GPIO_AF2
#define GPIO_AF_TIM4	GPIO_AF2
#define GPIO_AF_TIM5	GPIO_AF2

#define GPIO_AF_TIM8	GPIO_AF3
#define GPIO_AF_TIM9	GPIO_AF3
#define GPIO_AF_TIM10	GPIO_AF3
#define GPIO_AF_TIM11	GPIO_AF3

#define GPIO_AF_I2C1	GPIO_AF4
#define GPIO_AF_I2C2	GPIO_AF4
#define GPIO_AF_I2C3	GPIO_AF4

#define GPIO_AF_SPI1	GPIO_AF5
#define GPIO_AF_SPI2	GPIO_AF5
#define GPIO_AF_SPI4	GPIO_AF5
#define GPIO_AF_SPI5	GPIO_AF5
#define GPIO_AF_SPI6	GPIO_AF5

#define GPIO_AF_SPI3	GPIO_AF6

#define GPIO_AF_UART1	GPIO_AF7
#define GPIO_AF_UART2	GPIO_AF7
#define GPIO_AF_UART3	GPIO_AF7

#define GPIO_AF_UART4	GPIO_AF8
#define GPIO_AF_UART5	GPIO_AF8
#define GPIO_AF_UART6	GPIO_AF8
#define GPIO_AF_UART7	GPIO_AF8
#define GPIO_AF_UART8	GPIO_AF8

#define GPIO_AF_CAN1	GPIO_AF9
#define GPIO_AF_CAN2	GPIO_AF9
#define GPIO_AF_TIM12	GPIO_AF9
#define GPIO_AF_TIM13	GPIO_AF9
#define GPIO_AF_TIM14	GPIO_AF9

#define GPIO_AF_OTG_FS	GPIO_AF10
#define GPIO_AF_OTG_HS	GPIO_AF10

#define GPIO_AF_ETH		GPIO_AF11

#define GPIO_AF_FSMC	GPIO_AF12
#define GPIO_AF_SDIO	GPIO_AF12

#define GPIO_AF_DCMI	GPIO_AF13

#define GPIO_AF_EVENTPUT	GPIO_AF15

struct stm32_config {
	unsigned char af	:4;
	unsigned char mode	:2;
	unsigned char pupd	:2;
	unsigned char otype	:1;
	unsigned char value	:1;
};

struct stm32_gpio_regs {
	uint32_t moder;		/* GPIO port mode */
	uint32_t otyper;	/* GPIO port output type */
	uint32_t ospeedr;	/* GPIO port output speed */
	uint32_t pupdr;		/* GPIO port pull-up/pull-down */
	uint32_t idr;		/* GPIO port input data */
	uint32_t odr;		/* GPIO port output data */
	uint32_t bsrr;		/* GPIO port bit set/reset */
	uint32_t lckr;		/* GPIO port configuration lock */
	uint32_t afr[2];	/* GPIO alternate function */
};

#define GPIOA(n)		((0 << 4) + n)
#define GPIOB(n)		((1 << 4) + n)
#define GPIOC(n)		((2 << 4) + n)
#define GPIOD(n)		((3 << 4) + n)
#define GPIOE(n)		((4 << 4) + n)
#define GPIOF(n)		((5 << 4) + n)
#define GPIOG(n)		((6 << 4) + n)
#define GPIOH(n)		((7 << 4) + n)
#define GPIOI(n)		((8 << 4) + n)

#endif /* _STM32_GPIO_H_ */
