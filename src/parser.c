#include "ft_ping.h"

int64_t max_count = -1;
int64_t interval = 1;
t_flags flags = NO_FLAGS;

static int64_t parse_num(char const* const str)
{
	for (size_t i = 0; str[i]; i++) {
		if (!isdigit(str[i])) {
			fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", __progname, str, str + i);
			return -1;
		}
	}

	return atol(str);
}

static void case_v(char const* const str)
{
	(void)str;
	flags |= VERBOSE_OUTPUT;
}

static void case_c(char const* const str)
{
	max_count = parse_num(str);
	if (max_count < 0)
		exit(EXIT_FAILURE);
	optind++;
}

static void case_i(char const* const str)
{
	if (flags & FLOOD) {
		fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
		exit(EXIT_FAILURE);
	}

	interval = parse_num(str);
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
}

static void case_f(char const* const str)
{
	(void)str;
	flags |= FLOOD;
	interval = 0;

	if (flags & INTERVAL) {
		fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
		exit(EXIT_FAILURE);
	}

	if (getuid() != 0) {
		fprintf(stderr, "%s: TODO\n", __progname); // TODO: poner el mensaje que toca
		exit(EXIT_FAILURE);
	}
}

static void case_default(char const* const str)
{
	(void)str;

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

static uint8_t get_case(char c)
{
	switch (c) {
		case 'v': return 1;
		case 'c': return 2;
		case 'i': return 3;
		case 'f': return 4;
		default:  return 0;
	}
}

void parser(int argc, char* argv[])
{
	void (*cases[])(char const* const) = {
		case_default,
		case_v,
		case_c,
		case_i,
		case_f
	};
	int opt;

	flags = NO_FLAGS;
	while ((opt = getopt(argc, argv, "?vcfi")) > 0)
		(cases[get_case(opt)])(argv[optind]);

	if (optind >= argc) {
		fprintf(stderr, "%s: usage error: Destination address required\n", __progname);
		exit(EXIT_FAILURE);
	}
}
