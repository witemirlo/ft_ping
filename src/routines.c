#include "ft_ping.h"

static t_time_info get_time_info(size_t count, uint32_t otime, uint32_t rtime)
{
	static t_time_info time_info = {.min_time = DBL_MAX, .avg_time = 0, .max_time = 0};
	static long double total_time = 0;
	long double        t1, t2;
	struct timeval     tv;

	gettimeofday(&tv, NULL);

	t1 = ((uint32_t)tv.tv_sec * 1000) + ((uint32_t)tv.tv_usec / 1000.);
	t2 = (htonl(otime) * 1000) + (htonl(rtime) / 1000.);

	time_info.time =  t1 - t2;
	total_time += time_info.time;

	time_info.min_time = (time_info.time < time_info.min_time) ? time_info.time : time_info.min_time;
	time_info.max_time = (time_info.time > time_info.max_time) ? time_info.time : time_info.max_time;
	time_info.avg_time = total_time / count;
	return time_info;
}

static void print_data_received(t_complete_packet const* const packet, t_time_info const* const time_info)
{
	if (config.flags & QUIET)
		return;

	if (config.flags & FLOOD) {
		printf("\b \b");
		return;
	}

	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n"
		, ntohs(packet->ip.ip_len) - (uint16_t)sizeof(struct ip)
		, inet_ntoa(packet->ip.ip_src)
		, ntohs(packet->icmp.icmp_seq)
		, packet->ip.ip_ttl
		, time_info->time
	);
}

static void print_ttl_exceeded(t_complete_packet const* const packet)
{
	printf("%d bytes from %s: Time to live exceeded\n"
		, ntohs(packet->ip.ip_len) - (uint16_t)sizeof(struct ip)
		, inet_ntoa(packet->ip.ip_src)
	);
}

static t_time_stats routine_receive(t_connection_data* const data, int fd, pid_t pid)
{
	t_complete_packet packet;
	ssize_t           bytes_readed, count, packets_received;
	t_time_info       time_info;
	uint16_t          packet_id;
	fd_set            set;
	struct timeval    tv;

	FD_ZERO(&set);

	count = 0;
	packets_received = 0;
	while (is_running) {
		if (config.max_count > 0 && count >= config.max_count)
			is_running = false;

		FD_SET(data->sockfd, &set);
		memset(&tv, 0, sizeof(tv));
		if (select(data->sockfd + 1, &set, NULL, NULL, &tv) < 0) {
			kill(pid, SIGINT);
			break;
		}

		if (!FD_ISSET(data->sockfd, &set)) {
			continue;
		}

		memset(&packet, 0, sizeof(packet));
		bytes_readed = recvfrom(data->sockfd, &packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr*)&data->addr, &data->addr_len);

		if (bytes_readed <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				errno = 0;
				continue;
			}

			if (bytes_readed == 0) {
				close(fd);
				kill(pid, SIGINT);
				error_destroy_connection_data(data);
			}
		}

		if (packet.icmp.icmp_type == ICMP_TIME_EXCEEDED) {
			packet_id = ((uint16_t*)(&packet))[10 + 4 + 10 + 2];
			if (packet_id != config.id)
				continue;
			count++;
			print_ttl_exceeded(&packet);
			continue;
		}

		if(packet.icmp.icmp_id != config.id)
			continue;

		if (packet.icmp.icmp_type == ICMP_ECHO)
			continue;

		count++;
		packets_received++;
		time_info = get_time_info(count, packet.icmp.icmp_otime, packet.icmp.icmp_rtime);
		print_data_received(&packet, &time_info);
	}

	kill(pid, SIGINT);

	return (t_time_stats){
		.min_time = time_info.min_time,
		.avg_time = time_info.avg_time,
		.max_time = time_info.max_time,
		.packets_received = packets_received
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
		printf(".");

	return true;
}
 
static void routine_send(t_connection_data* const data, int fd)
{
	ssize_t count;
	char    msg[sizeof(struct icmp) + 36];

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

	send(fd, &count, sizeof(count), 0);
	destroy_connection_data(data);
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

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	pid = fork();

	if (pid < 0)
		error_destroy_connection_data(data);
	else if (pid == 0) {
		close(sv[1]);
		routine_send(data, sv[0]);
	}

	close(sv[0]);
	time_stats = routine_receive(data, sv[1], pid);
	
	waitpid(pid, NULL, 0);
	if (recv(sv[1], &time_stats.packets_sent, sizeof(time_stats.packets_sent), 0) < 0) {
		close(sv[1]);
		error_destroy_connection_data(data);
	}
	
	close(sv[1]);
	return time_stats;
}
