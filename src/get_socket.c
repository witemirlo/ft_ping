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
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP,
		.ai_flags = AI_CANONNAME
	};
	
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

static int get_fd_from_addrinfo(struct addrinfo const* addr, t_addr* const data)
{
	int sockfd = -1;

	while (addr != NULL) {
		sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sockfd > -1)
			break;
		addr = addr->ai_next;
	}

	if (addr == NULL)
		fprintf(stderr, "%s: Error: Could not connect\n", __progname);
	else {
		data->addr = ((struct sockaddr_in*)(addr->ai_addr))->sin_addr.s_addr;
		strcpy(data->canonname, addr->ai_canonname);
	}

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

int get_socket(char const* const addr, t_addr* const data)
{
	const struct addrinfo hints = get_hints();
	struct addrinfo       *result = NULL;
	int                   sockfd = -1;

	get_addrinfo(addr, &hints, &result);
	sockfd = get_fd_from_addrinfo(result, data);

	freeaddrinfo(result);
	if (sockfd < 0)
		exit(EXIT_FAILURE);

	set_socket_options(sockfd);

	return sockfd;
}
