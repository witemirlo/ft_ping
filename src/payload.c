#include "ft_ping.h"

t_payload_pattern payload_pattern = {0};

static bool valid_hex_number(char const* const str)
{
	const char valid_chars[] = "0123456789abcdef";
	uint8_t    i;

	for (i = 0; i < 32 && str[i] != '\0'; i++) {
		if (strchr(valid_chars, str[i]) == NULL)
			return false;
	}

	return true;
}

bool init_payload(char const* const str)
{
	// 112233445566778899aabbccddeeff42 como maximo
	uint8_t i, j;

	if (!valid_hex_number(str))
		return NULL;

	for (i = 0, j = 0; i < 32 && str[i] != '\0'; i++) {
		if (i % 2 != 0) {
			payload_pattern.pattern[j] <<= 8;
			payload_pattern.pattern[j] |= (str[i] - '0');
			j++;
		}
		else
			payload_pattern.pattern[j] = str[i] - '0'; 
		// printf("char: %c : %x\n", str[i], payload_pattern.pattern[j]);
	}

	payload_pattern.size = j + 1;
	return true;
}

// void set_payload(void* buffer, size_t size)
// {

// }

int main(int argc, char** argv)
{
	init_payload(argv[1]);

	// printf("size: %d\n", payload_pattern.size);

	for (uint8_t i = 0; i < payload_pattern.size; i++) {
		printf("%x%x ", payload_pattern.pattern[i] >> 8, payload_pattern.pattern[i] & 0xf);
	}

	printf("\n");
}