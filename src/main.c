#include "ft_ping.h"

t_config config = {
	.max_count = -1,
	.interval  = 1000000,
	.preload   = 0,
	.ttl       = 64,
	.flags     = NO_FLAGS
};

int main(int argc, char* argv[])
{
	t_connection_data data = {0};
	t_time_stats      time_stats;

	init(argc, argv, &data);
	time_stats = routines(&data);

	const double packet_loss = (
		((time_stats.packets_sent - time_stats.packets_received) * 100.)
		/ time_stats.packets_sent
	);

	printf("--- %s %s statistics ---\n"
		"%lu packets transmitted, %lu packets received, %.1f%% packet loss\n"
		"round-trip min/avg/max = %.3f/%.3f/%.3f ms\n"
		, data.canonname
		, __progname
		, time_stats.packets_sent
		, time_stats.packets_received
		, packet_loss
		, time_stats.min_time
		, time_stats.avg_time
		, time_stats.max_time
	);

	destroy_connection_data(&data);
	return EXIT_SUCCESS;
}
