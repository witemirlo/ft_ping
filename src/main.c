#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int argc, char* argv[])
{
	t_flags flags = get_flags(argc, argv);

	char const *const addr = argv[optind];

	// int sockfd = get_socket(addr);
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	if (sockfd < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	int opt = 1;
	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}


	// TODO: realmente necesito poner manualmente el header de ip?
	struct ip ip = {0};
	ip.ip_v = 0x4;
	ip.ip_hl = 0x5; // TODO: calcular el tamano
	ip.ip_len = 0x1c;
	ip.ip_id = 0xabcf; // TODO: what?
	ip.ip_ttl = 0x40;
	ip.ip_p = 0x1;
	ip.ip_dst.s_addr = GET_OCTET(8, 8, 8, 8); // TODO: poner la que toca
	// ip.ip_src = ;

	struct icmp icmp = {0};
	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_hun.ih_idseq.icd_id = 0x1234;
	icmp.icmp_hun.ih_idseq.icd_seq = 1;

	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_type << 8, icmp.icmp_code);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_id);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_seq);

	// printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);
	icmp.icmp_cksum = 0xffff - icmp.icmp_cksum;
	// printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);


	uint8_t tmp[sizeof(ip) + sizeof(icmp)] = {0};
	memcpy(tmp, &ip, sizeof(ip));
	memcpy(tmp + sizeof(ip), &icmp, sizeof(icmp));
	
	if (send(sockfd, &tmp, sizeof(tmp), 0) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	sleep(1);

	char buffer[BUFSIZ] = {0};
	if (recv(sockfd, buffer, BUFSIZ, 0) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
