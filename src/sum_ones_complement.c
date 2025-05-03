#include <stdint.h>

uint16_t sum_ones_complement(uint16_t a, uint16_t b)
{
	uint32_t c = (uint32_t)a + (uint32_t)b;

        while(0xff0000 & c) {
                c = (c & 0xffff) + 1;
        }

        return c;
}
