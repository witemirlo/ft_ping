#include <arpa/inet.h>
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

int get_socket(char const* const addr, t_addr* const data)
{
	int             sockfd;
	int             ret;
	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;

	ret = getaddrinfo(addr, 0, &hints, &result);
	if (ret < 0) {
		fprintf(stderr, "%s: %s: %s\n", __progname, addr, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd >= 0)
			break;
		close(sockfd);
	}

	data->addr = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr;
	strcpy(data->canonname, rp->ai_canonname);

	freeaddrinfo(result);
	if (rp == NULL) {
		fprintf(stderr, "%s: Error: Could not connect\n", __progname);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}
