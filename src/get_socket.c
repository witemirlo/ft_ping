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
	hints.ai_socktype = SOCK_DGRAM;

	ret = getaddrinfo(addr, 0, &hints, &result);
	if (ret < 0) {
		fprintf(stderr, "%s: %s: %s\n", __progname, addr, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) > -1) // TODO: ping tiene un commando para hacer esto, quizas no deberia hacerlo por defecto?
			break;
		close(sockfd);
	}
	
	freeaddrinfo(result);
	if (rp == NULL) {
		fprintf(stderr, "%s: Error: Could not connect\n", __progname);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}
