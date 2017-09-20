#ifndef __DRIVER_PLATFORM_H
#define __DRIVER_PLARFORM_H

#include "driver/device.h"
#include "driver/device_table.h"

struct resource;
struct pinctrl_group;

struct platform_device {
	const char *name;
#if CONFIG_PINCTRL
	const char *pinctrl;
	struct pinctrl_groups *pingrp;
#endif
	int id;
	struct device dev;
	struct resource *resource;
	unsigned int num_resources;
	void *private_data;
};

#define to_platform_device(x) container_of((x), struct platform_device, dev)

struct platform_driver {
	struct device_driver driver;
	struct platform_device_id *id_table;
};

#define to_platform_driver(drv)	(container_of((drv), struct platform_driver, \
				 driver))

extern struct resource *platform_get_resource(struct platform_device *dev,
				       unsigned int type, unsigned int num);
extern int platform_device_register(struct platform_device *pdev);
extern int platform_driver_register(struct platform_driver *pdrv);

extern int platform_bus_init(void);
#endif
