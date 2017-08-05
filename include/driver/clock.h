#ifndef __DRIVER_CLOCK_H
#define __DRIVER_CLODK_H

struct device;


/* user call */
extern unsigned long clk_get(unsigned int clck);
extern void clk_setup_periph(unsigned int periph_addr);
extern void clk_setup_dev(struct device *dev);
extern int clk_update(unsigned int clk_freq);

#endif /* __DRIVER_CLOCK_H */
