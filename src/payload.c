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

static bool valid_hex_number(char const* const str)
{
	const char valid_chars[] = "0123456789abcdefABCDEF";
	uint8_t    i, str_size;

	str_size = get_size(str);
	for (i = 0; i < str_size; i++) {
		if (strchr(valid_chars, str[i]) == NULL)
			return false;
	}

	return true;
}

bool init_payload(char const* const str)
{
	// 112233445566778899aabbccddeeff42 como maximo
	uint8_t i, j, str_size;

	if (!valid_hex_number(str))
		return NULL;

	str_size = get_size(str);
	for (i = 0, j = 0; i < str_size; i++) {
		printf("i: %d, j: %d\n", i, j);
		if (i % 2 != 0) {
			payload_pattern.pattern[j] <<= 8;
			payload_pattern.pattern[j] |= hexchar_to_int(str[i]);
			j++;
		}
		else
			payload_pattern.pattern[j] = hexchar_to_int(str[i]); 
	}

	payload_pattern.size = (str_size % 2 == 0) ? j : j + 1;
	return true;
}

void set_payload(void* buffer, size_t size)
{
	size_t i;

	i = 0;
	while (i < size) {
		i++;
	}
}

int main(int argc, char** argv)
{
	// for (int i = 0; argv[1][i]; i++)
	// 	printf("%d ", hexchar_to_int(argv[1][i]));
	// printf("\n");

	init_payload(argv[1]);

	printf("size: %d\n", payload_pattern.size);

	for (uint8_t i = 0; i < payload_pattern.size; i++) {
		printf("%x%x ", payload_pattern.pattern[i] >> 8, payload_pattern.pattern[i] & 0xf);
	}

	printf("\n");
}