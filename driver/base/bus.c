#include "common.h"
#include "driver/base.h"
#include "driver/device.h"

LIST_HEAD(bus_node_root);


int bus_find_driver(const char *name, struct bus_node *bus)
{
	struct device_driver *drv;
	list_for_each_entry(drv, &bus->dev->driver_list, driver_list) {
		if (!strcmp(drv->name, name))
			return 0;
	}

	return -ENODEV;
}

int bus_for_each_drv(struct bus_node *bus, struct device_driver *start,
		     void *data, int (*fn)(struct device_driver *, void *))
{
	struct device_driver *drv;
	int error = 0;

	if(!bus)
		return -EINVAL;

	list_for_each_entry(drv, &bus->dev->driver_list, driver_list) {
		if (!error)
			error = fn(drv, data);
	}

	return error;
}
/**
 * bus_for_each_dev - device iterator.
 * @bus: bus type.
 * @start: device to start iterating from.
 * @data: data for the callback.
 * @fn: function to be called for each device.
 *
 * Iterate over @bus's list of devices, and call @fn for each,
 * passing it @data. If @start is not NULL, we use that device to
 * begin iterating from.
 *
 * We check the return of @fn each time. If it returns anything
 * other than 0, we break out and return that value.
 *
 * NOTE: The device that returns a non-zero value is not retained
 * in any way, nor is its refcount incremented. If the caller needs
 * to retain this data, it should do so, and increment the reference
 * count in the supplied callback.
 */
int bus_for_each_dev(struct bus_node *bus, struct device *start,
		     void *data, int (*fn)(struct device *, void *))
{
	struct device *dev;
	int error;

	if (!bus)
		return -EINVAL;

	list_for_each_entry(dev, &bus->dev->device_list, device_list) {
		while (!error)
			error = fn(dev, data);
	}

	return error;
}

static int __device_attach(struct device_driver *drv, void *data)
{
	struct device *dev = data;

	if (!driver_match_device(drv, dev))
		return 0;

	return driver_probe_device(drv, dev);
}

/**
 * device_attach - try to attach device to a driver.
 * @dev: device.
 *
 * Walk the list of drivers that the bus has and call
 * driver_probe_device() for each pair. If a compatible
 * pair is found, break out and return.
 *
 * Returns 1 if the device was bound to a driver;
 * 0 if no matching driver was found;
 * -ENODEV if the device is not registered.
 *
 * When called for a USB interface, @dev->parent lock must be held.
 */
int device_attach(struct device *dev)
{
	int ret = 0;

	if (!dev->driver) {
		ret = bus_for_each_drv(dev->bus, NULL, dev, __device_attach);
	}

	return ret;
}

/**
 * bus_add_device - add device to bus
 * @dev: device being added
 *
 * - Create links to device's bus.
 * - Add the device to its bus's list of devices.
 */
int bus_add_device(struct device *dev)
{
	struct bus_node *bus = dev->bus;
	int error = 0;

	if (bus) {
		dbg("bus: '%s': add device %s\n", bus->dev->name, dev->name);
		list_add_tail(&dev->device_list, &bus->dev->device_list);
	}
	return error;
}

/**
 * bus_probe_device - probe drivers for a new device
 * @dev: device to probe
 *
 * - Automatically probe for a driver if the bus allows it.
 */
void bus_probe_device(struct device *dev)
{
	struct bus_node *bus = dev->bus;

	if (!bus)
		return;

	device_attach(dev);
}
/**
 * bus_remove_device - remove device from bus
 * @dev: device to be removed
 *
 * - Delete device from bus's list.
 * - Detach from its driver.
 * - Drop reference taken in bus_add_device().
 */
void bus_remove_device(struct device *dev)
{
	struct bus_node *bus = dev->bus;

	if (!bus)
		return;

	list_del(&dev->device_list);
	device_release_driver(dev);
	dbg("bus: '%s': remove device %s\n",
		dev->bus->dev->name, dev->name);
}


static int __driver_attach(struct device *dev, void *data)
{
	struct device_driver *drv = data;

	if (!driver_match_device(drv, dev))
		return 0;

	if (!dev->driver)
		driver_probe_device(drv, dev);

	return 0;
}

/**
 * driver_attach - try to bind driver to devices.
 * @drv: driver.
 *
 * Walk the list of devices that the bus has on it and try to
 * match the driver with each one.  If driver_probe_device()
 * returns 0 and the @dev->driver is set, we've found a
 * compatible pair.
 */
int driver_attach(struct device_driver *drv)
{
	return bus_for_each_dev(drv->bus, NULL, drv, __driver_attach);
}
/**
 * driver_detach - detach driver from all devices it controls.
 * @drv: driver.
 */
void driver_detach(struct device_driver *drv)
{
	list_del(&drv->driver_list);
}
/**
 * bus_add_driver - Add a driver to the bus.
 * @drv: driver.
 */
int bus_add_driver(struct device_driver *drv)
{
	struct bus_node *bus = drv->bus;
	int ret;

	if (!bus)
		return -EINVAL;

	dbg("bus '%s': add driver %s\n", bus->dev->name, drv->name);

	ret = bus_find_driver(drv->name, bus);
	if (ret)
		list_add_tail(&drv->driver_list, &bus->dev->driver_list);

	ret = driver_attach(drv);

	return ret;
}
/**
 * bus_remove_driver - delete driver from bus's knowledge.
 * @drv: driver.
 *
 * Detach the driver from the devices it controls, and remove
 * it from its bus's list of drivers. Finally, we drop the reference
 * to the bus we took in bus_add_driver().
 */
void bus_remove_driver(struct device_driver *drv)
{
	if (!drv->bus)
		return;

	driver_detach(drv);
	dbg("bus '%s': demove driver %s\n",
		drv->bus->dev->name, drv->name);
}

int bus_register(struct bus_node *bus_node)
{
	INIT_LIST_HEAD(&bus_node->node_list);
	list_add_tail(&bus_node->node_list, &bus_node_root);

	dbg("bus '%s': registered\n", bus_node->dev->name);
	return 0;
}
