#include "ft_ping.h"

int main(int argc, char* argv[])
{
	t_connection_data data;

	// TODO: mover a otro sitio
	srand(time(NULL));
        id = rand();

	parser(argc, argv);
	data = get_connection_data(argv[optind]);
	print_header(&data);
	
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
		close(sv[1]);
		routine_send(&data, sv[0]);
	}

	close(sv[0]);
	t_time_stats time_stats = routine_receive(&data, sv[1]);
	
	size_t packets_sent;
	if (recv(sv[1], &packets_sent, sizeof(packets_sent), 0) < 0) {
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		// TODO: hacer que retorne (liberando) un status de error
	}
	close(sv[1]);

	const double packet_loss = (
		((packets_sent - time_stats.packets_received) * 100.)
		/ packets_sent
	);

	// TODO: CON EL FLOOD SALEN UNOS NUMEROS DEMASIADO GRANDES
	printf("--- %s %s statistics ---\n"
		"%lu packets transmitted, %lu packets received, %.1f%% packet loss\n"
		"round-trip min/avg/max = %.3f/%.3f/%.3f ms\n"
		, data.canonname
		, __progname
		, packets_sent
		, time_stats.packets_received
		, packet_loss
		, time_stats.min_time
		, time_stats.avg_time
		, time_stats.max_time
	); // TODO: terminar bien

	destroy_connection_data(&data);
	return EXIT_SUCCESS;
}
