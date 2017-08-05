#ifndef __DRIVER__BASE_H
#define __DRIVER_BASE_H

#include "driver/device.h"


typedef void (*__driver_init)(void);
#define module_init(func_init) \
	static __driver_init driver_##func_init __attribute__((used)) \
	__attribute__((section(".driver.init"))) = func_init

static inline int driver_match_device(struct device_driver *drv,
				      struct device *dev)
{
	return drv->bus->match ? drv->bus->match(dev, drv) : 1;
}

extern int bus_add_device(struct device *dev);
extern void bus_probe_device(struct device *dev);
extern void bus_remove_device(struct device *dev);

extern int bus_add_driver(struct device_driver *drv);
extern void bus_remove_driver(struct device_driver *drv);

#endif
