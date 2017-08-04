#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "common.h"


void rtos_putchar(uint8_t c)
{
	if (!std_outbyte) return;
	(*std_outbyte)(c);
};

static void rtos_puts_x(char *string, int width, const char pad)
{
	while (*string) {
		rtos_putchar(*string);
		string ++;
		width --;
	}

	while (width > 0) {
		rtos_putchar(pad);
		width --;
	}
}

static void rtos_put_decimal(const int32_t signed_value, const int width,
		    const char initial_pad)
{
	uint32_t divisor;
	int digits;
	unsigned char is_negative;
	int32_t value;
	char pad;

	is_negative = signed_value < 0;
	value = is_negative ? -signed_value : signed_value;

	/* estimate number of spaces and digits */
	for (divisor = 1, digits = 1;
		    value / divisor >= 10;
		    divisor *= 10, digits++);

	if (is_negative)
	    digits ++;

	pad = is_negative ? ' ' : initial_pad;

	/* print spaces */
	for (; digits < width; digits++)
		rtos_putchar(pad);

	if (is_negative)
		rtos_putchar('-');

	/* print digits */
	do {
		rtos_putchar(((value/divisor) % 10) + '0');
	} while (divisor /= 10);
}

#define hexchars(x)   \
		(((x) < 10) ? \
		('0' + (x)) : \
		('a' + ((x) - 10)))

static int rtos_put_hex(const uint32_t value, int width, const char pad)
{
	int i;
	int n = 0;
	int v_width;

	uint32_t v;
	char digit;

	/* Find the width of value in hex digits */
	v_width = 1; // At least one digit
	while ( value >> (4 * v_width) && ((unsigned) v_width < 2 * sizeof(value)))
		v_width ++;

	/* Value wider than specifier? */
	if (width == 0 && width < v_width)
		width = v_width;

	/* Paddings */
	for (i = width - v_width; i > 0; i--, n++)
		rtos_putchar(pad);

	/* Characters */
	for (i = 4 * (v_width - 1); i >= 0; i -= 4, n++) {
		v = (value >> i) & 0xF;
		if (v<10)
			digit = '0' + v;
		else
			digit = 'a' + (v-10);
		rtos_putchar(digit);
	}

	return n;
}

void rtos_vprintf(char* format, va_list va)
{
	int mode = 0;       // Mode = 1: in specifiers
	int width = 0;      // Width of output
	char pad = ' ';
	int size = 16;      // Target variable size
	char character;

	while (*format) {
		character = *format;

		if (character == '%' && mode == 0) {
			mode = 1;
			pad = ' ';
			width = 0;
			size = 32;
			format++;
			continue;
		}

		if (!mode) {
			rtos_putchar(character);
		} else {
			switch (character) {
				case 'c':
					rtos_putchar(va_arg(va, uint32_t));
					mode = 0;
					break;
				case 's':
					rtos_puts_x(va_arg(va, char *), width, pad);
					mode = 0;
					break;
				case 'l':
				case 'L':
					size = 64;
					break;
				case 'd':
				case 'D':
					rtos_put_decimal((size==32)?
							    va_arg(va, int32_t):
							    va_arg(va, int64_t), width, pad);
					mode = 0;
					break;
				case 'p':
				case 't':
					size = 32;
					width = 8;
					pad = '0';
				case 'x':
				case 'X':
					rtos_put_hex((size==32)?
							    va_arg(va, uint32_t):
							    va_arg(va, uint64_t), width, pad);
					mode = 0;
					break;
				case '%':
					rtos_putchar('%');
					mode = 0;
					break;
				case '0':
					if (!width)
						pad = '0';
					break;
				/* Do not support -, just eat the character */
				case '-':
				    break;
				case ' ':
					pad = ' ';
			} // switch

			if (character >= '0' && character <= '9')
				width = width * 10 + (character - '0');
		} // in-mode
		format++;
	} // while format
}

void rtos_printf(char* format, ...)
{
	va_list va;
	va_start(va, format);
	rtos_vprintf(format, va);
	va_end(va);
}
