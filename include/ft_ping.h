#ifndef FT_PING_H
# define FT_PING_H

# include <stdint.h>

# define GET_OCTET(a, b, c, d) (((a & 0xf) << 24) | ((b & 0xf) << 16) | ((c & 0xf) << 8) | (d & 0xf))

typedef enum e_flags
{
	NO_FLAGS = 0x0,
	VERBOSE_OUTPUT = 0x1
} t_flags;

uint16_t sum_ones_complement(uint16_t a, uint16_t b);

t_flags get_flags(int argc, char* argv[]);

#endif
