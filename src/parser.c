#include "ft_ping.h"

static int64_t parse_num(char const* const str, bool comma, int muliplier)
{
	for (size_t i = 0; str[i]; i++) {
		if (isdigit(str[i]))
			continue;

		if (comma && str[i] == '.') {
			comma = false;
			continue;
		}

		fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", __progname, str, str + i);
		return -1;
	}

	return atof(str) * muliplier;
}

static void case_v(char const* const str)
{
	(void)str;
	config.flags |= VERBOSE_OUTPUT;
}

static void case_c(char const* const str)
{
	config.max_count = parse_num(str, false, 1);
	if (config.max_count < 0)
		exit(EXIT_FAILURE);
	optind++;
}

static void case_i(char const* const str)
{
	const int multiplier = 1000000;

	if (config.flags & FLOOD) {
		fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
		exit(EXIT_FAILURE);
	}

	config.interval = parse_num(str, true, multiplier); // TODO: ahora que son microsegundos, admitir decimales
	if (config.interval < 0)
		exit(EXIT_FAILURE);
	if (config.interval < multiplier) {
		fprintf(stderr, "%s: value too small: %.2f\n", __progname, (double)config.interval / (double)multiplier);
		exit(EXIT_FAILURE);
	}
	optind++;
}

static void case_f(char const* const str)
{
	(void)str;
	config.flags |= FLOOD;
	config.interval = 10000;

	if (config.flags & INTERVAL) {
		fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
		exit(EXIT_FAILURE);
	}

	if (getuid() != 0) {
		fprintf(stderr, "%s: Error: Operation not permitted\n", __progname);
		exit(EXIT_FAILURE);
	}
}

static void case_q(char const* const str)
{
	(void)str;
	config.flags |= QUIET;
}

static void case_p(char const* const str)
{
	(void)str;
	init_payload(str);
	optind++;
}

static void case_l(char const* const str)
{
	(void)str;
	config.flags |= LOAD;
	config.preload = parse_num(str, false, 1);

	if (config.preload < 0)
		exit(EXIT_FAILURE);

	if (getuid() != 0) {
		fprintf(stderr, "%s: Error: Operation not permitted\n", __progname);
		exit(EXIT_FAILURE);
	}

	optind++;
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
		"  -l NUMBER                  send NUMBER packets as fast as possible before\n"
                "                             falling into normal mode of behavior (root only)\n"
		"  -p PATTERN                 fill ICMP packet with given pattern (hex)"
		"  -q                         quiet output\n"
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
		case 'q': return 5;
		case 'p': return 6;
		case 'l': return 7;
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
		case_f,
		case_q,
		case_p,
		case_l
	};
	int opt;

	init_payload("0");
	config.flags = NO_FLAGS;
	while ((opt = getopt(argc, argv, "?vcfiqfpl")) > 0)
		(cases[get_case(opt)])(argv[optind]);

	if (optind >= argc) {
		fprintf(stderr, "%s: usage error: Destination address required\n", __progname);
		exit(EXIT_FAILURE);
	}
}
