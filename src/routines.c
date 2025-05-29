#include "ft_ping.h"

static t_time_info get_time_info(char* buffer, size_t buffer_len, size_t count, uint32_t otime, uint32_t rtime)
{
	static t_time_info time_info = {.min_time = DBL_MAX, .avg_time = 0, .max_time = 0};
	static double      total_time = 0;
	struct timeval     tv;
	uint64_t           t1, t2;

	gettimeofday(&tv, NULL);

	snprintf(buffer, buffer_len, "%lu%lu", tv.tv_sec, tv.tv_usec);
	t1 = strtoull(buffer, NULL, 10);

	snprintf(buffer, buffer_len, "%u%u", ntohl(otime), ntohl(rtime));
	t2 = strtoull(buffer, NULL, 10);

	time_info.time =  (t1 - t2) / 1000.; // TODO: ESTO DA UNAS BURRADAS DE TIEMPO A VECES
	total_time += time_info.time;

	time_info.min_time = (time_info.time < time_info.min_time) ? time_info.time : time_info.min_time;
	time_info.max_time = (time_info.time > time_info.max_time) ? time_info.time : time_info.max_time;
	time_info.avg_time = total_time / count;
	return time_info;
}

t_time_stats routine_receive(t_connection_data* const data, int fd)
{
	t_complete_packet packet;
	char              buffer[BUFSIZ];
	ssize_t           bytes_readed, count;
	t_time_info       time_info;

	count = 0;
	while (is_running) {
		bytes_readed = recvfrom(data->sockfd, &packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr*)&data->addr, &data->addr_len);
		if (!is_running)
			break;
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			errno = 0;
			continue;
		}
		if (bytes_readed <= 0) {
			fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			destroy_connection_data(data);
			close(fd);
			// TODO: enviar senal al hijo para que no deje huerfanos
			exit(EXIT_FAILURE);
		}
		if (packet.icmp.icmp_type != ICMP_ECHOREPLY)
			continue;
		if (packet.icmp.icmp_id != id)
			continue;
		count++;
		if (max_count > 0 && count >= max_count)
			is_running = false;
		time_info = get_time_info(buffer, sizeof(buffer), count, packet.icmp.icmp_otime, packet.icmp.icmp_rtime);
		if (flags & FLOOD)
			write(1, "\b \b", 3);
		else
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n"
				, ntohs(packet.ip.ip_len) - (uint16_t)sizeof(struct ip)
				// , buffer
				, data->ip_char
				, ntohs(packet.icmp.icmp_seq)
				, packet.ip.ip_ttl
				, time_info.time
			);
	}

	return (t_time_stats){
		.min_time = time_info.min_time,
		.avg_time = time_info.avg_time,
		.max_time = time_info.max_time,
		.packets_received = count
	};
}

void routine_send(t_connection_data* const data, int fd)
{
	struct icmp icmp;
	ssize_t     count;
	int         status;
	char        msg[sizeof(icmp) + 36];

	init_icmp(&icmp);
	memset(msg, 0, sizeof(msg));

	status = 0;
	count = 0;
	while (is_running) {
		update_icmp(&icmp);
		update_icmp_checksum(&icmp);

		memcpy(msg, &icmp, sizeof(icmp));
		// TODO: hacerlo no bloqueante
		if (sendto(data->sockfd, msg, sizeof(msg), 0, (struct sockaddr*)&data->addr, data->addr_len) < 0) {
			fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
			status = errno;
			break;
		}
		count++;
		if (flags & FLOOD)
			write(1, ".", 1);
		if (max_count > 0 && count >= max_count)
			is_running = false;
		usleep(interval);
	}

	destroy_connection_data(data);
	send(fd, &count, sizeof(count), 0);
	close(fd);
	exit(status); // TODO: al final se usa el status?
}
