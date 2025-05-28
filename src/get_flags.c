#include "ft_ping.h"

ssize_t max_count = -1;

t_flags get_flags(int argc, char* argv[])
{
	t_flags flags;
	int     opt;

	flags = NO_FLAGS;
	while ((opt = getopt(argc, argv, "?vc")) > 0) {
		switch (opt) {
		case 'v':
			flags |= VERBOSE_OUTPUT;
			break;
		case 'c':
			// TODO: HACER FUNCION VALIDADORA
			for (unsigned int i = 0; argv[optind][i]; i++) {
				if (!isdigit(argv[optind][i])) {
					fprintf(stderr, "%s: invalid valude (`%s' near `%s')\n", __progname, argv[optind], argv[optind] + i);
					exit(EXIT_FAILURE);
				}
			}
			max_count = atol(argv[optind]);
			optind++;
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
