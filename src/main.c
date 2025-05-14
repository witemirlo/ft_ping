#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
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
	t_flags           flags;
	t_connection_data data;

	flags = get_flags(argc, argv);
	print_verbose_header(flags);
	data = get_connection_data(argv[optind]);
	printf("%s:%d: %s: %s (%d.%d.%d.%d)\n", __FILE__, __LINE__, __func__, data.canonname, (data.addr.sin_addr.s_addr & 0xff), ((data.addr.sin_addr.s_addr >> 8) & 0xff), ((data.addr.sin_addr.s_addr >> 16) & 0xff), ((data.addr.sin_addr.s_addr >> 24) & 0xff)); // TODO: el endianess

	struct icmp icmp = {0};
	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_hun.ih_idseq.icd_id = 0x1234; // TODO: en linux es un numero random
	icmp.icmp_hun.ih_idseq.icd_seq = htons(1);

	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_type, icmp.icmp_code);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_id);
	icmp.icmp_cksum = sum_ones_complement(icmp.icmp_cksum, icmp.icmp_hun.ih_idseq.icd_seq);

	icmp.icmp_cksum = 0xffff - icmp.icmp_cksum;
	
	int tmp3 = inet_pton(AF_INET, data.ip_char, &data.addr.sin_addr.s_addr);

	if (tmp3 < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	} else if (tmp3 == 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}

	char buffer[BUFSIZ] = {0};

	signal(SIGINT, signal_int);
	signal(SIGQUIT, signal_quit);
	int i = 1;
	while (is_running) {
		icmp.icmp_hun.ih_idseq.icd_seq = htons(i++);
		clock_t start = clock();

		if (sendto(data.sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&data.addr, data.addr_len) < 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			return EXIT_FAILURE;
		}

		if (recvfrom(data.sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&data.addr, &data.addr_len) <= 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			return EXIT_FAILURE;
		}

		clock_t end = clock();
		printf("time: %f\n", (((double)(end - start)) / CLOCKS_PER_SEC) * 100000);
		sleep(1); // TODO: el bucle no es exactamente asi, pero tengo que ver si ping hace alguna cola, timeout o si llega un paquete posterior descarta el anterior ni no ha llegado
	}

	destroy_connection_data(&data);
	printf("TODO: data\n");
	return EXIT_SUCCESS;
}
