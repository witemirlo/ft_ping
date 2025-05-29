#include "ft_ping.h"

int64_t max_count = -1;
int64_t interval = 1;
t_flags flags = NO_FLAGS;

static int64_t parse_num(char const* str)
{
	for (size_t i = 0; str[i]; i++) {
		if (!isdigit(str[i])) {
			fprintf(stderr, "%s: invalid valude (`%s' near `%s')\n", __progname, str, str + i);
			return -1;
		}
	}

	return atol(str);
}

void get_flags(int argc, char* argv[])
{
	int     opt;

	flags = NO_FLAGS;
	while ((opt = getopt(argc, argv, "?vcfi")) > 0) {
		switch (opt) {
		case 'v':
			flags |= VERBOSE_OUTPUT;
			break;
		case 'c':
			max_count = parse_num(argv[optind]);
			fprintf(stderr, "%s:%d: %ld\n", __FILE__, __LINE__, max_count); // TODO: BORRAR
			if (max_count < 0)
				exit(EXIT_FAILURE);
			optind++;
			break;
		case 'i':
			if (flags & FLOOD) {
				fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
				exit(EXIT_FAILURE);
			}

			interval = parse_num(argv[optind]);
			if (interval < 1) {
				switch (interval) {
				case 0:
					fprintf(stderr, "%s: value too small: %ld\n", __progname, interval);
					/* FALLTHRU */
				default:
					exit(EXIT_FAILURE);
				}
			}
			optind++;
			break;
		case 'f':
			if (flags & INTERVAL) {
				fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
				exit(EXIT_FAILURE);
			}
			flags |= FLOOD;
			interval = 0;
			if (getuid() != 0) {
				fprintf(stderr, "%s: TODO\n", __progname); // TODO: poner el mensaje que toca
				exit(EXIT_FAILURE);
			}
			break;
		default:
			printf(
				"Usage: %s [OPTION...] HOST ...\n"
				"Send ICMP ECHO_REQUEST packets to network hosts.\n"
				" \nOptions valid for all request types:\n\n"
				"  -c NUMBER                  stop after sending NUMBER packets\n"
				"  -i NUMBER                  wait NUMBER seconds between sending each packet\n"
				" \nOptions valid for --echo requests:\n\n"
				"  -f                         flood ping (root only)\n"
				"  -v                         verbose output\n"
				"  -?                         give this help list\n"
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
}
