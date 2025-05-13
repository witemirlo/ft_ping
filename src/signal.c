#include <unistd.h>

#include "ft_ping.h"

bool is_running = true;

void signal_handler(int sig)
{
	(void)sig;
	is_running = false;
	write(1, "\n", 1);
}
