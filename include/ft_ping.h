#ifndef FT_PING_H
# define FT_PING_H

# include <stdint.h>

# define GET_OCTET(a, b, c, d) (((a & 0xf) << 24) | ((b & 0xf) << 16) | ((c & 0xf) << 8) | (d & 0xf))

extern const char *__progname;

uint16_t sum_ones_complement(uint16_t, uint16_t);

#endif
