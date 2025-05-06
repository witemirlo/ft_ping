#ifndef FT_PING_H
# define FT_PING_H

# include <stdint.h>

# define GET_OCTET(a, b, c, d) (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))

extern const char *__progname;

typedef enum e_flags
{
	NO_FLAGS = 0x0,
	VERBOSE_OUTPUT = 0x1
} t_flags;

uint16_t sum_ones_complement(uint16_t a, uint16_t b);

t_flags get_flags(int argc, char* argv[]);

int get_socket(char const* const addr);

#endif
