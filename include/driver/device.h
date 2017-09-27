#ifndef __DRIVER_DEVICE_H
#define __DRIVER_DEVICE_H

#include "common/list.h"

struct device;
struct device_driver;
struct bus_node;

enum node_type {
	NODE_NONE = 0,
	NODE_PLATFORM,
	NODE_GPIO,
	NODE_PINCTRL,
	NODE_UART,
	NODE_SPI,
	NODE_CAN,
	NODE_NET,
};

struct bus_node {
	enum node_type node_type;
	struct device *dev;
	struct list_head node_list;
	int (*match)(struct device *dev, struct device_driver *drv);
};

extern int bus_register(struct bus_node *bus_node);
int bus_for_each_dev(struct bus_node *bus, struct device *start, void *data,
		     int (*fn)(struct device *dev, void *data));
int bus_for_each_drv(struct bus_node *bus, struct device_driver *start,
		     void *data, int (*fn)(struct device_driver *, void *));
int bus_find_driver(const char *name, struct bus_node *bus);


struct device {
	const char *name;
	struct device *parent;
	struct bus_node *bus;
	struct device_driver *driver;
	struct list_head driver_list;
	struct list_head device_list;
	void *private_data;
};

struct device_driver {
	const char *name;
	struct bus_node *bus;
	const struct of_device_id *of_match_table;
	struct list_head device_list;
	struct list_head driver_list;
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
};

extern int driver_register(struct device_driver *drv);
extern void driver_unregister(struct device_driver *drv);

extern int device_register(struct device *dev);
extern void device_unregister(struct device *dev);
extern void device_initialize(struct device *dev);
extern int device_add(struct device *dev);

extern int driver_probe_device(struct device_driver *drv, struct device *dev);
extern void device_release_driver(struct device *dev);

void board_init(void);


#endif
