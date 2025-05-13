#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int argc, char* argv[])
{
	t_flags flags = get_flags(argc, argv);

	t_connexion_data data = get_connexion_data(argv[1]);
	printf("%s:%d: %s: %s (%d.%d.%d.%d)\n", __FILE__, __LINE__, __func__, data.canonname, (data.addr & 0xff), ((data.addr >> 8) & 0xff), ((data.addr >> 16) & 0xff), ((data.addr >> 24) & 0xff)); // TODO: el endianess

	struct icmp icmp = {0};
	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_hun.ih_idseq.icd_id = 0x1234; // TODO: que id deberia poner?
	icmp.icmp_hun.ih_idseq.icd_seq = 1;

	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_type, icmp.icmp_code);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_id);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_seq);

	icmp.icmp_cksum = 0xffff - icmp.icmp_cksum;
	
	struct sockaddr_in tmp2 = {0};
	socklen_t tmp22 = sizeof(tmp2);
	tmp2.sin_family = AF_INET;

	int tmp3 = inet_pton(AF_INET, argv[1], &tmp2.sin_addr.s_addr);

	if (tmp3 < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	} else if (tmp3 == 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: not a valid network address\n", __progname);
		return EXIT_FAILURE;
	}

	char buffer[BUFSIZ] = {0};
	clock_t start = clock();
	if (sendto(sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&tmp2, tmp22) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	if (recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&tmp2, &tmp22) <= 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}
	
	clock_t end = clock();
	printf("time: %f\n", (((double)(end - start)) / CLOCKS_PER_SEC) * 100000);

	return EXIT_SUCCESS;
}
