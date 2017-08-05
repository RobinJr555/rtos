#ifndef __DRIVER_DEVICE_TABLE_H
#define __DRIVER_DEVICE_TABLE_H

/*
 * Struct used for matching a device
 */
struct of_device_id
{
	char	name[32];
	char	type[32];
	char	compatible[128];
	const void *data;
};


#define PLATFORM_NAME_SIZE	20

struct platform_device_id {
	char name[PLATFORM_NAME_SIZE];
	unsigned int driver_data;
};

#endif
