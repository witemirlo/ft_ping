
#include "ft_ping.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

bool is_running = true;
bool multipress = false;

void signal_int(int sig)
{
	(void)sig;
	is_running = false;

	write(1, "\n", 1);
	if (multipress) 
		exit(EXIT_FAILURE);

	multipress = true;
}


void signal_quit(int sig)
{
	(void)sig;
	exit(EXIT_FAILURE);
}
