#ifndef __DRIVER_GPIO_H_
#define __DRIVER_GPIO_H_

struct device;

/**
 * struct gpio_ops - Driver model GPIO operations
 *
 * Also it would be useful to standardise additional functions like
 * pullup, slew rate and drive strength.
 *
 * gpio_request() and gpio_free() are optional - if NULL then they will
 * not be called.
 *
 */
struct gpio_ops {
	int (*direction_input) (struct device *dev,
			        unsigned int gpio);
	int (*direction_output) (struct device *dev,
			        unsigned int gpio, int value);
	int (*gpio_get_value) (struct device *dev,
			        unsigned int gpio);
	int (*gpio_set_value) (struct device *dev,
			        unsigned int gpio,
			        int value);
	int (*gpio_request) (struct device *dev,
			        unsigned int gpio);
	void (*gpio_free) (struct device *dev,
			        unsigned int gpio);

	void (*gpio_irq_set) (struct device *dev,
			        unsigned int gpio,
			        unsigned int flag);
	void (*gpio_irq_enable) (struct device *dev,
			        unsigned int gpio,
			        unsigned int enable);
	void (*gpio_irq_pend_clear) (struct device *dev,
			        unsigned int gpio);
};

void gpio_register(struct gpio_ops *ops);
int gpio_bus_init(void);

/* user call */
int gpio_set_input(struct device *dev, unsigned int gpio);
int gpio_set_output(struct device *dev, unsigned int gpio, int value);
int gpio_get(struct device *dev, unsigned int gpio);
int gpio_set(struct device *dev, unsigned int gpio, int value);
int gpio_request(struct device *dev, unsigned int gpio);
void gpio_free(struct device *dev, unsigned int gpio);

#endif	/* __DRIVER_GPIO_H_ */
