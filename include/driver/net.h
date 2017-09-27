#ifndef __DRIVER_NET_H
#define __DRIVER_NET_H

/*
 * Maximum packet size; used to allocate packet storage. Use
 * the maxium Ethernet frame size as specified by the Ethernet
 * standard including the 802.1Q tag (VLAN tagging).
 * maximum packet size =  1522
 * maximum packet size and multiple of 32 bytes =  1536
 */
#define PKTSIZE				1522
#define PKTSIZE_ALIGN		1536

extern unsigned char net_pkt_buf[PKTSIZE_ALIGN];

struct net_device_stats {
	unsigned long	rx_packets;
	unsigned long	tx_packets;
	unsigned long	rx_bytes;
	unsigned long	tx_bytes;
	unsigned long	rx_errors;
	unsigned long	tx_errors;
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;
	unsigned long	rx_crc_errors;
	unsigned long	rx_frame_errors;
	unsigned long	rx_fifo_errors;
	unsigned long	rx_missed_errors;
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
};

struct net_device {
	struct	list_head ndev_list;
	struct	bus_node *bus;
	struct	net_device_stats stats;
	uint8_t	dev_addr[6];
#define NET_NAME_LEN	5
	char	name[NET_NAME_LEN];	// "eth(?)", "wl(?)"
	uint8_t	index;

	int (*netif_xmit)(struct net_device *ndev, void *packet, int len);
	int (*netif_rx)(struct net_device *ndev, void *packet, int len);
	int (*netif_poll)(struct net_device *ndev);
	void (*netif_halt)(struct net_device *ndev);
};


/**
 * is_zero_ethaddr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 */
static inline int is_zero_ethaddr(const uint8_t *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

/**
 * is_multicast_ethaddr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline int is_multicast_ethaddr(const uint8_t *addr)
{
	return 0x01 & addr[0];
}

/*
 * is_broadcast_ethaddr - Determine if the Ethernet address is broadcast
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is the broadcast address.
 */
static inline int is_broadcast_ethaddr(const uint8_t *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) ==
		0xff;
}

/*
 * is_valid_ethaddr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 */
static inline int is_valid_ethaddr(const uint8_t *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ethaddr(addr) && !is_zero_ethaddr(addr);
}


extern struct net_device *find_eth_by_index(uint8_t index);
extern struct net_device *find_eth_by_name(const char *name);
extern int eth_register(struct net_device *ndev);

extern struct list_head *net_dev_root(void);
extern void netif_bind_rx(struct net_device *ndev,
		    int (*netif_rx)(struct net_device *, void *, int));
extern int netif_rx(struct net_device *ndev, void *packet, int len);
extern int net_register(struct net_device *ndev);
extern int net_bus_init(void);

#endif
