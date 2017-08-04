#ifndef __RTOS_BYTEORDER_H
#define __RTOS_BYTEORDER_H

#include <stdint.h>

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

#define swap_16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define swap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#define _swap_64(x, sfx) \
	((((x) & 0xff00000000000000##sfx) >> 56) | \
	 (((x) & 0x00ff000000000000##sfx) >> 40) | \
	 (((x) & 0x0000ff0000000000##sfx) >> 24) | \
	 (((x) & 0x000000ff00000000##sfx) >>  8) | \
	 (((x) & 0x00000000ff000000##sfx) <<  8) | \
	 (((x) & 0x0000000000ff0000##sfx) << 24) | \
	 (((x) & 0x000000000000ff00##sfx) << 40) | \
	 (((x) & 0x00000000000000ff##sfx) << 56))
#if defined(__GNUC__)
# define swap_64(x) _swap_64(x, ull)
#else
# define swap_64(x) _swap_64(x, )
#endif

#if CONFIG_CPU_LITTLE_ENDIAN
# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)
# define cpu_to_be16(x)		swap_16(x)
# define cpu_to_be32(x)		swap_32(x)
# define cpu_to_be64(x)		swap_64(x)
# define be16_to_cpu(x)		swap_16(x)
# define be32_to_cpu(x)		swap_32(x)
# define be64_to_cpu(x)		swap_64(x)
#else
# define cpu_to_le16(x)		swap_16(x)
# define cpu_to_le32(x)		swap_32(x)
# define cpu_to_le64(x)		swap_64(x)
# define le16_to_cpu(x)		swap_16(x)
# define le32_to_cpu(x)		swap_32(x)
# define le64_to_cpu(x)		swap_64(x)
# define cpu_to_be16(x)		(x)
# define cpu_to_be32(x)		(x)
# define cpu_to_be64(x)		(x)
# define be16_to_cpu(x)		(x)
# define be32_to_cpu(x)		(x)
# define be64_to_cpu(x)		(x)
#endif


#endif /* __RTOS_BYTEORDER_H */
