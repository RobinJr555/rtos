#include <stddef.h>
#include <sys/stat.h>
#include "common.h"
#include "driver.h"

extern __driver_init __driver_start;
extern __driver_init __driver_end;

static void driver_init(void)
{
	__driver_init *func;
	for (func = &__driver_start; func < &__driver_end; func++) {
		(*func)();
	}
}

static void bus_init(void)
{
	platform_bus_init();
#if CONFIG_PINCTRL
	pinctrl_bus_init();
#endif
}

__attribute((weak)) void hardware_init_hook(void)
{
#if CONFIG_EARLY_PRINTF
	early_console_init();
#endif
	bus_init();
	driver_init();
	device_init();
}

__attribute((weak)) void _exit(int return_code)
{
	while(1);
}

/**
 * Newlib cstub porting
 */
#if defined(__arm__)
#define CSTUB_READ       _read
#define CSTUB_WRITE      _write
#define CSTUB_OPEN       _open
#define CSTUB_CLOSE      _close
#define CSTUB_SBRK       _sbrk
#define CSTUB_ISATTY     _isatty
#define CSTUB_LSEEK      _lseek
#define CSTUB_FSTAT      _fstat
#define CSTUB_GETPID     _getpid
#define CSTUB_KILL       _kill
#else
#define CSTUB_READ       read
#define CSTUB_WRITE      write
#define CSTUB_OPEN       open
#define CSTUB_CLOSE      close
#define CSTUB_SBRK       sbrk
#define CSTUB_ISATTY     isatty
#define CSTUB_LSEEK      lseek
#define CSTUB_FSTAT      fstat
#define CSTUB_GETPID     getpid
#define CSTUB_KILL       kill
#endif

extern int errno;
extern uint32_t __HeapBase;
extern uint32_t __HeapLimit;
__attribute((weak)) caddr_t CSTUB_SBRK(int incr)
{
	static unsigned char* heap = (unsigned char*)&__HeapBase;
	unsigned char*        prev_heap = heap;
	unsigned char*        new_heap = heap + incr;

	if (new_heap >= (unsigned char*)&__HeapLimit) {     /* __HeapLimit is end of heap section */
		errno = ENOMEM;
		return (caddr_t)-1;
	}

	heap = new_heap;
	return (caddr_t) prev_heap;
}

inbyte std_inbyte = NULL;
__attribute((weak)) int CSTUB_READ(int file, char *ptr, unsigned int nbyte)
{
	unsigned int i;
	if (!std_inbyte) return 0;

	for (i = 0; i < nbyte; i++) {
		ptr[i] = (*std_inbyte)();
		if ((ptr[i] == '\n') || (ptr[i] == '\r')) {
			ptr[i] = 0;
			break;
		}
	}
	return (i);
}

outbyte std_outbyte = NULL;
__attribute((weak)) int CSTUB_WRITE(int file, char *ptr, unsigned int nbyte)
{
	if (!std_outbyte) return 0;

	for (unsigned int i = 0; i < nbyte; i++) {
		(*std_outbyte)(ptr[i]);
	}

	return (nbyte);
}

__attribute((weak)) int CSTUB_ISATTY(int file)
{
	return (1);
}

__attribute((weak)) int CSTUB_FSTAT(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

__attribute((weak)) int CSTUB_GETPID(void)
{
	return (1);
}

__attribute((weak)) int CSTUB_KILL(int pid, int sig)
{
	errno = EINVAL;
	return (-1);
}

__attribute((weak)) int CSTUB_OPEN(const char *name, int openmode)
{
	errno = EIO;
	return (-1);
}

__attribute((weak)) int CSTUB_CLOSE(int file)
{
	return (0);
}

__attribute((weak)) int CSTUB_LSEEK(int file, off_t ptr, int dir)
{
	errno = ESPIPE;
	return ((off_t)-1);
}
