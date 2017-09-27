#include "common.h"
#include "driver/base.h"
#include "driver/net.h"

static uint8_t eth_num = 0;

struct net_device *find_eth_by_index(uint8_t index)
{
	struct net_device *ndev;
	struct list_head *root = net_dev_root();

	list_for_each_entry(ndev, root, ndev_list) {
		if(ndev->index == index && !strncmp(ndev->name, "eth", 3))
			return ndev;
	}

	return NULL;
}

struct net_device *find_eth_by_name(const char *name)
{
	struct net_device *ndev;
	struct list_head *root = net_dev_root();

	list_for_each_entry(ndev, root, ndev_list) {
		if(!strcmp(ndev->name, name))
			return ndev;
	}

	return NULL;
}

int eth_register(struct net_device *ndev)
{
	ndev->index = eth_num++;

	/* set ndev name: eth(?) */
	ndev->name[0] = 'e';
	ndev->name[1] = 't';
	ndev->name[2] = 'h';
	ndev->name[3] = '0' + ndev->index;
	ndev->name[4] = '\0';

	return net_register(ndev);
}

