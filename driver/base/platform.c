#include "common.h"
#include "driver/base.h"
#include "driver/device.h"
#include "driver/platform.h"
#include "driver/resource.h"


struct device platform_bus = {
	.name = "platform",
};

struct bus_node platform_bus_node;

/**
 * platform_get_resource - get a resource for a device
 * @dev: platform device
 * @type: resource type
 * @num: resource index
 */
struct resource *platform_get_resource(struct platform_device *dev,
				       unsigned int type, unsigned int num)
{
	unsigned int i;

	for (i = 0; i < dev->num_resources; i++) {
		struct resource *r = &dev->resource[i];

		if ((type & r->flags) && (num-- == 0))
			return r;
	}
	return NULL;
}

static const struct platform_device_id *platform_match_id(
			const struct platform_device_id *id,
			struct platform_device *pdev)
{
	while (id->name[0]) {
		if (strcmp(pdev->dev.name, id->name) == 0) {
			return id;
		}
		id++;
	}
	return NULL;
}

/**
 * platform_match - bind platform device to platform driver.
 * @dev: device.
 * @drv: driver.
 *
 * Platform device IDs are assumed to be encoded like this:
 * "<name><instance>", where <name> is a short description of the type of
 * device, like "pci" or "floppy", and <instance> is the enumerated
 * instance of the device, like '0' or '42'.  Driver IDs are simply
 * "<name>".  So, extract the <name> from the platform_device structure,
 * and compare it against the name of the driver. Return whether they match
 * or not.
 */
static int platform_match(struct device *dev, struct device_driver *drv)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct platform_driver *pdrv = to_platform_driver(drv);

//	/* Attempt an OF style match first */
//	if (of_driver_match_device(dev, drv))
//		return 1;

	/* Then try to match against the id table */
	if (pdrv->id_table)
		return platform_match_id(pdrv->id_table, pdev) != NULL;

	/* fall-back to driver name match */
	return (strcmp(dev->name, drv->name) == 0);
}

/**
 * __platform_driver_register - register a driver for platform-level devices
 * @drv: platform driver structure
 */
int platform_driver_register(struct platform_driver *pdrv)
{
	if (!pdrv->driver.name)
		return -EINVAL;

	pdrv->driver.bus = &platform_bus_node;
	return driver_register(&pdrv->driver);
}

/**
 * platform_device_add - add a platform device to device hierarchy
 * @pdev: platform device we're adding
 *
 * This is part 2 of platform_device_register()
 */
int platform_device_add(struct platform_device *pdev)
{
	int ret;

	if (!pdev)
		return -EINVAL;

	pdev->dev.bus = &platform_bus_node;
	dbg("Registering platform device '%s'\n", pdev->dev.name);
	ret = device_add(&pdev->dev);

	return ret;
}
/**
 * platform_device_register - add a platform-level device
 * @pdev: platform device we're adding
 */
int platform_device_register(struct platform_device *pdev)
{
	device_initialize(&pdev->dev);
	return platform_device_add(pdev);
}


struct bus_node platform_bus_node = {
	.node_type = NODE_PLATFORM,
	.dev = &platform_bus,
	.match = platform_match,
};

int platform_bus_init(void)
{
	int error;

	error = device_register(&platform_bus);
	if (error)
		return error;
	error = bus_register(&platform_bus_node);
	if(error)
		device_unregister(&platform_bus);
	return error;
}
