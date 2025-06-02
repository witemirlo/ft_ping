#include "ft_ping.h"

bool is_running = true;

void signal_int(int sig)
{
	(void)sig;
	is_running = false;
	fflush(stdout);
}

void signal_quit(int sig)
{
	(void)sig;
	exit(EXIT_FAILURE);
}
