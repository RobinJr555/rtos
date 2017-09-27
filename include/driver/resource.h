#ifndef __DRIVER_RESOURCE_H
#define __DRIVER_RESOURCE_H

struct resource {
	uint32_t start;
	uint32_t end;
	uint32_t flags;
};

/*
 * | reserved | reserved | 15 - 8 | 7 - 0 |
 * 15 - 08: resource types
 * 07 - 00: resource offsets or attribute
 */
#define RESOURCE_IO				(0x00000100)
#define RESOURCE_MEM			(0x00000200)
#define RESOURCE_REG			(0x00000400)
#define RESOURCE_IRQ			(0x00000800)
#define RESOURCE_DMA			(0x00001000)
#define RESOURCE_BUS			(0x00002000)

// RESOURCE_IO

// RESOURCE_MEM

// RESOURCE_REG

// RESOURCE_IRQ
#define IRQ_FLAG_RISING			(0x00000000)
#define IRQ_FLAG_FAILING		(0x00000001)
#define IRQ_FLAG_MASK			(0x000000ff)

// RESOURCE_DMA


#endif
