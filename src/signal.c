#include "ft_ping.h"

volatile bool is_running = true;

void signal_int(int sig)
{
	(void)sig;
	is_running = false;
}

void signal_quit(int sig)
{
	(void)sig;
	exit(EXIT_FAILURE);
}
