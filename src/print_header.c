#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_ping.h"

void print_header(t_flags flags, t_connection_data* data)
{
	int tmp_fd;

	tmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (tmp_fd < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(tmp_fd);

	if (flags & VERBOSE_OUTPUT)
		printf("%s: sock4.fd: %d (socktype: SOCK_RAW), sock6.fd: -1 (socktype: 0), hints.ai_family: AF_INET\n\n"
			"ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n"
			, __progname
			, tmp_fd
			, data->canonname
		);

	printf("PING %s (%s) 56(84) bytes of data.\n", data->canonname, data->ip_char); // TODO: harcoded, el primer numero es lo enviado - 20 (cabezera ip) - 8 (cabezera icmp)
}
