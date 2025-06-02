#include "ft_ping.h"

static struct addrinfo get_hints(void)
{
	struct addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;

	return hints;
}

static void get_addrinfo(char const* const addr, struct addrinfo const* const hints, struct addrinfo** result)
{
	const int ret = getaddrinfo(addr, 0, hints, result);

	if (ret < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: %s: %s\n", __progname, addr, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
}

static int get_fd_from_addrinfo(struct addrinfo* addr, struct addrinfo** rp)
{
	int sockfd = -1;

	while (addr != NULL) {
		sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sockfd > -1)
			break;
		addr = addr->ai_next;
	}

	if (addr == NULL) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	*rp = addr;
	return sockfd;
}

static void set_socket_options(int sockfd)
{
	const int opt = 1;

	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVERR, &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

char* get_ip_in_chars(struct in_addr addr)
{
	const char* tmp = inet_ntoa(addr);
	char*       ip_char = NULL;

	ip_char = strdup(tmp);
	if (ip_char == NULL) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return ip_char;
}

t_connection_data get_connection_data(char const* const str_addr)
{
	t_connection_data data = {0};

	const struct addrinfo hints = get_hints();
	struct addrinfo       *result = NULL;
	struct addrinfo       *rp = NULL;

	get_addrinfo(str_addr, &hints, &result);
	data.sockfd = get_fd_from_addrinfo(result, &rp);

	if (data.sockfd < 0 || rp == NULL) {
		freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}

	data.addr = *((struct sockaddr_in*)(rp->ai_addr));
	data.addr_len = sizeof(data.addr);
	data.canonname = strdup(rp->ai_canonname);

	freeaddrinfo(result);
	if (data.canonname == NULL) {
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); // TODO: BORRAR
		fprintf(stderr, "%s: Error: %s\n", __progname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	data.ip_char = get_ip_in_chars(data.addr.sin_addr);

	set_socket_options(data.sockfd);

	return data;
}

void destroy_connection_data(bool destroy_sockfd) // TODO: realmente hace falta el bool?
{
	free(data.canonname);
	data.canonname = NULL;

	free(data.ip_char);
	data.ip_char = NULL;

	if (destroy_sockfd)
		close(data.sockfd);
}
