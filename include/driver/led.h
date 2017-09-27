#ifndef __DRIVER_LED_H
#define __DRIVER_LED_H


int led_on(struct device *dev, unsigned int gpio);
int led_off(struct device *dev, unsigned int gpio);
int led_triger(struct device *dev, unsigned int gpio);

int led_request(struct device *dev, unsigned int led);
void led_free(struct device *dev, unsigned int led);


#endif /* __DEVICE_LED_H */
