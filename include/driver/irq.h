#ifndef __DEIVER_IRQ_H
#define __DRIVER_IRQ_H

typedef void (*irq_handler_t)(void);

extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
extern void request_irq(unsigned int irq, irq_handler_t handler, unsigned long flag);
extern void irq_init(void);

#endif
