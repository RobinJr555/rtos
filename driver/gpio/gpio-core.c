#include "driver/base.h"
#include "driver/device.h"
#include "driver/gpio.h"
#include "common.h"

struct device gpio_bus = {
	.name = "gpio",
};

struct bus_node gpio_bus_node = {
	.node_type = NODE_GPIO,
	.dev = &gpio_bus,
};


int gpio_set_input(struct device *dev, unsigned int gpio)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->direction_input)
		return ops->direction_input(dev, gpio);
	return -EINVAL;
}

int gpio_set_output(struct device *dev, unsigned int gpio, int value)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->direction_output)
		return ops->direction_output(dev, gpio, value);
	return -EINVAL;
}

int gpio_get(struct device *dev, unsigned int gpio)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_get_value)
		return ops->gpio_get_value(dev, gpio);
	return -EINVAL;
}

int gpio_set(struct device *dev, unsigned int gpio, int value)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_set_value)
		return ops->gpio_set_value(dev, gpio, value);
	return -EINVAL;
}

int gpio_request(struct device *dev, unsigned int gpio)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_request)
		return ops->gpio_request(dev, gpio);
	return -EINVAL;
}

void gpio_free(struct device *dev, unsigned int gpio)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_free)
		ops->gpio_free(dev, gpio);
}

void gpio_irq_set(struct device *dev, unsigned int gpio, unsigned int flag)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_irq_set)
		ops->gpio_irq_set(dev, gpio, flag);
}

void gpio_irq_enable(struct device *dev, unsigned int gpio, unsigned int enable)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_irq_enable)
		ops->gpio_irq_enable(dev, gpio, enable);
}

void gpio_irq_pend_clear(struct device *dev, unsigned int gpio)
{
	struct gpio_ops *ops = (struct gpio_ops *)gpio_bus.private_data;
	if (ops && ops->gpio_irq_pend_clear)
		ops->gpio_irq_pend_clear(dev, gpio);
}

void gpio_register(struct gpio_ops *ops)
{
	if (ops)
		gpio_bus.private_data = (void *)ops;
}


int gpio_bus_init(void)
{
	int error;

	error = device_register(&gpio_bus);
	if (error)
		return error;
	error = bus_register(&gpio_bus_node);
	if(error)
		device_unregister(&gpio_bus);
	return error;
}
