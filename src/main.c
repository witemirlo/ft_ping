#include <arpa/inet.h>
#include <errno.h>
#include <float.h>
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

void routine_send(t_connection_data* const data, int fd)
{
	struct icmp icmp;
	size_t      count;
	int         status;

	init_icmp(&icmp);
	status = 0;
	count = 0;
	// TODO: el tamaño del paquete
	while (is_running) {
		update_icmp(&icmp);
		update_icmp_checksum(&icmp);

		if (sendto(data->sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&data->addr, data->addr_len) < 0) {
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			status = errno;
			break;
		}
		count++;
		sleep(1);
	}
	if (sendto(data->sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&data->addr, data->addr_len) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
	}
	destroy_connection_data(data);
	send(fd, &count, sizeof(count), 0);
	close(fd);
	exit(status); // TODO: al final se usa el status?
}

t_time_info get_time_info(char* buffer, size_t buffer_len, size_t count, uint32_t otime, uint32_t rtime)
{
	t_time_info    time_info;
	struct timeval tv;
	uint64_t       t1, t2;

	gettimeofday(&tv, NULL);
	time_info.min_time = DBL_MAX;
	time_info.avg_time = 0;
	time_info.max_time = 0;

	// TODO: dado que son dos enteros de 32 bits, no seria lo mismo mover otime 32 bytes a la izquierda?
	snprintf(buffer, buffer_len, "%lu%lu", tv.tv_sec, tv.tv_usec);
	t1 = strtoull(buffer, NULL, 10);

	snprintf(buffer, buffer_len, "%u%u", ntohl(otime), ntohl(rtime));
	t2 = strtoull(buffer, NULL, 10);

	time_info.time =  (t1 - t2) / 1000.;

	// TODO: no salen bien las estadisticas
	time_info.min_time = (time_info.time < time_info.min_time) ? time_info.time : time_info.min_time;
	time_info.max_time = (time_info.time > time_info.max_time) ? time_info.time : time_info.max_time;
	time_info.avg_time = (time_info.avg_time + time_info.time) / count;

	// printf("\n%s:%d: %lu - %lu = %f\n", __FILE__, __LINE__, t1, t2, time_info.time); // TODO: BORRAR
	return time_info;
}

t_time_stats routine_receive(t_connection_data* const data, int fd)
{
	t_complete_packet packet;
	char              buffer[BUFSIZ];
	size_t            bytes_readed, count;
	t_time_info       time_info;

	count = 0;
	while (is_running) {
		// TODO: cuando haces ping a localhost te manda dos paquetes de vuelta e imprime ambos, el original no
		bytes_readed = recvfrom(data->sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&data->addr, &data->addr_len);
		if (!is_running)
			break;
		if (bytes_readed <= 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			destroy_connection_data(data);
			close(fd);
			exit(EXIT_FAILURE);
		}
		count++;
		time_info = get_time_info(buffer, sizeof(buffer), count, packet.icmp.icmp_otime, packet.icmp.icmp_rtime);
		getnameinfo((struct sockaddr const *)&data->addr, data->addr_len, buffer, sizeof(buffer), NULL, 0, 0);
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2f ms\n"
			, ntohs(packet.ip.ip_len)
			, buffer
			, data->ip_char
			, ntohs(packet.icmp.icmp_seq)
			, packet.ip.ip_ttl
			, time_info.time);
	}

	return (t_time_stats){
		.min_time = time_info.min_time,
		.avg_time = time_info.avg_time,
		.max_time = time_info.max_time,
		.packets_received = count
	};
}

int main(int argc, char* argv[])
{
	t_flags           flags;
	t_connection_data data;

	flags = get_flags(argc, argv);
	data = get_connection_data(argv[optind]);
	print_header(flags, &data);
	
	// TODO: esto era para ver que era una direccion valida o para que?
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

	// TODO: mover esto de aqui y definir mejor que tendrian que hacer
	signal(SIGINT, signal_int);
	signal(SIGQUIT, signal_quit);

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		destroy_connection_data(&data);
		return EXIT_FAILURE; // TODO: realmente esto podria ser que simplemente llegue al final y punto
	}

	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		destroy_connection_data(&data);
		return EXIT_FAILURE; // TODO: realmente esto podria ser que simplemente llegue al final y punto
	} else if (pid == 0) {
		// enviar ping
		close(sv[1]);
		routine_send(&data, sv[0]);
	}
	// recibir ping
	close(sv[0]);
	t_time_stats time_stats = routine_receive(&data, sv[1]);

	size_t packets_sent;
	if (recv(sv[1], &packets_sent, sizeof(packets_sent), 0) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		// TODO: hacer que retorne (liberando) un status de error
	}
	close(sv[1]);
	// TODO: habría que enviar señal al proceso hijo?

	// TODO: un waitpid o algo?

	const double packet_loss = (
		((packets_sent - (packets_sent - time_stats.packets_received)) * 100.)
		/ packets_sent
	);

	printf("\n--- %s %s statistics ---\n"
		"%lu packets transmitted, %lu received, %.2f%% packet loss, time %dms\n"
		"rtt min/avg/max = %.3f/%.3f/%.3f/ ms\n"
		, data.canonname
		, __progname
		, packets_sent
		, time_stats.packets_received
		, packet_loss
		, 42 // TODO: ver que es y calcular o modificar time_stats
		, time_stats.min_time
		, time_stats.avg_time
		, time_stats.max_time
	); // TODO: terminar bien

	destroy_connection_data(&data);
	return EXIT_SUCCESS;
}
