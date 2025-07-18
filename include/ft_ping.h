#ifndef FT_PING_H
# define FT_PING_H

# include <arpa/inet.h>
# include <ctype.h>
# include <errno.h>
# include <float.h>
# include <getopt.h>
# include <linux/if_packet.h>
# include <net/ethernet.h>
# include <netdb.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <time.h>
# include <unistd.h>

// TYPES -----------------------------------------------------------------------
typedef enum e_flags {
	NO_FLAGS       = 0x0,
	VERBOSE_OUTPUT = 0x1,
	COUNT          = 0x2,
	FLOOD          = 0x4,
	INTERVAL       = 0x10,
	QUIET          = 0x20,
	LOAD           = 0x40,
	TTL            = 0x80
} t_flags;

typedef struct s_complete_packet {
	struct ip   ip;
	struct icmp icmp;
	char        payload[BUFSIZ - sizeof(struct ip) - sizeof(struct icmp)];

} t_complete_packet;

typedef struct s_connection_data {
	struct sockaddr_in addr;
	socklen_t          addr_len;
	int                sockfd;
	char*              canonname;
	char*              ip_char;
} t_connection_data;

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
	size_t packets_sent;
} t_time_stats;

typedef struct s_payload_pattern {
	uint8_t size;
	uint8_t pattern[16];
} t_payload_pattern;

typedef struct s_config {
	uint16_t id;
	int64_t max_count;
	int64_t interval;
	int64_t preload;
	int32_t ttl;
	t_flags flags;
} t_config;

// GLOBALS ---------------------------------------------------------------------
extern const char*   __progname;
extern volatile bool is_running;
extern t_config      config;

// FUNCTION PROTOTIPES ---------------------------------------------------------
// -- INIT ---------------------------------------------------------------------
void init(int argc, char** argv, t_connection_data* data);
void parser(int argc, char* argv[]);


// -- CONNECTION DATA ----------------------------------------------------------
void get_connection_data(t_connection_data* data, char const* const str_addr);
void destroy_connection_data(t_connection_data* const data);
void error_destroy_connection_data(t_connection_data* data);

// -- FLAGS --------------------------------------------------------------------
void print_header(t_connection_data const* const data);

// -- SIGNALS ------------------------------------------------------------------
void signal_int(int sig);
void signal_quit(int sig);

// -- ICMP PAQUETS -------------------------------------------------------------
void init_icmp(struct icmp* const icmp);
void update_icmp(struct icmp* const icmp, void const* const payload, size_t payload_size);
uint16_t icmp_checksum(struct icmp const* const icmp, void const* const payload, size_t payload_size);

// -- ROUTINES -----------------------------------------------------------------
t_time_stats routines(t_connection_data* data);

// -- PAYLOAD ------------------------------------------------------------------
void init_payload(char const* const str);
void set_payload(void* buffer, size_t size);

#endif
