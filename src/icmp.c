#include "ft_ping.h"
#include <stdint.h>

static u_int16_t sum_ones_complement(u_int16_t a, u_int16_t b)
{
	u_int32_t c = (u_int32_t)a + (u_int32_t)b;

        while(0xff0000 & c)
                c = (c & 0xffff) + 1;

        return c;
}

static void icmp_timestamp(struct icmp* const icmp)
{
        static uint16_t       seq = 0;
	static struct timeval tv = {0};

	if (gettimeofday(&tv, NULL) < 0) {
		// TODO: errores, quizas retornar algo para que main limpie
		return;
	}

	icmp->icmp_otime = htonl(tv.tv_sec);
	icmp->icmp_rtime = htonl(tv.tv_usec);
	icmp->icmp_seq = htons(seq);

        seq++;
}

uint16_t icmp_checksum(struct icmp const* const icmp, void const* const payload, size_t payload_size)
{
	uint16_t checksum;

	checksum = sum_ones_complement(icmp->icmp_type, icmp->icmp_code);
	checksum = sum_ones_complement(checksum, icmp->icmp_id);
	checksum = sum_ones_complement(checksum, icmp->icmp_seq);

	checksum = sum_ones_complement(checksum, (icmp->icmp_otime >> 16));
	checksum = sum_ones_complement(checksum, (icmp->icmp_otime & 0xffff));

	checksum = sum_ones_complement(checksum, (icmp->icmp_rtime >> 16));
	checksum = sum_ones_complement(checksum, (icmp->icmp_rtime & 0xffff));

	for (size_t i = 0; i < payload_size / 2; i++) {
		checksum = sum_ones_complement(checksum, ((uint16_t*)payload)[i]);
	}

	checksum = 0xffff - checksum;

	return checksum;
}

void update_icmp(struct icmp* const icmp, void const* const payload, size_t payload_size)
{
	icmp_timestamp(icmp);
	icmp->icmp_cksum = icmp_checksum(icmp, payload, payload_size);
}

void init_icmp(struct icmp* const icmp)
{
        memset(icmp, 0, sizeof(*icmp));

        icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = config.id;
}
