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

	char const *const addr = argv[optind];

	int sockfd = get_socket(addr);
	// int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (sockfd < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	int opt = 1;
	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVERR, &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	struct icmp icmp = {0};
	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_hun.ih_idseq.icd_id = 0x1234; // TODO: que id deberia poner?
	icmp.icmp_hun.ih_idseq.icd_seq = 1;

	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_type, icmp.icmp_code);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_id);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_seq);

	// printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);
	icmp.icmp_cksum = 0xffff - icmp.icmp_cksum;
	// printf("%s:%d: %s: %x\n", __FILE__, __LINE__, __func__, icmp.icmp_cksum);
	
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

	// if (sendto(sockfd, &tmp, sizeof(tmp), 0, (struct sockaddr*)&tmp2, tmp22) < 0) {

	clock_t start = clock();
	if (sendto(sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&tmp2, tmp22) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	char buffer[BUFSIZ] = {0};


	// struct icmp prueba = {0};
	// if (recvfrom(sockfd, &prueba, sizeof(prueba), 0, (struct sockaddr*)&tmp2, &tmp22) <= 0) {
	if (recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&tmp2, &tmp22) <= 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	// printf("%d\n",prueba.icmp_type);
	// printf("%d\n",prueba.icmp_code);

	// char buffer1[BUFSIZ] = {0};
	// char buffer2[BUFSIZ] = {0};
	// struct msghdr msg = {
	// 	.msg_name = &buffer1,
	// 	.msg_namelen = BUFSIZ,
	// 	.msg_control = &buffer2,
	// 	.msg_controllen = BUFSIZ,
	// 	.msg_iov = 0,
	// 	.msg_iovlen = 0,
	// 	.msg_flags = 0
	// };
	// if (recvmsg(sockfd, &msg, 0) < 0) {
	// 	fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
	// 	fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
	// 	return EXIT_FAILURE;
	// }
	clock_t end = clock();
	printf("time: %f\n", (((double)(end - start)) / CLOCKS_PER_SEC) * 100000);

	// printf("%d\n", ((struct cmsghdr*)(msg.msg_control))->cmsg_level);

	return EXIT_SUCCESS;
}
