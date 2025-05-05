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

#include "ft_ping.h"

int main(int argc, char* argv[])
{
	int sockfd;
	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int ret = getaddrinfo("8.8.8.8", 0, &hints, &result);
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

	// TODO: realmente necesito poner manualmente el header de ip?
	struct ip ip = {0};
	ip.ip_v = 0x4;
	ip.ip_hl = 0x5; // TODO: calcular el tamano
	ip.ip_len = 0x1c;
	ip.ip_id = 0xabcf; // TODO: what?
	ip.ip_ttl = 0x40;
	ip.ip_p = 0x1;
	memcpy(&(ip.ip_dst), rp->ai_addr, sizeof(*rp->ai_addr));
	// ip.ip_src = ;

	struct icmp icmp = {0};
	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_hun.ih_idseq.icd_id = 0x1234;
	icmp.icmp_hun.ih_idseq.icd_seq = 1;

	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_type, icmp.icmp_code);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_id);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_seq);

	printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);
	icmp.icmp_cksum = 0xffff - icmp.icmp_cksum;
	printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);

	return 0;
}
