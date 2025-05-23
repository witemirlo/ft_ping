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

#include <sys/queue.h>

#include "ft_ping.h"

struct entry {
	double time;
	SLIST_ENTRY(entry) entries;
};

SLIST_HEAD(slisthead, entry);

int main(int argc, char* argv[])
{
	t_flags           flags;
	t_connection_data data;

	//----------------------------------------------------------------------
	struct entry *node;
	struct slisthead head;

	SLIST_INIT(&head);
	//----------------------------------------------------------------------

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
	struct ip ip = {0};


	size_t count1 = 0, count2 = 0;
	while (is_running) {
		memset(buffer, 0, sizeof(buffer));
		update_icmp(&icmp);
		update_icmp_checksum(&icmp);

		if (sendto(data.sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&data.addr, data.addr_len) < 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			// TODO: control de leaks
			return EXIT_FAILURE;
		}
		count1++;

		// TODO: Habria que meter paralelismo?
		tmp = recvfrom(data.sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&data.addr, &data.addr_len);
		if (tmp <= 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			// TODO: control de leaks
			return EXIT_FAILURE;
		}
		count2++;

		gettimeofday(&tv, NULL);

		memcpy(&ip, buffer, sizeof(struct ip));
		memcpy(&received, buffer + sizeof(struct ip), tmp - sizeof(struct ip));


		uint64_t t1, t2;

		snprintf(buffer, sizeof(buffer), "%lu%lu", tv.tv_sec, tv.tv_usec);
		t1 = strtoull(buffer, NULL, 10);

		snprintf(buffer, sizeof(buffer), "%u%u", ntohl(received.icmp_otime), ntohl(received.icmp_rtime));
		t2 = strtoull(buffer, NULL, 10);

		double time =  (t1 - t2) / 1000.;

		node = malloc(sizeof(*node));
		if (node == NULL) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			// TODO: control de leaks
			return EXIT_FAILURE;
		}

		node->time = time;
		SLIST_INSERT_HEAD(&head, node, entries);

		printf("\n%s:%d: %lu - %lu = %f\n", __FILE__, __LINE__, t1, t2, time); // TODO: BORRAR

		getnameinfo((struct sockaddr const *)&data.addr, data.addr_len, buffer, sizeof(buffer), NULL, 0, 0);
		// printf("time: %f\n", (time) / 1000.);
		printf("%d bytes from %s (TODO: ¿HAY ALGUNA MANERA DE ACERLO SIN BITWISE?): icmp_seq=%d ttl=%d time=%.2f ms\n", ntohs(ip.ip_len), buffer, ntohs(received.icmp_seq), ip.ip_ttl, time);
		sleep(1); // TODO: el bucle no es exactamente asi, pero tengo que ver si ping hace alguna cola, timeout o si llega un paquete posterior descarta el anterior ni no ha llegado
	}

	double total_time = 0; // TODO: seguro que lo otro mide esto?
	SLIST_FOREACH(node, &head, entries)
		total_time += node->time;

	printf("--- %s %s statistics ---\n"
		"%lu packets transmitted, %lu received, %f%% packet loss, time %fms\n"
		"rtt min/avg/max/mdev = TODO: 5.213/6.849/10.907/2.351 ms\n"
		, data.canonname
		, __progname
		, count1
		, count2
		, ((double)count1 - (double)count2) / 100.
		, total_time
	); // TODO: terminar bien

	while (!SLIST_EMPTY(&head)) {
               node = SLIST_FIRST(&head);
               SLIST_REMOVE_HEAD(&head, entries);
               free(node);
	}


	destroy_connection_data(&data);
	printf("TODO: data\n");
	return EXIT_SUCCESS;
}
