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
#include <sys/time.h>
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
	init_icmp(&icmp);
	
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

	size_t tmp = 0;
	struct timeval tv = {0};
	struct icmp received = {0};
	while (is_running) {
		memset(buffer, 0, sizeof(buffer));
		update_icmp(&icmp);
		update_icmp_checksum(&icmp);

		if (sendto(data.sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&data.addr, data.addr_len) < 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			return EXIT_FAILURE;
		}

		tmp = recvfrom(data.sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&data.addr, &data.addr_len);
		if (tmp <= 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			return EXIT_FAILURE;
		}

		gettimeofday(&tv, NULL);
		memcpy(&received, buffer + sizeof(struct ip), tmp - sizeof(struct ip));

		uint64_t t1, t2;

		snprintf(buffer, sizeof(buffer), "%lu%lu", tv.tv_sec, tv.tv_usec);
		t1 = strtoull(buffer, NULL, 10);

		snprintf(buffer, sizeof(buffer), "%u%u", ntohl(received.icmp_otime), ntohl(received.icmp_rtime));
		t2 = strtoull(buffer, NULL, 10);

		uint64_t time =  t1 - t2;
		printf("%s:%d: %lu - %lu = %lu\n", __FILE__, __LINE__, t1, t2, time); // TODO: BORRAR

		printf("time: %f\n", (time) / 1000.);
		sleep(1); // TODO: el bucle no es exactamente asi, pero tengo que ver si ping hace alguna cola, timeout o si llega un paquete posterior descarta el anterior ni no ha llegado
	}

	destroy_connection_data(&data);
	printf("TODO: data\n");
	return EXIT_SUCCESS;
}
