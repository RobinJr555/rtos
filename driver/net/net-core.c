#include "common.h"
#include "driver/base.h"
#include "driver/device.h"
#include "driver/net.h"

/* Receive buffer */
unsigned char net_pkt_buf[PKTSIZE_ALIGN];
static LIST_HEAD(ndev_list_root);

struct device net_bus = {
	.name = "net",
};

struct bus_node net_bus_node = {
	.node_type = NODE_NET,
	.dev = &net_bus,
};

struct list_head *net_dev_root(void)
{
	return &ndev_list_root;
}

void netif_bind_rx(struct net_device *ndev,
		    int (*netif_rx)(struct net_device *, void *, int))
{
	ndev->netif_rx = netif_rx;
}

int netif_rx(struct net_device *ndev, void *packet, int len)
{
	if (ndev->netif_rx)
		return ndev->netif_rx(ndev, packet, len);
	return -EINVAL;
}

int netif_poll(struct net_device *ndev)
{
	if (ndev->netif_poll)
		return ndev->netif_poll(ndev);
	return -EINVAL;
}

int net_register(struct net_device *ndev)
{
	ndev->bus = &net_bus_node;
	list_add_tail(&ndev->ndev_list, &ndev_list_root);

	return 0;
}

int net_bus_init(void)
{
	int error;

	error = device_register(&net_bus);
	if (error)
		return error;
	error = bus_register(&net_bus_node);
	if(error)
		device_unregister(&net_bus);

	net_bus.private_data = &ndev_list_root;

	return error;
}
