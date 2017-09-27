#ifndef __DRIVER_PINCTRL_H
#define __DRIVER_PINCTRL_H

#include "driver/gpio.h"

/**
 * struct pinctrl_groups - deal with groups of pins
 * @name: name for group
 * @pins: an array of pins describing all the pins in one group
 * @config: an array of the pins configuration
 * @num_pins: number of the pins in one group
 */
struct pinctrl_groups {
	const char *name;
	const unsigned int *pins;
	const void *config;
	const unsigned int num_pins;
};

struct pinctrl_group {
	const struct pinctrl_groups *groups;
	const unsigned int num_grps;
};

/**
 * struct pinctrl_desc - pin controller descriptor, register this to pin
 * control subsystem
 * @pctrlops: pin control operation vtable, to support global concepts like
 *	grouping of pins, this is optional.
 * @confops: pin config operations vtable, if you support pin configuration in
 *	your driver
 * @gpiofops: gpio operations vtable, to support io operation like set value
 * @group: an array of pinctrl_group descriptors
 *	this pin controller
 * @npins: number of pins
 */
struct pinctrl_desc {
	struct gpio_ops *pctrlops;
	struct pinconf_ops *confops;
	struct pinctrl_group *group;
	unsigned int npins;
};

/**
 * struct pinconf_ops - pin config operations, to be implemented by
 * pin configuration capable drivers.
 * @pin_config_get: get the config of a certain pin, if the requested config
 *	is not available on this controller this should return -ENOTSUPP
 *	and if it is available but disabled it should return -EINVAL
 * @pin_config_set: configure an individual pin
 */
struct pinconf_ops {
	int (*pin_config_get) (struct device *dev,
			       unsigned gpio,
			       unsigned long *config);
	int (*pin_config_set) (struct device *dev,
			       unsigned gpio,
			       unsigned long config);
};

extern int pinctrl_set_pin_config(struct device *dev, unsigned gpio,
		    unsigned long config);
extern int pinctrl_get_pin_config(struct device *dev, unsigned gpio,
		    unsigned long *config);
extern int pinctrl_bind_pins(struct device *dev);
extern struct gpio_ops *pinctrl_gpio_ops_get(struct device *dev);
extern void pinctrl_register(struct pinctrl_desc *pctrl_desc);
extern int pinctrl_bus_init(void);

#endif
