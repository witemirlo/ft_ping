#ifndef FT_PING_H
# define FT_PING_H

# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>
# include <sys/socket.h>


// GLOBALS ---------------------------------------------------------------------
extern const char *__progname;
extern bool is_running;


// TYPES -----------------------------------------------------------------------
typedef enum e_flags {
	NO_FLAGS = 0x0,
	VERBOSE_OUTPUT = 0x1
} t_flags;

typedef struct s_complete_packet {
	struct ip ip;
	struct icmp icmp;
	char   payload[BUFSIZ - sizeof(struct ip) - sizeof(struct icmp)];

} t_complete_packet;

typedef struct s_connection_data {
	struct sockaddr_in addr;
	socklen_t          addr_len;
	int                sockfd;
	char*              canonname;
	char*              ip_char;
} t_connection_data;

// TODO: ELEGIR MEJORES NOMBRES PARA AMBAS ESTRUCTURAS
typedef struct s_time_info {
	double min_time;
	double avg_time;
	double max_time;
	double time;
} t_time_info;

typedef struct s_time_stats {
	double min_time;
	double avg_time;
	double max_time;
	size_t packets_received;
} t_time_stats;


// FUNCTION PROTOTIPES ---------------------------------------------------------
// -- CONNECTION DATA ----------------------------------------------------------
t_connection_data get_connection_data(char const* const addr);
void destroy_connection_data(t_connection_data* data);

// -- PARSING ------------------------------------------------------------------
t_flags get_flags(int argc, char* argv[]);

// -- FLAGS --------------------------------------------------------------------
void print_header(t_flags flag, t_connection_data* datas);

// -- SIGNALS ------------------------------------------------------------------
void signal_int(int sig);
void signal_quit(int sig);

// -- ICMP PAQUETS -------------------------------------------------------------
void init_icmp(struct icmp* const icmp);
void update_icmp(struct icmp* const icmp);
void update_icmp_checksum(struct icmp* const icmp);

#endif
