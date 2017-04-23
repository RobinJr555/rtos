#include <stddef.h>
#include <driver/clock.h>
#include <driver/gpio.h>

typedef int (*init_func)(void);

static init_func init_sequence[] = {
	clock_init,
	gpio_init,
#if CONFIG_DRIVER_SPI
	spi_init,
#endif
	NULL,
};


void board_init_r(void)
{
	const init_func *init_func_ptr;
	int ret;

	for(init_func_ptr = init_sequence; *init_func_ptr; ++init_func_ptr) {
		ret = (*init_func_ptr)();
		if (ret) {
			debug("initcall sequence %p failed (err=%d)\n",
				init_func_ptr, ret);
			return;
		}
	}
}
