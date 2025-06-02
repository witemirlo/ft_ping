#include "ft_ping.h"

bool is_running = true;
bool multipress = false; // TODO: borrar

void signal_int_send_routine(int sig)
{
	(void)sig;
	is_running = false;
	destroy_connection_data(true);
	interval = 0;

	// if (multipress) 
	// 	exit(EXIT_FAILURE);

	multipress = true;
}

void signal_int_receive_routine(int sig)
{
	(void)sig;
	is_running = false;

	// if (multipress) 
	// 	exit(EXIT_FAILURE);

	multipress = true;
}



void signal_quit(int sig)
{
	(void)sig;
	exit(EXIT_FAILURE);
}
