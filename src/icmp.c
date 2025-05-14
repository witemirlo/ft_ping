#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ft_ping.h"

static u_int16_t sum_ones_complement(u_int16_t a, u_int16_t b)
{
	u_int32_t c = (u_int32_t)a + (u_int32_t)b;

        while(0xff0000 & c)
                c = (c & 0xffff) + 1;

        return c;
}

void init_icmp(struct icmp* const icmp)
{
        int n;

        memset(icmp, 0, sizeof(icmp));

        srand(time(NULL));
        n = rand();

        icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_hun.ih_idseq.icd_id = n; // TODO: en linux es un numero random
}

void update_icmp_seq(struct icmp* const icmp)
{
        static short seq = 1;

	icmp->icmp_hun.ih_idseq.icd_seq = htons(seq);
        seq++;
}

void update_icmp_checksum(struct icmp* const icmp)
{
	icmp->icmp_cksum = sum_ones_complement(icmp->icmp_type, icmp->icmp_code);
	icmp->icmp_cksum = sum_ones_complement(icmp->icmp_cksum, icmp->icmp_hun.ih_idseq.icd_id);
	icmp->icmp_cksum = sum_ones_complement(icmp->icmp_cksum, icmp->icmp_hun.ih_idseq.icd_seq);

	icmp->icmp_cksum = 0xffff - icmp->icmp_cksum;
}