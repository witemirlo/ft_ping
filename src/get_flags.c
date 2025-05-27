#include "ft_ping.h"

t_flags get_flags(int argc, char* argv[])
{
	t_flags flags;
	int     opt;

	flags = NO_FLAGS;
	while ((opt = getopt(argc, argv, "?v")) > 0) {
		switch (opt) {
		case 'v':
			flags |= VERBOSE_OUTPUT;
			break;
		default:
			printf(
				"\n"
				"Usage\n"
				"  ping [options] <destination>\n"
				"\n"
				"Options:\n"
				"  <destination>      DNS name or IP address\n"
				"  -v                 verbose output\n"
			);
			exit(EXIT_SUCCESS);
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: usage error: Destination address required\n", __progname);
		exit(EXIT_FAILURE);
	}

	return flags;
}
