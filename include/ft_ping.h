#ifndef FT_PING_H
# define FT_PING_H

# include <netdb.h>
# include <netinet/in.h>
# include <stdbool.h>
# include <stdint.h>
#include <sys/socket.h>


// GLOBALS ---------------------------------------------------------------------
extern const char *__progname;
extern bool is_running;


// TYPES -----------------------------------------------------------------------
typedef enum e_flags {
	NO_FLAGS = 0x0,
	VERBOSE_OUTPUT = 0x1
} t_flags;

typedef struct s_connection_data {
	struct sockaddr_in addr;
	socklen_t          addr_len;
	int                sockfd;
	char*              canonname;
	char*              ip_char;
} t_connection_data;


// FUNCTION PROTOTIPES ---------------------------------------------------------
t_connection_data get_connection_data(char const* const addr);

void destroy_connection_data(t_connection_data* data);

t_flags get_flags(int argc, char* argv[]);

uint16_t sum_ones_complement(uint16_t a, uint16_t b);

void print_verbose_header(t_flags flags);

void signal_int(int sig);
void signal_quit(int sig);

#endif
