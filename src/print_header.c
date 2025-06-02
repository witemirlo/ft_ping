#include "ft_ping.h"

void print_header(void)
{
	printf("PING %s (%s) 56 data bytes", data.canonname, data.ip_char); // TODO: harcoded, el primer numero es lo enviado - 20 (cabezera ip) - 8 (cabezera icmp)
	if (flags & VERBOSE_OUTPUT)
		printf(", id 0x%x = %d\n", id, id);
	else
		printf("\n");
}
