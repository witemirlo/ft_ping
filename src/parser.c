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

static void case_verbose(char const* const str)
{
	(void)str;
	config.flags |= VERBOSE_OUTPUT;
}

static void case_count(char const* const str)
{
	config.max_count = parse_num(str, false, 1);
	if (config.max_count < 0)
		exit(EXIT_FAILURE);
}

static void case_interval(char const* const str)
{
	const int multiplier = 1000000;

	if (config.flags & FLOOD) {
		fprintf(stderr, "%s: -f and -i incompatible options\n", __progname);
		exit(EXIT_FAILURE);
	}

	config.interval = parse_num(str, true, multiplier);
	if (config.interval < 0)
		exit(EXIT_FAILURE);
	if (config.interval < (multiplier / 5)) {
		fprintf(stderr, "%s: value too small: %.2f\n", __progname, (double)config.interval / (double)multiplier);
		exit(EXIT_FAILURE);
	}
}

static void case_flood(char const* const str)
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

static void case_quiet(char const* const str)
{
	(void)str;
	config.flags |= QUIET;
}

static void case_pattern(char const* const str)
{
	(void)str;
	init_payload(str);
}

static void case_preload(char const* const str)
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
}

static void case_ttl(char const* const str)
{
	config.flags |= TTL;
	config.ttl = (int32_t)parse_num(str, false, 1);

	if (config.ttl < 0)
		exit(EXIT_FAILURE);

	if (config.ttl == 0) {
		fprintf(stderr, "%s: option value too small: %d\n", __progname, config.ttl);
		exit(EXIT_FAILURE);
	}

	if (config.ttl > 255) {
		fprintf(stderr, "%s: option value too big: %d\n", __progname, config.ttl);
		exit(EXIT_FAILURE);
	}
}

static void case_default(char const* const str)
{
	(void)str;

	printf(
		"Usage: %s [OPTION...] HOST ...\n"
		"Send ICMP ECHO_REQUEST packets to network hosts.\n"
		"\n"
		" Options valid for all request types:\n"
		"\n"
		"  -c, --count=NUMBER         stop after sending NUMBER packets\n"
		"  -i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n"
		"      --ttl=N                specify N as time-to-live\n"
		"  -v, --verbose              verbose output\n"
		"\n"
		" Options valid for --echo requests:\n"
		"\n"
		"  -f, --flood                flood ping (root only)\n"
		"  -l, --preload=NUMBER       send NUMBER packets as fast as possible before\n"
		"                             falling into normal mode of behavior (root only)\n"
		"  -p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n"
		"  -q, --quiet                quiet output\n"
		"\n"
		"  -?, --help                 give this help list\n"
		"      --usage                give a short usage message\n"
		"\n"
		"Mandatory or optional arguments to long options are also mandatory or optional\n"
		"for any corresponding short options.\n"
		"\n"
		"Options marked with (root only) are available only to superuser.\n"
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
		case 1:   return 8;
		default:  return 0;
	}
}

void parser(int argc, char* argv[])
{
	void (*cases[])(char const* const) = {
		case_default,
		case_verbose,
		case_count,
		case_interval,
		case_flood,
		case_quiet,
		case_pattern,
		case_preload,
		case_ttl
	};
	int opt;

	init_payload("0");
	config.flags = NO_FLAGS;

	int option_index = 0;
	static struct option long_options[] = {
		{"ttl",      required_argument, 0,  1},
		{"count",    required_argument, 0,  'c'},
		{"interval", required_argument, 0,  'i'},
		{"verbose",  no_argument,       0,  'v'},
		{"flood",    no_argument,       0,  'f'},
		{"preload",  required_argument, 0,  'l'},
		{"pattern",  required_argument, 0,  'p'},
		{"quiet",    no_argument,       0,  'q'},
		{"help",     no_argument,       0,  '?'},
		{"usage",    no_argument,       0,  '?'},
		{0,          0,                 0,  0}
	};

	while ((opt = getopt_long(argc, argv, "?vc:fi:qfp:l:", long_options, &option_index)) > 0) {
		(cases[get_case(opt)])(optarg);
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: usage error: Destination address required\n", __progname);
		exit(EXIT_FAILURE);
	}
}
