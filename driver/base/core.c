#include "driver/base.h"
#include "driver/device.h"
#include "driver/pinctrl.h"
#include "common.h"


/**
 * device_add - add device to device hierarchy.
 * @dev: device.
 *
 * This is part 2 of device_register(), though may be called
 * separately device_initialize() has been called separately.
 */
int device_add(struct device *dev)
{
	int error = -EINVAL;

	if (!dev)
		goto done;

	if (!dev->name)
		goto done;

	error = bus_add_device(dev);
	if (error)
		goto done;

	bus_probe_device(dev);

done:
	return error;
}

/**
 * device_initialize - init device structure.
 * @dev: device.
 *
 * This prepares the device for use by other layers by initializing
 * its fields.
 * It is the first half of device_register(), if called by
 * that function, though it can also be called separately, so one
 * may use @dev's fields. In particular, get_device()/put_device()
 * may be used for reference counting of @dev after calling this
 * function.
 */
void device_initialize(struct device *dev)
{
	dev->bus = NULL;
	INIT_LIST_HEAD(&dev->driver_list);
	INIT_LIST_HEAD(&dev->device_list);
}

/**
 * device_register - register a device with the system.
 * @dev: pointer to the device structure
 *
 * This happens in two clean steps - initialize the device
 * and add it to the system. The two steps can be called
 * separately, but this is the easiest and most common.
 * I.e. you should only call the two helpers separately if
 * have a clearly defined need to use and refcount the device
 * before it is added to the hierarchy.
 *
 * For more information, see the kerneldoc for device_initialize()
 * and device_add().
 */
int device_register(struct device *dev)
{
	device_initialize(dev);
	return device_add(dev);
}

/**
 * device_unregister - unregister device from system.
 * @dev: device going away.
 */
void device_unregister(struct device *dev)
{
	bus_remove_device(dev);
}

/**
 * driver_register - register driver with bus
 * @drv: driver to register
 *
 * We pass off most of the work to the bus_add_driver() call,
 * since most of the things we have to do deal with the bus
 * structures.
 */
int driver_register(struct device_driver *drv)
{
	int ret;

	ret = bus_add_driver(drv);
	return ret;
}
/**
 * driver_unregister - remove driver from system.
 * @drv: driver.
 *
 * Again, we pass off most of the work to the bus-level call.
 */
void driver_unregister(struct device_driver *drv)
{
	if (!drv) {
		dbg("Unexpected driver ungister!\n");
		return;
	}

	bus_remove_driver(drv);
}

/**
 * driver_probe_device - attempt to bind device & driver together
 * @drv: driver to bind a device to
 * @dev: device to try to bind to the driver
 *
 * This function returns -ENODEV if the device is not registered,
 * 1 if the device is bound successfully and 0 otherwise.
 *
 * This function must be called with @dev lock held.  When called for a
 * USB interface, @dev->parent lock must be held as well.
 */
int driver_probe_device(struct device_driver *drv, struct device *dev)
{
	int ret = 0;

	dev->driver = drv;

#if CONFIG_PINCTRL
	/* If using pinctrl, bind pins now before probing */
	ret = pinctrl_bind_pins(dev);
	if (ret)
		goto probe_failed;
#endif

	if (drv->probe) {
		ret = drv->probe(dev);
		if (ret)
			goto probe_failed;
	}
	goto done;

probe_failed:
	dev->driver = NULL;
	ret = 0;
done:
	return ret;
}

/**
 * device_release_driver - manually detach device from driver.
 * @dev: device.
 *
 * Manually detach device from driver.
 * When called for a USB interface, @dev->parent lock must be held.
 */
void device_release_driver(struct device *dev)
{
	/*
	 * If anyone calls device_release_driver() recursively from
	 * within their ->remove callback for the same device, they
	 * will deadlock right here.
	 */
	struct device_driver *drv;

	drv = dev->driver;
	if (drv) {
		if (drv->remove)
			drv->remove(dev);
		dev->driver = NULL;
	}
}

