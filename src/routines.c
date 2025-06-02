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

static void print_data_received(t_connection_data const* const data, t_complete_packet const* const packet, t_time_info const* const time_info)
{
	if (config.flags & QUIET)
		return;

	if (config.flags & FLOOD) {
		write(1, "\b \b", 3);
		return;
	}

	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n"
		, ntohs(packet->ip.ip_len) - (uint16_t)sizeof(struct ip)
		, data->ip_char
		, ntohs(packet->icmp.icmp_seq)
		, packet->ip.ip_ttl
		, time_info->time
	);
}

static t_time_stats routine_receive(t_connection_data* const data, int fd, pid_t pid) // TODO: refactor
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
			close(fd);
			kill(pid, SIGINT);
			error_destroy_connection_data(data);
		}

		if (packet.icmp.icmp_type != ICMP_ECHOREPLY)
			continue;

		if(packet.icmp.icmp_id != config.id)
			continue;

		count++;
		if (config.max_count > 0 && count >= config.max_count)
			is_running = false;

		time_info = get_time_info(buffer, sizeof(buffer), count, packet.icmp.icmp_otime, packet.icmp.icmp_rtime);
		print_data_received(data, &packet, &time_info);
	}

	kill(pid, SIGINT);

	return (t_time_stats){
		.min_time = time_info.min_time,
		.avg_time = time_info.avg_time,
		.max_time = time_info.max_time,
		.packets_received = count
	};
}

static bool send_msg(t_connection_data* const data, void* const buffer, size_t size)
{
	update_icmp(buffer, (uint8_t*)buffer + sizeof(struct icmp), size);

	if (sendto(data->sockfd, buffer, sizeof(struct icmp) + size, 0, (struct sockaddr*)&data->addr, data->addr_len) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		return false;
	}

	if (config.flags & FLOOD && !(config.flags & QUIET))
		write(1, ".", 1);

	return true;
}
 
static void routine_send(t_connection_data* const data, int fd)
{
	ssize_t count;
	char    msg[sizeof(struct icmp) + 36]; // TODO: hacer typedef msg

	count = 0;

	init_icmp((struct icmp*)msg);
	memset(msg + sizeof(struct icmp), 0, sizeof(msg) - sizeof(struct icmp));
	set_payload(msg + sizeof(struct icmp), sizeof(msg) - sizeof(struct icmp));

	if (config.flags & LOAD) {
		if (config.max_count > 0)
			config.max_count += config.preload;
		for (int64_t i = 0; i < config.preload; i++) {
			if (!send_msg(data, msg, sizeof(msg) - sizeof(struct icmp))) {
				is_running = false;
				kill(getppid(), SIGINT);
				break;
			}
			count++;
		}
	}

	while (is_running) {
		if (!send_msg(data, msg, sizeof(msg) - sizeof(struct icmp))) {
			kill(getppid(), SIGINT);
			break;
		}
		count++;
		if (config.max_count > 0 && count >= config.max_count)
			break;
		usleep(config.interval);
	}

	destroy_connection_data(data);
	send(fd, &count, sizeof(count), 0);
	close(fd);
	exit(0);
}

t_time_stats routines(t_connection_data* data)
{
	t_time_stats time_stats = {0};
	pid_t        pid;
	int          sv[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
		error_destroy_connection_data(data);

	pid = fork();

	if (pid < 0)
		error_destroy_connection_data(data);
	else if (pid == 0) {
		close(sv[1]);
		routine_send(data, sv[0]);
	}

	close(sv[0]);
	time_stats = routine_receive(data, sv[1], pid);
	
	if (recv(sv[1], &time_stats.packets_sent, sizeof(time_stats.packets_sent), 0) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		// TODO: hacer que retorne (liberando) un status de error
	}
	close(sv[1]);

	return time_stats;
}
