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
				"Usage: %s [OPTION...] HOST ...\n"
				"Send ICMP ECHO_REQUEST packets to network hosts.\n"
				" \nOptions valid for all request types:\n\n"
				"  -c, NUMBER                 stop after sending NUMBER packets\n"
				"  -i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n"
				" \nOptions valid for --echo requests:\n\n"
				"  -f,                        flood ping (root only)\n"
				"  -v,                        verbose output\n"
				"  -?,                        give this help list\n"
				"\nMandatory or optional arguments to long options are also mandatory or optional\n"
				"for any corresponding short options.\n"
				"\nOptions marked with (root only) are available only to superuser.\n"
				, __progname
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
