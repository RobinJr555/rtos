#include "common.h"
#include "driver/base.h"
#include "driver/clock.h"
#include "driver/gpio.h"
#include "driver/pinctrl.h"
#include "driver/platform.h"
#include "asm/io.h"
#include "asm/arch/clock.h"
#include "asm/arch/gpio.h"


static int stm32_gpio_probe(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	if (!pdev->pingrp)
		goto fail;

	struct gpio_ops *ops = pinctrl_gpio_ops_get(dev);
	if (!ops)
		goto fail;
	gpio_register(ops);

	clk_setup_dev(dev);

	return 0;
fail:
	return -ENODEV;
}

static struct platform_driver stm32_gpio_platform_driver = {
	.driver = {
		.name = "stm32-gpio",
		.probe = stm32_gpio_probe,
	},
};

void stm32_gpio_init(void)
{
	platform_driver_register(&stm32_gpio_platform_driver);
}

module_init(stm32_gpio_init);
