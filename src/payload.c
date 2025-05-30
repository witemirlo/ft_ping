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
	uint8_t i, j, str_size;

	if (!valid_hex_number(str))
		return false;

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

	payload_pattern.size = (str_size % 2 == 0) ? j : j + 1; // TODO: los calculos bien
	fprintf(stderr, "%s:%d: \"%s\" (%d) -> %d\n", __FILE__, __LINE__, str, str_size, j); // TODO: BORRAR
	
	return true;
}

void set_payload(void* buffer, size_t size)
{
	// TODO: no hace lo que toca
	size_t i;

	for (i = 0; i < size; i++) {
		// TODO: probablemente tiene que hacerse operaciones de bits
		((uint8_t*)buffer)[i] = payload_pattern.pattern[i % payload_pattern.size];
	}
}

// TODO: borrar
int main(int argc, char** argv)
{
	char buffer[41];

	init_payload(argv[1]);

	printf("size: %d\n", payload_pattern.size);
	for (size_t i = 0; i < payload_pattern.size; i++)
		printf("%x%x ", payload_pattern.pattern[i] >> 4, payload_pattern.pattern[i] & 0xf);
	printf("\n");

	// for (uint8_t i = 0; i < payload_pattern.size; i++) {
	// 	printf("%x ", payload_pattern.pattern[i]);
	// }
	// printf("\n");

	// set_payload(buffer, sizeof(buffer));

	// for (size_t i = 0; i < sizeof(buffer); i++) {
	// 	if (i % 2 == 0)
	// 		printf("%x", buffer[i]);
	// 	else
	// 		printf("%x ", buffer[i]);
	// }
	printf("\n");

}