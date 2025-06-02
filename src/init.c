#include "ft_ping.h"

void init(int argc, char** argv, t_connection_data* data)
{
	srand(time(NULL));
        config.id = rand();

        parser(argc, argv);
        get_connection_data(data, argv[optind]);
        print_header(data);

	signal(SIGINT, signal_int);
	signal(SIGQUIT, signal_quit);
}
