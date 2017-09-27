#include "common.h"
#include "driver/device.h"
#include "driver/pinctrl.h"
#include "driver/platform.h"

struct device pinctrl_bus = {
	.name = "pinctrl",
};

struct bus_node pinctrl_bus_node;

/**
 * pinctrl group operations
 */
static int pinctrl_get_group_count(struct pinctrl_group *group)
{
	return group->num_grps;
}

static const char *pinctrl_get_group_name(struct pinctrl_group *group,
		    unsigned selector)
{
	return group->groups[selector].name;
}

static int pinctrl_get_group_pins(struct pinctrl_group *group,
		    unsigned selector,
		    unsigned **pins,
		    unsigned *num_pins)
{
	*pins = (unsigned *)group->groups[selector].pins;
	*num_pins = group->groups[selector].num_pins;
	return 0;
}

static int pinctrl_get_device_pins(struct platform_device *pdev,
		    unsigned **pins,
		    unsigned *num_pins)
{
	*pins = (unsigned *)pdev->pingrp->pins;
	*num_pins = pdev->pingrp->num_pins;
	return 0;
}
static int pinctrl_group_match(struct device *dev, struct pinctrl_desc *desc)
{
	struct platform_device *pdev = to_platform_device(dev);

	if (!pdev->pinctrl || pdev->pingrp)
		return -EINVAL;

	unsigned char num_count = pinctrl_get_group_count(desc->group);
	for (unsigned char i = 0; i < num_count; i++) {
		if(!strcmp(pdev->pinctrl, pinctrl_get_group_name(desc->group, i)))
			return i;
	}
	return -ENODEV;
}

/**
 * pinctrl config operations
 */
int pinctrl_set_pin_config(struct device *dev, unsigned gpio,
		    unsigned long config)
{
	struct platform_device *pdev = to_platform_device(dev);
	if (!pdev->pingrp)
		return -EINVAL;

	struct pinctrl_desc *desc = (struct pinctrl_desc *)pinctrl_bus.private_data;
	if (!desc->confops)
		return -EINVAL;

	return desc->confops->pin_config_set(dev, gpio, config);
}

int pinctrl_get_pin_config(struct device *dev, unsigned gpio,
		    unsigned long *config)
{
	struct platform_device *pdev = to_platform_device(dev);
	if (!pdev->pingrp)
		return -EINVAL;

	struct pinctrl_desc *desc = (struct pinctrl_desc *)pinctrl_bus.private_data;
	if (!desc->confops)
		return -EINVAL;

	return desc->confops->pin_config_get(dev, gpio, config);
}

/**
 * pinctrl_bind_pins() - called by the device core before probe
 * @dev: the device that is just about to probe
 */
int pinctrl_bind_pins(struct device *dev)
{
	int ret = 0;

	if (dev->bus->node_type != NODE_PLATFORM)
		goto fail;

	struct pinctrl_desc *desc = (struct pinctrl_desc *)pinctrl_bus.private_data;
	if (!desc || !desc->group)
		goto fail;

	/* match pinctrl group and bind to device */
	int selector = pinctrl_group_match(dev, desc);
	if (selector < 0) {
		dbg("bus %s: device %s group name is not matched",
			pinctrl_bus.name,
			dev->name);
		ret = -ENODEV;
		goto fail;
	}
	struct platform_device *pdev = to_platform_device(dev);
	pdev->pingrp = (struct pinctrl_groups *)&desc->group->groups[selector];

	/* set device pins config */
	unsigned *pins; unsigned num_pins;
	unsigned long *config = (unsigned long *)pdev->pingrp->config;
	pinctrl_get_device_pins(pdev, &pins, &num_pins);
	for (unsigned char i = 0; i < num_pins; i++)
		pinctrl_set_pin_config(dev, pins[i], *config);

	ret = 0;
fail:
	return ret;
}

struct gpio_ops *pinctrl_gpio_ops_get(struct device *dev)
{
	struct pinctrl_desc *desc = (struct pinctrl_desc *)pinctrl_bus.private_data;

	if (!desc->pctrlops)
		return NULL;

	return desc->pctrlops;
}

void pinctrl_register(struct pinctrl_desc *desc)
{
	if (desc)
		pinctrl_bus.private_data = (void *)desc;
}

struct bus_node pinctrl_bus_node = {
	.node_type = NODE_PINCTRL,
	.dev = &pinctrl_bus,
};

int pinctrl_bus_init(void)
{
	int error;

	error = device_register(&pinctrl_bus);
	if (error)
		return error;
	error = bus_register(&pinctrl_bus_node);
	if(error)
		device_unregister(&pinctrl_bus);
	return error;
}
