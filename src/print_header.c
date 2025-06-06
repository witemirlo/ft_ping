#include "ft_ping.h"

void print_header(t_connection_data const* const data)
{
	printf("PING %s (%s) 56 data bytes", data->canonname, data->ip_char);
	if (config.flags & VERBOSE_OUTPUT)
		printf(", id 0x%x = %d\n", config.id, config.id);
	else
		printf("\n");
}
