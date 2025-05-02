#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int sockfd;
	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int ret = getaddrinfo("216.58.209.78", 0, &hints, &result);
	if (ret < 0) {
		gai_strerror(ret);
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) > -1)
			break;
		close(sockfd);
	}
	
	freeaddrinfo(result);
	if (rp == NULL) {
		dprintf(2, "Error: Could not connect\n");
		exit(EXIT_FAILURE);
	}

	struct ip ip;
	struct icmp icmp;

	return 0;
}
