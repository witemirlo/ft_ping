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

int get_socket(char const* const addr)
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
		printf("rp->ai_family: %d, rp->ai_socktype: %d, rp->ai_protocol: %d\n", rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd >= 0)
			break;
		close(sockfd);
	}

	printf("%p\n", rp);
	
	// TODO: que se pase un puntero y le metes la info de rp ahi
	int tmp = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr; // TODO: este el el address que tendre que poner en el paquete
	printf("%s:%d: %s: %s (%d.%d.%d.%d)\n", __FILE__, __LINE__, __func__, rp->ai_canonname, (tmp & 0xff), ((tmp >> 8) & 0xff), ((tmp >> 16) & 0xff), ((tmp >> 24) & 0xff)); // TODO: el endianess

	freeaddrinfo(result);
	if (rp == NULL) {
		fprintf(stderr, "%s: Error: Could not connect\n", __progname);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}
