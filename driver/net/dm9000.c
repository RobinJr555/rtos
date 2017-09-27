#include "common.h"
#include "driver/base.h"
#include "driver/irq.h"
#include "driver/net.h"
#include "driver/platform.h"
#include "driver/resource.h"
#include "driver/time.h"
#include "driver/dm9000.h"
#include "asm/io.h"
#include "asm/arch/base.h"
#include "dm9000.h"

/* DM9000 register address locking.
 *
 * The DM9000 uses an address register to control where data written
 * to the data register goes. This means that the address register
 * must be preserved over interrupts or similar calls.
 *
 * During interrupt and other critical calls, a spinlock is used to
 * protect the system, but the calls themselves save the address
 * in the address register in case they are interrupting another
 * access to the device.
 *
 * For general accesses a lock is provided so that calls which are
 * allowed to sleep are serialised so that the address register does
 * not need to be saved. This lock also serves to serialise access
 * to the EEPROM and PHY access registers which are shared between
 * these two devices.
 */

/* The driver supports the original DM9000E, and now the two newer
 * devices, DM9000A and DM9000B.
 */

enum dm9000_type {
	TYPE_DM9000E,	/* original DM9000 */
	TYPE_DM9000A,
	TYPE_DM9000B
};

/* Structure/enum declaration ------------------------------- */
struct board_info {
	void		*io_addr;	/* Register I/O base address */
	void		*io_data;	/* Data I/O address */
	uint8_t		irq;		/* IRQ */
	uint8_t		irq_flag;

	uint8_t		imr_all;

	enum dm9000_type type;

	uint16_t	tx_pkt_cnt;
	uint16_t	queue_pkt_len;
	void		*queue_pkt_data;

	unsigned int	flags;
	unsigned int	in_timeout:1;
	unsigned int	in_suspend:1;


	void (*inblk)(void *data, uint32_t length);
	void (*outblk)(void *data, uint32_t length);
	void (*dumpblk)(uint32_t length);
	void (*rx_status)(uint16_t *rx_status, uint16_t *rx_len);

	struct device		*dev;
	struct net_device	*ndev;
};
static struct board_info dm9000_info;
#define DM9000_ADDR		dm9000_info.io_addr
#define DM9000_DATA		dm9000_info.io_data

/* DM9000 network board routine ---------------------------- */

/*
 *   Read a byte from I/O port
 */
static uint8_t ior(int reg)
{
	writeb(reg, DM9000_ADDR);
	return readb(DM9000_DATA);
}

/*
 *   Write a byte to I/O port
 */
static void iow(int reg, int value)
{
	writeb(reg, DM9000_ADDR);
	writeb(value, DM9000_DATA);
}

static void dm9000_reset(struct board_info *db)
{
	dbg("%s resetting device\n", db->dev->name);

	/* Reset DM9000, see DM9000 Application Notes V1.22 Jun 11, 2004 page 29
	 * The essential point is that we have to do a double reset, and the
	 * instruction is to set LBK into MAC internal loopback mode.
	 */
	iow(DM9000_NCR, NCR_RST | NCR_MAC_LBK);
	udelay(100); /* Application note says at least 20 us */
	if (ior(DM9000_NCR) & 1)
		printf("dm9000 did not respond to first reset\n");

	iow(DM9000_NCR, 0);
	iow(DM9000_NCR, NCR_RST | NCR_MAC_LBK);
	udelay(100);
	if (ior(DM9000_NCR) & 1)
		printf("dm9000 did not respond to second reset\n");
}

/* routines for sending block to chip */
static void dm9000_outblk_8bit(void *data, uint32_t count)
{
	uint32_t i;
	for (i = 0; i < count; i++)
		writeb(((uint8_t *)data)[i] & 0xff, DM9000_DATA);
}

static void dm9000_outblk_16bit(void *data, uint32_t count)
{
	uint32_t i;
	uint32_t tmplen = (count + 1) >> 1;

	for (i = 0; i < tmplen; i++)
		writew(((uint16_t *)data)[i], DM9000_DATA);
}

static void dm9000_outblk_32bit(void *data, uint32_t count)
{
	uint32_t i;
	uint32_t tmplen = (count + 3) >> 2;

	for (i = 0; i < tmplen; i++)
		writew(((uint32_t *)data)[i], DM9000_DATA);
}

/* input block from chip to memory */
static void dm9000_inblk_8bit(void *data, uint32_t count)
{
	uint32_t i;
	for (i = 0; i < count; i++)
		((uint8_t *)data)[i] = readb(DM9000_DATA);
}


static void dm9000_inblk_16bit(void *data, uint32_t count)
{
	uint32_t i;
	uint32_t tmplen = (count + 1) >> 1;

	for (i = 0; i < tmplen; i++)
		((uint16_t *)data)[i] = readw(DM9000_DATA);
}

static void dm9000_inblk_32bit(void *data, uint32_t count)
{
	uint32_t i;
	uint32_t tmplen = (count + 3) >> 2;

	for (i = 0; i < tmplen; i++)
		((uint32_t *)data)[i] = readl(DM9000_DATA);
}

/* dump block from chip to null */
static void dm9000_dumpblk_8bit(uint32_t count)
{
	uint32_t i;

	for (i = 0; i < count; i++)
		readb(DM9000_DATA);
}

static void dm9000_dumpblk_16bit(uint32_t count)
{
	uint32_t i;

	count = (count + 1) >> 1;
	for (i = 0; i < count; i++)
		readw(DM9000_DATA);
}

static void dm9000_dumpblk_32bit(uint32_t count)
{
	uint32_t i;

	count = (count + 3) >> 2;
	for (i = 0; i < count; i++)
		readl(DM9000_DATA);
}

static void dm9000_rx_status_8bit(uint16_t *rx_status, uint16_t *rx_len)
{
	writeb(DM9000_MRCMD, DM9000_ADDR);

	*rx_status = le16_to_cpu(readb(DM9000_DATA) +
			  (readb(DM9000_DATA) << 8));
	*rx_len = le16_to_cpu(readb(DM9000_DATA) +
			  (readb(DM9000_DATA) << 8));
}

static void dm9000_rx_status_16bit(uint16_t *rx_status, uint16_t *rx_len)
{
	writeb(DM9000_MRCMD, DM9000_ADDR);

	*rx_status = le16_to_cpu(readw(DM9000_DATA));
	*rx_len = le16_to_cpu(readw(DM9000_DATA));
}

static void dm9000_rx_status_32bit(uint16_t *rx_status, uint16_t *rx_len)
{
	uint32_t tmpdata;

	writeb(DM9000_MRCMD, DM9000_ADDR);

	tmpdata = readl(DM9000_DATA);
	*rx_status = le16_to_cpu(tmpdata);
	*rx_len = le16_to_cpu(tmpdata >> 16);
}

/* read a word from phyxcer */
static uint16_t dm9000_phy_read(int reg)
{
	uint16_t val;
	uint32_t reg_save;

	/* Save previous register address */
	reg_save = readb(DM9000_ADDR);

	/* Fill the phyxcer register into REG_0C */
	iow(DM9000_EPAR, DM9000_PHY | reg);
	/* Issue phyxcer read command */
	iow(DM9000_EPCR, EPCR_ERPRR | EPCR_EPOS);

	writeb(reg_save, DM9000_ADDR);

	udelay(100);					/* Wait read complete */

	reg_save = readb(DM9000_ADDR);
	iow(DM9000_EPCR, 0x0);			/* Clear phyxcer read command */
	val = (ior(DM9000_EPDRH) << 8) | ior(DM9000_EPDRL);

	writeb(reg_save, DM9000_ADDR);

	/* The read data keeps on REG_0D & REG_0E */
	dbg("dm9000_phy_read(0x%x): 0x%x\n", reg, val);
	return val;
}

/* write a byte to I/O port */
static void dm9000_phy_write(int reg, uint16_t value)
{
	uint32_t reg_save;

	/* Save previous register address */
	reg_save = readb(DM9000_ADDR);

	/* Fill the phyxcer register into REG_0C */
	iow(DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	iow(DM9000_EPDRL, (value & 0xff));
	iow(DM9000_EPDRH, ((value >> 8) & 0xff));
	/* Issue phyxcer write command */
	iow(DM9000_EPCR, EPCR_EPOS | EPCR_ERPRW);

	writeb(reg_save, DM9000_ADDR);

	mdelay(1);					/* Wait write complete */

	reg_save = readb(DM9000_ADDR);

	iow(DM9000_EPCR, 0x0);			/* Clear phyxcer write command */

	writeb(reg_save, DM9000_ADDR);

	dbg("dm9000_phy_write(reg:0x%x, value:0x%x)\n", reg, value);
}

/* read a word data from eeprom */
static void dm9000_read_eeprom(int offset, uint8_t *to)
{
	struct board_info *db = &dm9000_info;
	if (db->flags & DM9000_PLATF_NO_EEPROM) {
		to[0] = 0xff;
		to[1] = 0xff;
		return;
	}

	iow(DM9000_EPAR, offset);
	iow(DM9000_EPCR, EPCR_ERPRR);
	mdelay(8);
	iow(DM9000_EPCR, 0x0);
	to[0] = ior(DM9000_EPDRL);
	to[1] = ior(DM9000_EPDRH);
}

/* write a word to eeprom */
static void dm9000_write_eeprom(int offset, uint16_t val)
{
	iow(DM9000_EPAR, offset);
	iow(DM9000_EPDRH, ((val >> 8) & 0xff));
	iow(DM9000_EPDRL, (val & 0xff));
	iow(DM9000_EPCR, EPCR_WEP | EPCR_ERPRW);
	mdelay(8);
	iow(DM9000_EPCR, 0);
}

/*
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
static int dm9000_start_xmit(struct net_device *ndev,
		    void *packet, int length)
{
	struct board_info *db = &dm9000_info;

	if (db->tx_pkt_cnt > 1)
		return -EBUSY;

	iow(DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */

	/* Move data to DM9000 TX RAM */
	writeb(DM9000_MWCMD, DM9000_ADDR); /* Prepare for TX-data */

	/* push the data to the TX-fifo */
	(db->outblk)(packet, length);
	ndev->stats.tx_bytes += length;

	db->tx_pkt_cnt++;
	if (db->tx_pkt_cnt == 1) {
		/* Set TX length to DM9000 */
		iow(DM9000_TXPLL, length & 0xff);
		iow(DM9000_TXPLH, (length >> 8) & 0xff);

		/* Issue TX polling command */
		iow(DM9000_TCR, TCR_TXREQ); /* Cleared after TX complete */
	} else {
		db->queue_pkt_data = packet;
		db->queue_pkt_len = length;
	}

	return 0;
}

/*
 * DM9000 interrupt handler
 * receive the packet to upper layer, free the transmitted packet
 */
static void dm9000_tx_done(void)
{
	struct board_info *db = &dm9000_info;
	struct net_device *ndev = db->ndev;
	int tx_status = ior(DM9000_NSR);	/* clear TX status */

	if (tx_status & (NSR_TX2END | NSR_TX1END)) {
		/* One packet sent complete */
		db->tx_pkt_cnt--;
		ndev->stats.tx_packets++;

		/* Queue packet check & send */
		if (db->tx_pkt_cnt > 0)
			dm9000_start_xmit(db->ndev, db->queue_pkt_data,
					   db->queue_pkt_len);
	}
}

/*
 *  received a packet and pass to upper layer
 */
static void dm9000_rx(void)
{
	uint8_t rxbyte;
	uint8_t *rdptr = (uint8_t *)&net_pkt_buf[0];
	uint16_t rx_status, rx_len = 0;
	bool GoodPacket;
	struct board_info *db = &dm9000_info;
	struct net_device *ndev = db->ndev;

	/* check packet ready or not */
	do {
		ior(DM9000_MRCMDX);	/* dummy read */

		/* get most updated data */
		rxbyte = readb(db->io_data);

		/* Status check: this byte must be 0 or 1 */
		if (rxbyte & DM9000_PKT_ERR) {
			dbg("dm9000 status check fail: %d\n", rxbyte);
			iow(DM9000_RCR, 0x00);	/* stop Device */
			return;
		}

		if (!(rxbyte & DM9000_PKT_RDY))
			return;

		/* a packet ready now  & Get status/length */
		(db->rx_status)(&rx_status, &rx_len);

		GoodPacket = true;

		dbg("dm9000 rx: status %02x, length %04x\n", rx_status, rx_len);

		/* packet Status check */
		if (rx_len < 0x40) {
			GoodPacket = false;
			printf("dm9000 rx: bad packet (runt)\n");
		}

		if (rx_len > DM9000_PKT_MAX) {
			printf("dm9000 RST: rx len:%x\n", rx_len);
		}

		/*rx_status is identical to RSR register. */
		rx_status >>= 8;
		if (rx_status & (RSR_FOE | RSR_CE | RSR_AE |
				      RSR_PLE | RSR_RWTO |
				      RSR_LCS | RSR_RF)) {
			GoodPacket = false;
			if (rx_status & RSR_FOE) {
				printf("dm9000 rx fifo error\n");
				ndev->stats.rx_fifo_errors++;
			}
			if (rx_status & RSR_CE) {
				printf("dm9000 rx crc error\n");
				ndev->stats.rx_crc_errors++;
			}
			if (rx_status & RSR_RF) {
				printf("dm9000 rx length error\n");
				ndev->stats.rx_length_errors++;
			}
		}

		/* move data from DM9000 */
		if (GoodPacket) {
			/* read received packet from RX SRAM */
			(db->inblk)(rdptr, rx_len);
			ndev->stats.rx_bytes += rx_len;

			/* pass to upper layer */
			netif_rx(ndev, rdptr, rx_len);
			ndev->stats.rx_packets++;
		} else {
			ndev->stats.rx_errors++;
			/* need to dump the packet's data */
			(db->dumpblk)(rx_len);
		}
	} while (rxbyte & DM9000_PKT_RDY);
}

static void dm9000_mask_interrupts(void)
{
	iow(DM9000_IMR, IMR_PAR);
}

static void dm9000_unmask_interrupts(void)
{
	struct board_info *db = &dm9000_info;
	iow(DM9000_IMR, db->imr_all);
}

static void dm9000_interrupt(void)
{
	int int_status;
	uint8_t reg_save;

	/* save previous register address */
	reg_save = readb(DM9000_ADDR);

	dm9000_mask_interrupts();

	/* get DM9000 interrupt status */
	int_status = ior(DM9000_ISR);
	iow(DM9000_ISR, int_status);	/* clear ISR status */

	/* received the coming packet */
	if (int_status & ISR_PRS)
		dm9000_rx();

	/* transmit interrupt check */
	if (int_status & ISR_PTS)
		dm9000_tx_done();

	dm9000_unmask_interrupts();

	/* restore previous register address */
	writeb(reg_save, DM9000_ADDR);
}

/*
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
static int dm9000_send(struct net_device *ndev, void *packet, int length)
{
	uint32_t tmo;
	struct board_info *db = &dm9000_info;

	iow(DM9000_ISR, IMR_PTM);			/* Clear Tx bit in ISR */

	/* move data to DM9000 TX RAM */
	writeb(DM9000_MWCMD, DM9000_ADDR);	/* prepare for TX-data */

	/* push the data to the TX-fifo */
	(db->outblk)(packet, length);

	/* set TX length to DM9000 */
	iow(DM9000_TXPLL, length & 0xff);
	iow(DM9000_TXPLH, (length >> 8) & 0xff);

	ndev->stats.tx_packets++;

	/* issue TX polling command */
	iow(DM9000_TCR, TCR_TXREQ);			/* Cleared after TX complete */

	/* wait for end of transmission */
	tmo = get_timer() + 5 * CONFIG_SYS_HZ;
	while ( !(ior(DM9000_NSR) & (NSR_TX1END | NSR_TX2END)) ||
		!(ior(DM9000_ISR) & IMR_PTM) ) {
		if (get_timer() >= tmo) {
			ndev->stats.tx_packets--;
			ndev->stats.tx_errors++;
			printf("transmission timeout\n");
			break;
		}
	}
	iow(DM9000_ISR, IMR_PTM);			/* Clear Tx bit in ISR */

	return 0;
}

static int dm9000_poll(struct net_device *ndev)
{
	/* Check packet ready or not, we must check
	   the ISR status first for DM9000A */
	if (!(ior(DM9000_ISR) & ISR_PRS)) /* Rx-ISR bit must be set. */
		return -EAGAIN;

	iow(DM9000_ISR, ISR_PRS); /* clear PR status latched in bit 0 */
	dm9000_rx();

	return 0;
}
/*
  Stop the interface.
  The interface is stopped when it is brought.
*/
static void dm9000_halt(struct net_device *ndev)
{
	/* RESET devie */
	dm9000_phy_write(0, 0x8000);	/* PHY RESET */
	iow(DM9000_GPR, 0x01);			/* Power-Down PHY */
	iow(DM9000_IMR, 0x80);			/* Disable all interrupt */
	iow(DM9000_RCR, 0x00);			/* Disable RX */

}

static unsigned char dm9000_type_to_char(enum dm9000_type type)
{
	switch (type) {
	case TYPE_DM9000E: return 'e';
	case TYPE_DM9000A: return 'a';
	case TYPE_DM9000B: return 'b';
	}

	return '?';
}

static int dm9000_probe(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct resource *addr_res, *data_res, *irq_res;
	struct board_info *db = &dm9000_info;
	struct net_device *ndev;
	uint32_t i, oft, lnk;
	uint8_t io_mode;
	uint32_t id_val;
	int ret = 0;

	ndev = malloc(sizeof(struct net_device));
	if (!ndev)
		return -ENOMEM;

	db->dev = dev;
	db->ndev = ndev;

	/* get io_addr and io_data from platform resource */
	addr_res = platform_get_resource(pdev, RESOURCE_MEM, 0);
	data_res = platform_get_resource(pdev, RESOURCE_MEM, 1);
	if (addr_res == NULL || data_res == NULL) {
		printf("dm9000 insufficient resources\n");
		ret = -ENOENT;
		goto out;
	}

	db->io_addr = (void *)addr_res->start;
	db->io_data = (void *)data_res->start;

	irq_res = platform_get_resource(pdev, RESOURCE_IRQ, 0);
	if (irq_res != NULL) {
		db->irq = irq_res->start;
		db->irq_flag = irq_res->flags & IRQ_FLAG_MASK;
		request_irq(db->irq, dm9000_interrupt, db->irq_flag);
		ndev->netif_xmit = dm9000_start_xmit;
	} else {
		ndev->netif_xmit = dm9000_send;
		ndev->netif_poll = dm9000_poll;
	}

	ndev->netif_halt = dm9000_halt;

	if (eth_register(ndev)) {
		ret = -EFAULT;
		goto out;
	}

	printf("dm9000%c at %p,%p IRQ %d MAC: %pM\n",
		       dm9000_type_to_char(db->type),
		       db->io_addr, db->io_data, db->irq,
		       ndev->dev_addr);

	dm9000_reset(db);
	dm9000_mask_interrupts();

	/* auto-detect 8/16/32 bit mode, ISR Bit 6+7 indicate bus width */
	io_mode = ior(DM9000_ISR) >> 6;
	switch (io_mode) {
	case 0x0:  /* 16-bit mode */
		printf("dm9000: running in 16 bit mode\n");
		db->inblk     = dm9000_inblk_16bit;
		db->outblk    = dm9000_outblk_16bit;
		db->dumpblk   = dm9000_dumpblk_16bit;
		db->rx_status = dm9000_rx_status_16bit;
		break;
	case 0x01:  /* 32-bit mode */
		printf("dm9000: running in 32 bit mode\n");
		db->inblk     = dm9000_inblk_32bit;
		db->outblk    = dm9000_outblk_32bit;
		db->dumpblk   = dm9000_dumpblk_32bit;
		db->rx_status = dm9000_rx_status_32bit;
		break;
	case 0x02: /* 8 bit mode */
		printf("dm9000: running in 8 bit mode\n");
		db->inblk     = dm9000_inblk_8bit;
		db->outblk    = dm9000_outblk_8bit;
		db->dumpblk   = dm9000_dumpblk_8bit;
		db->rx_status = dm9000_rx_status_8bit;
		break;
	default:
		/* Assume 8 bit mode, will probably not work anyway */
		printf("dm9000: Undefined IO-mode:0x%x\n", io_mode);
		db->inblk     = dm9000_inblk_8bit;
		db->outblk    = dm9000_outblk_8bit;
		db->dumpblk   = dm9000_dumpblk_8bit;
		db->rx_status = dm9000_rx_status_8bit;
		break;
	};

	/* try multiple times, DM9000 sometimes gets the read wrong */
	for (i = 0; i < 8; i++) {
		id_val  = ior(DM9000_VIDL);
		id_val |= (u32)ior(DM9000_VIDH) << 8;
		id_val |= (u32)ior(DM9000_PIDL) << 16;
		id_val |= (u32)ior(DM9000_PIDH) << 24;

		if (id_val == DM9000_ID)
			break;
		printf("dm9000 read wrong id 0x%08x\n", id_val);
	}

	if (id_val != DM9000_ID) {
		ret = -ENODEV;
		goto out;
	}

	/* identify what type of DM9000 we are working on */
	id_val = ior(DM9000_CHIPR);
	dbg("dm9000 revision 0x%02x\n", id_val);

	switch (id_val) {
	case CHIPR_DM9000A:
		db->type = TYPE_DM9000A;
		break;
	case CHIPR_DM9000B:
		db->type = TYPE_DM9000B;
		break;
	default:
		dbg("ID %02x => defaulting to DM9000E\n", id_val);
		db->type = TYPE_DM9000E;
	}

	/* try reading the node address from the attached EEPROM */
	for (i = 0; i < 6; i += 2)
		dm9000_read_eeprom(i / 2, ndev->dev_addr+i);

	dbg("MAC: %pM\n", ndev->dev_addr);
	if (!is_valid_ethaddr(ndev->dev_addr))
		printf("WARNING: Bad MAC address (uninitialized EEPROM?)\n");

	/* fill device MAC address registers */
	for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
		iow(oft, ndev->dev_addr[i]);
	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		iow(oft, 0xff);

	/* read back mac, just to be sure */
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		dbg("%02x:", ior(oft));
	dbg("\n");

	iow(DM9000_GPCR, GPCR_GEP_CNTL);	/* let GPIO0 output */
	iow(DM9000_GPR, 0);					/* power interral PHY */

	/* program operating register, only internal phy supported */
	iow(DM9000_NCR, 0x0);
	/* TX Polling clear */
	iow(DM9000_TCR, 0);
	/* less 3Kb, 200us */
	iow(DM9000_BPTR, BPTR_BPHW(3) | BPTR_JPT_600US);
	/* flow Control : High/Low Water */
	iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));
	/* SH FIXME: This looks strange! Flow Control */
	iow(DM9000_FCR, 0x0);
	/* special Mode */
	iow(DM9000_SMCR, 0);
	/* clear TX status */
	iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	/* clear interrupt status */
	iow(DM9000_ISR, ISR_ROOS | ISR_ROS | ISR_PTS | ISR_PRS);

	/* activate DM9000 */
	/* RX enable */
	iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* enable TX/RX interrupt mask */
	iow(DM9000_IMR, IMR_PAR);

	db->imr_all = IMR_PAR | IMR_PTM | IMR_PRM;
	if (db->type != TYPE_DM9000E)
		db->imr_all |= IMR_LNKCHNG;

	i = 0;
	while (!(dm9000_phy_read(1) & 0x20)) {	/* autonegation complete bit */
		mdelay(1);
		i++;
		if (i == 10000) {
			printf("could not establish link\n");
			return 0;
		}
	}

	/* see what we've got */
	lnk = dm9000_phy_read(17) >> 12;
	printf("operating at ");
	switch (lnk) {
	case 1:
		printf("10M half duplex ");
		break;
	case 2:
		printf("10M full duplex ");
		break;
	case 4:
		printf("100M half duplex ");
		break;
	case 8:
		printf("100M full duplex ");
		break;
	default:
		printf("unknown: %d ", lnk);
		break;
	}
	printf("mode\n");

	dm9000_unmask_interrupts();
	return 0;

out:
	printf("dm9000 not found (%d).\n", ret);
	free(ndev);

	return ret;
}


static struct platform_driver dm9000_driver = {
	.driver = {
		.name = "dm9000",
		.probe = dm9000_probe,
	},
};

void dm9000_init(void)
{
	platform_driver_register(&dm9000_driver);
}

module_init(dm9000_init);
