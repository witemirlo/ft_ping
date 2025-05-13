#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ft_ping.h"

static struct addrinfo get_hints(void)
{
	struct addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;

	return hints;
}

static void get_addrinfo(char const* const addr, struct addrinfo const* const hints, struct addrinfo** result)
{
	const int ret = getaddrinfo(addr, 0, hints, result);

	if (ret < 0) {
		fprintf(stderr, "%s: %s: %s\n", __progname, addr, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
}

static int get_fd_from_addrinfo(struct addrinfo* addr, struct addrinfo** rp)
{
	int sockfd = -1;

	while (addr != NULL) {
		sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sockfd > -1)
			break;
		addr = addr->ai_next;
	}

	if (addr == NULL) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	*rp = addr;
	return sockfd;
}

static void set_socket_options(int sockfd)
{
	const int opt = 1;

	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVERR, &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

t_connexion_data get_connexion_data(char const* const str_addr)
{
	char                  *canonname = NULL;
	int                   sockfd = -1;
	in_addr_t             addr;

	const struct addrinfo hints = get_hints();
	struct addrinfo       *result = NULL;
	struct addrinfo       *rp = NULL;

	get_addrinfo(str_addr, &hints, &result);
	sockfd = get_fd_from_addrinfo(result, &rp);

	if (sockfd < 0 || rp == NULL) {
		freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}

	addr = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr;
	canonname = strdup(rp->ai_canonname);
	if (canonname == NULL) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);
	set_socket_options(sockfd);

	return (t_connexion_data){.addr = addr, .sockfd = sockfd, .canonname = canonname};
}
