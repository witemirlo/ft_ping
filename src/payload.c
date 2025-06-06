#include "ft_ping.h"

t_payload_pattern payload_pattern = {0};

static uint8_t get_size(char const* const str)
{
	uint8_t i;

	for (i = 0; i < 32 && str[i] != '\0'; i++) {}

	return i;
}

static uint8_t hexchar_to_int(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - '7';
	else
		return c - 'W';
}

static void validate_hex_number(char const* const str)
{
	const char valid_chars[] = "0123456789abcdefABCDEF";
	uint8_t    i, str_size;

	str_size = get_size(str);
	for (i = 0; i < str_size; i++) {
		if (strchr(valid_chars, str[i]) == NULL) {
			fprintf(stderr, "%s: error in pattern near %c\n", __progname, str[i]);
			exit(EXIT_FAILURE);
		}
	}
}

void init_payload(char const* const str)
{
	uint8_t i, j, str_size;

	validate_hex_number(str);

	str_size = get_size(str);
	for (i = 0, j = 0; i < str_size; i++) {
		if (i % 2 == 0)
			payload_pattern.pattern[j] = hexchar_to_int(str[i]); 
		else {
			payload_pattern.pattern[j]<<= 4; 
			payload_pattern.pattern[j] |= hexchar_to_int(str[i]); 
			j++;
		}
	}

	payload_pattern.size = (str_size % 2 == 0) ? j : j + 1;
}

void set_payload(void* buffer, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++) {
		((uint8_t*)buffer)[i] = payload_pattern.pattern[i % payload_pattern.size];
	}
}
