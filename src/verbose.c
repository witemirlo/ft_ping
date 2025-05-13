#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_ping.h"

void print_verbose_header(t_flags flags)
{
	int tmp_fd;

	if (!(flags & VERBOSE_OUTPUT))
		return;

	tmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (tmp_fd < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(tmp_fd);
	printf("%s: sock4.fd: %d (socktype: SOCK_RAW), sock6.fd: -1 (socktype: 0), hints.ai_family: AF_INET\n\n", __progname, tmp_fd);
}
