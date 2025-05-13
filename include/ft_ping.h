#ifndef FT_PING_H
# define FT_PING_H

# include <stdint.h>
# include <netdb.h>
# include <netinet/in.h>

# define GET_OCTET(a, b, c, d) (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))

extern const char *__progname;

typedef enum e_flags {
	NO_FLAGS = 0x0,
	VERBOSE_OUTPUT = 0x1
} t_flags;

typedef struct s_connection_data {
	struct sockaddr_in addr;
	int                sockfd;
	char*              canonname;
	char*              ip_char;
} t_connection_data;

uint16_t sum_ones_complement(uint16_t a, uint16_t b);

t_flags get_flags(int argc, char* argv[]);

t_connection_data get_connection_data(char const* const addr);

void destroy_connection_data(t_connection_data* data);

#endif
