#include "common.h"
#include "driver/base.h"
#include "driver/clock.h"
#include "driver/pinctrl.h"
#include "driver/platform.h"
#include "asm/io.h"
#include "asm/arch/base.h"
#include "asm/arch/clock.h"
#include "asm/arch/gpio.h"
#include "asm/arch/exti.h"

enum stm32_gpio_port {
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I
};

enum stm32_gpio_pin {
	GPIO_PIN_0 = 0,
	GPIO_PIN_1,
	GPIO_PIN_2,
	GPIO_PIN_3,
	GPIO_PIN_4,
	GPIO_PIN_5,
	GPIO_PIN_6,
	GPIO_PIN_7,
	GPIO_PIN_8,
	GPIO_PIN_9,
	GPIO_PIN_10,
	GPIO_PIN_11,
	GPIO_PIN_12,
	GPIO_PIN_13,
	GPIO_PIN_14,
	GPIO_PIN_15
};

struct stm32_gpio_dsc {
	enum stm32_gpio_port	port;
	enum stm32_gpio_pin		pin;
};

struct stm32_gpio_ctl {
	enum stm32_gpio_mode	mode;
	enum stm32_gpio_otype	otype;
	enum stm32_gpio_speed	speed;
	enum stm32_gpio_pupd	pupd;
	enum stm32_gpio_af		af;
};

union u_config{
	struct stm32_config conf;
	unsigned long value;
};

static uint16_t stm32_pins_state[CONFIG_GPIO_NUM >> 4];
#define stm32_gpio_valid()	(stm32_pins_state[dsc.port] & (1 << dsc.pin))

#if defined(CONFIG_STM32F4) || defined(CONFIG_STM32F7)
static const unsigned long io_base[] = {
	GPIOA_BASE, GPIOB_BASE, GPIOC_BASE,
	GPIOD_BASE, GPIOE_BASE, GPIOF_BASE,
	GPIOG_BASE, GPIOH_BASE, GPIOI_BASE
};

#define CHECK_DSC(x)	(!x || x->port > 8 || x->pin > 15)
#define CHECK_CTL(x)	(!x || x->af > 15 || x->mode > 3 || x->otype > 1 || \
			x->pupd > 2 || x->speed > 3)

int stm32_gpio_config_set(const struct stm32_gpio_dsc *dsc,
		const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_regs *gpio_regs;
	uint32_t i;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}
	if (CHECK_CTL(ctl)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	/* enable pins clock first */
	clk_setup_periph(io_base[dsc->port]);

	i = (dsc->pin & 0x07) * 4;
	clrsetbits_le32(&gpio_regs->afr[dsc->pin >> 3], 0xF << i, ctl->af << i);


	i = dsc->pin;
	clrsetbits_le32(&gpio_regs->otyper, 0x1 << i, ctl->otype << i);
	i <<= 1;
	clrsetbits_le32(&gpio_regs->moder, 0x3 << i, ctl->mode << i);
	clrsetbits_le32(&gpio_regs->ospeedr, 0x3 << i, ctl->speed << i);
	clrsetbits_le32(&gpio_regs->pupdr, 0x3 << i, ctl->pupd << i);

	rv = 0;
out:
	return rv;
}

int stm32_gpio_config_get(const struct stm32_gpio_dsc *dsc,
		struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_regs *gpio_regs;
	uint32_t i;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}
	if (CHECK_CTL(ctl)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	i = (dsc->pin & 0x07) * 4;
	ctl->af = (readl(&gpio_regs->afr[dsc->pin >> 3]) >> i) & 0xF;

	i = dsc->pin;
	ctl->otype = (readl(&gpio_regs->otyper) >> i) & 0x1;
	i <<= 1;
	ctl->mode = (readl(&gpio_regs->moder) >> i) & 0x3;
	ctl->pupd = (readl(&gpio_regs->pupdr) >> i) & 0x3;

	rv = 0;
out:
	return rv;
}
#elif defined(CONFIG_STM32F1)
static const unsigned long io_base[] = {
	GPIOA_BASE, GPIOB_BASE, GPIOC_BASE,
	GPIOD_BASE, GPIOE_BASE, GPIOF_BASE,
	GPIOG_BASE
};

#define GPIO_CR_MODE_MASK		0x3
#define GPIO_CR_MODE_SHIFT(p)	(p * 4)
#define GPIO_CR_CNF_MASK		0x3
#define GPIO_CR_CNF_SHIFT(p)	(p * 4 + 2)

struct stm32_gpio_regs {
	uint32_t crl;		/* GPIO port configuration low */
	uint32_t crh;		/* GPIO port configuration high */
	uint32_t idr;		/* GPIO port input data */
	uint32_t odr;		/* GPIO port output data */
	uint32_t bsrr;		/* GPIO port bit set/reset */
	uint32_t brr;		/* GPIO port bit reset */
	uint32_t lckr;		/* GPIO port configuration lock */
};

#define CHECK_DSC(x)	(!x || x->port > 6 || x->pin > 15)
#define CHECK_CTL(x)	(!x || x->mode > 3 || x->icnf > 3 || x->ocnf > 3 || \
			 x->pupd > 1)

int stm32_gpio_config_set(const struct stm32_gpio_dsc *dsc,
		const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_regs *gpio_regs;
	uint32_t *cr;
	int p, crp;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}
	if (CHECK_CTL(ctl)) {
		rv = -EINVAL;
		goto out;
	}

	p = dsc->pin;

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	if (p < 8) {
		cr = &gpio_regs->crl;
		crp = p;
	} else {
		cr = &gpio_regs->crh;
		crp = p - 8;
	}

	clrbits_le32(cr, 0x3 << GPIO_CR_MODE_SHIFT(crp));
	setbits_le32(cr, ctl->mode << GPIO_CR_MODE_SHIFT(crp));

	clrbits_le32(cr, 0x3 << GPIO_CR_CNF_SHIFT(crp));
	/* Inputs set the optional pull up / pull down */
	if (ctl->mode == STM32_GPIO_MODE_IN) {
		setbits_le32(cr, ctl->icnf << GPIO_CR_CNF_SHIFT(crp));
		clrbits_le32(&gpio_regs->odr, 0x1 << p);
		setbits_le32(&gpio_regs->odr, ctl->pupd << p);
	} else {
		setbits_le32(cr, ctl->ocnf << GPIO_CR_CNF_SHIFT(crp));
	}

	rv = 0;
out:
	return rv;
}

int stm32_gpio_config_get(const struct stm32_gpio_dsc *dsc,
		struct stm32_gpio_ctl *ctl)
{
	int rv;

	rv = 0;
out:
	return rv;
}
#else
#error STM32 family not supported
#endif

static inline unsigned stm32_gpio_to_port(unsigned gpio)
{
	return gpio / 16;
}

static inline unsigned stm32_gpio_to_pin(unsigned gpio)
{
	return gpio % 16;
}

static int stm32_gpout_set(const struct stm32_gpio_dsc *dsc, int state)
{
	struct stm32_gpio_regs	*gpio_regs;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	if (state)
		writel(1 << dsc->pin, &gpio_regs->bsrr);
	else
		writel(1 << (dsc->pin + 16), &gpio_regs->bsrr);

	rv = 0;
out:
	return rv;
}

static int stm32_gpin_get(const struct stm32_gpio_dsc *dsc)
{
	struct stm32_gpio_regs	*gpio_regs;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];
	rv = (readl(&gpio_regs->idr) >> dsc->pin) & 1;
out:
	return rv;
}

static int stm32_config_get(struct device *dev, unsigned gpio,
		    unsigned long *config)
{
	struct stm32_gpio_dsc dsc;
	struct stm32_gpio_ctl ctl;
	union u_config u_config;
	int res;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	res = stm32_gpio_config_get(&dsc, &ctl);
	if (res < 0)
		goto out;

#if defined(CONFIG_STM32F4) || defined(CONFIG_STM32F7)
	u_config.conf.af = ctl.af;
	u_config.conf.mode = ctl.mode;
	u_config.conf.pupd = ctl.pupd;
	u_config.conf.otype = ctl.otype;
#else
#error STM32 family not supported
#endif

	*config = u_config.value;
out:
	return res;
}

static int stm32_config_set(struct device *dev, unsigned gpio,
		    unsigned long config)
{
	struct stm32_gpio_dsc dsc;
	struct stm32_gpio_ctl ctl;
	union u_config u_config;
	int res;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	u_config.value = config;
#if defined(CONFIG_STM32F4) || defined(CONFIG_STM32F7)
	ctl.af = u_config.conf.af;
	ctl.mode = u_config.conf.mode;
	ctl.otype = u_config.conf.otype;
	ctl.pupd = u_config.conf.pupd;
	ctl.speed = GPIO_SPEED_50M;
#else
#error STM32 family not supported
#endif

	res = stm32_gpio_config_set(&dsc, &ctl);
	if (res < 0)
		goto out;
	res = stm32_gpout_set(&dsc, u_config.conf.value);
out:
	return res;
}

static const struct pinconf_ops stm32_pconf_ops = {
	.pin_config_get = stm32_config_get,
	.pin_config_set = stm32_config_set,
};

#define STM32_INPUT_CONFIG
#define STM32_OUTPUT_CONFIG

static int stm32_direction_input(struct device *dev, unsigned int gpio)
{
	union u_config u_config;
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (!stm32_gpio_valid())
		return -EINVAL;

#if defined(CONFIG_STM32F4) || defined(CONFIG_STM32F7)
	u_config.conf.af = GPIO_AF0;
	u_config.conf.mode = GPIO_MODE_IN;
	u_config.conf.pupd = GPIO_PUPD_NO;
#else
#error STM32 family not supported
#endif
	return stm32_config_set(dev, gpio, u_config.value);
}

static int stm32_direction_output(struct device *dev, unsigned int gpio,
		    int value)
{
	union u_config u_config;
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (!stm32_gpio_valid())
		return -EINVAL;

#if defined(CONFIG_STM32F4) || defined(CONFIG_STM32F7)
	u_config.conf.af = GPIO_AF0;
	u_config.conf.mode = GPIO_MODE_IN;
	u_config.conf.pupd = GPIO_PUPD_NO;
	u_config.conf.otype = GPIO_OTYPE_PP;
#else
#error STM32 family not supported
#endif
	return stm32_config_set(dev, gpio, u_config.value);
}

static int stm32_gpio_get_value(struct device *dev, unsigned int gpio)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (!stm32_gpio_valid())
		return -EINVAL;
	return stm32_gpin_get(&dsc);
}

static int stm32_gpio_set_value(struct device *dev, unsigned int gpio,
		    int value)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (!stm32_gpio_valid())
		return -EINVAL;
	return stm32_gpout_set(&dsc, value);
}

static int stm32_gpio_request(struct device *dev, unsigned int gpio)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (stm32_pins_state[dsc.port] & (1 << dsc.pin))
		return -EBUSY;
	else
		stm32_pins_state[dsc.port] |= 1 << dsc.pin;

	return 0;
}

static void stm32_gpio_free(struct device *dev, unsigned int gpio)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	if (stm32_pins_state[dsc.port] & (1 << dsc.pin))
		stm32_pins_state[dsc.port] &= ~(1 << dsc.pin);
}

static void stm32_gpio_irq_set(struct device *dev, unsigned int gpio, unsigned int flag)
{
	struct stm32_exti_regs *exti_regs = (struct stm32_exti_regs *)EXTI_BASE;
	unsigned char pin;

	pin = stm32_gpio_to_pin(gpio);
	if (flag)
		writel(1 << pin, &exti_regs->ftsr);
	else
		writel(1 << pin, &exti_regs->rtsr);
}

static void stm32_gpio_irq_enable(struct device *dev, unsigned int gpio, unsigned int enable)
{
	struct stm32_exti_regs *exti_regs = (struct stm32_exti_regs *)EXTI_BASE;
	unsigned char pin;

	pin = stm32_gpio_to_pin(gpio);
	enable = enable? 1: 0;
	writel(enable << pin, &exti_regs->imr);
}

static void stm32_gpio_irq_pend_clear(struct device *dev, unsigned int gpio)
{
	struct stm32_exti_regs *exti_regs = (struct stm32_exti_regs *)EXTI_BASE;
	unsigned char pin;

	pin = stm32_gpio_to_pin(gpio);
	writel(1 << pin, &exti_regs->pr);
}

static const struct gpio_ops stm32_gpio_ops = {
	.direction_input = stm32_direction_input,
	.direction_output = stm32_direction_output,
	.gpio_get_value = stm32_gpio_get_value,
	.gpio_set_value = stm32_gpio_set_value,
	.gpio_request = stm32_gpio_request,
	.gpio_free = stm32_gpio_free,

	.gpio_irq_set = stm32_gpio_irq_set,
	.gpio_irq_enable = stm32_gpio_irq_enable,
	.gpio_irq_pend_clear = stm32_gpio_irq_pend_clear,
};


static int stm32_pctrl_probe(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct pinctrl_desc *pctrl_desc = NULL;

	pctrl_desc = (struct pinctrl_desc *)malloc(sizeof(struct pinctrl_desc));
	memset(pctrl_desc, 0, sizeof(struct pinctrl_desc));
	if (!pctrl_desc)
		return -ENOMEM;

	printf("device '%s': %d pins\n", dev->name, CONFIG_GPIO_NUM);
	dev->private_data = (void *)pctrl_desc;
	pctrl_desc->pctrlops = (struct gpio_ops *)&stm32_gpio_ops;
	pctrl_desc->confops = (struct pinconf_ops *)&stm32_pconf_ops;
	pctrl_desc->group = (struct pinctrl_group *)pdev->private_data;
	pctrl_desc->npins = CONFIG_GPIO_NUM;
	pinctrl_register(pctrl_desc);

	return 0;
}

static struct platform_driver stm32_pctrl_platform_driver = {
	.driver = {
		.name = "stm32-pctrl",
		.probe = stm32_pctrl_probe,
	},
};

void stm32_pctrl_init(void)
{
	platform_driver_register(&stm32_pctrl_platform_driver);
}

module_init(stm32_pctrl_init);
