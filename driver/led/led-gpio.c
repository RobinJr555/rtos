#include "driver/base.h"
#include "driver/gpio.h"

enum led_state {
	LED_OFF = 0,
	LED_ON
};

int led_on(struct device *dev, unsigned int led)
{
	return gpio_set(dev, led, LED_ON);
}

int led_off(struct device *dev, unsigned int led)
{
	return gpio_set(dev, led, LED_OFF);
}

int led_triger(struct device *dev, unsigned int led)
{
	unsigned int value = gpio_get(dev, led);
	return gpio_set(dev, led, !value);
}

int led_request(struct device *dev, unsigned int led)
{
	return gpio_request(dev, led);
}

void led_free(struct device *dev, unsigned int led)
{
	gpio_free(dev, led);
}
