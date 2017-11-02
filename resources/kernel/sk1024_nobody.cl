//Nobody GPL
#ifndef SK1024_CL
#define SK1024_CL

#define xor3(a, b, c) (a ^ b ^ c)
#define xor5(a, b, c, d, e) (a ^ b ^ c ^ d ^ e)

#pragma OPENCL EXTENSION cl_amd_media_ops : enable

inline static unsigned long ROL64(const unsigned long a, const int b)
{
	uint2 aa = as_uint2(a);
	if (b <= 32)
		return as_ulong(amd_bitalign((aa).xy, (aa).yx, 32 - b));
	else
		return as_ulong(amd_bitalign((aa).yx, (aa).xy, 64 - b));
}

__constant static const ulong keccakf_1600_rc[24] = 
{
    0x0000000000000001UL, 0x0000000000008082UL,
    0x800000000000808AUL, 0x8000000080008000UL,
    0x000000000000808BUL, 0x0000000080000001UL,
    0x8000000080008081UL, 0x8000000000008009UL,
    0x000000000000008AUL, 0x0000000000000088UL,
    0x0000000080008009UL, 0x000000008000000AUL,
    0x000000008000808BUL, 0x800000000000008BUL,
    0x8000000000008089UL, 0x8000000000008003UL,
    0x8000000000008002UL, 0x8000000000000080UL,
    0x000000000000800AUL, 0x800000008000000AUL,
    0x8000000080008081UL, 0x8000000000008080UL,
    0x0000000080000001UL, 0x8000000080008008UL
};

inline void Keccak1600(ulong *s)
{
	ulong m[5], v, w;
	
	#pragma unroll
	for(int i = 0; i < 24; ++i)
	{	
		m[0] = xor5(s[0], s[5], s[10], s[15], s[20]) ^ ROL64(xor5(s[2], s[7], s[12], s[17], s[22]), 1UL);
		m[1] = xor5(s[1], s[6], s[11], s[16], s[21]) ^ ROL64(xor5(s[3], s[8], s[13], s[18], s[23]), 1UL);
		m[2] = xor5(s[2], s[7], s[12], s[17], s[22]) ^ ROL64(xor5(s[4], s[9], s[14], s[19], s[24]), 1UL);
		m[3] = xor5(s[3], s[8], s[13], s[18], s[23]) ^ ROL64(xor5(s[0], s[5], s[10], s[15], s[20]), 1UL);
		m[4] = xor5(s[4], s[9], s[14], s[19], s[24]) ^ ROL64(xor5(s[1], s[6], s[11], s[16], s[21]), 1UL);
		
		v = s[1] ^ m[0];
		s[0] ^= m[4];
		s[1] =  ROL64(s[6] ^ m[0], 44UL);
		s[6] =  ROL64(s[9] ^ m[3], 20UL);
		s[9] =  ROL64(s[22] ^ m[1], 61UL);
		s[22] = ROL64(s[14] ^ m[3], 39UL);
		s[14] = ROL64(s[20] ^ m[4], 18UL);
		s[20] = ROL64(s[2] ^ m[1], 62UL);
		s[2] =  ROL64(s[12] ^ m[1], 43UL);
		s[12] = ROL64(s[13] ^ m[2], 25UL);
		s[13] = ROL64(s[19] ^ m[3], 8UL);
		s[19] = ROL64(s[23] ^ m[2], 56UL);
		s[23] = ROL64(s[15] ^ m[4], 41UL);
		s[15] = ROL64(s[4] ^ m[3], 27UL);
		s[4] =  ROL64(s[24] ^ m[3], 14UL);
		s[24] = ROL64(s[21] ^ m[0], 2UL);
		s[21] = ROL64(s[8] ^ m[2], 55UL);
		s[8] =  ROL64(s[16] ^ m[0], 45UL);
		s[16] = ROL64(s[5] ^ m[4], 36UL);
		s[5] =  ROL64(s[3] ^ m[2], 28UL);
		s[3] =  ROL64(s[18] ^ m[2], 21UL);
		s[18] = ROL64(s[17] ^ m[1], 15UL);
		s[17] = ROL64(s[11] ^ m[0], 10UL);
		s[11] = ROL64(s[7] ^ m[1], 6UL);
		s[7] =  ROL64(s[10] ^ m[4], 3UL);
		s[10] = ROL64(v, 1UL);

		#pragma unroll 2
		for (uint j=0; j<5; j++)
		{
			v = s[(j * 5) + 0];
			w = s[(j * 5) + 1];

			s[(j * 5) + 0] = bitselect(s[(j * 5) + 0] ^ s[(j * 5) + 2], s[(j * 5) + 0], s[(j * 5) + 1]);
			s[(j * 5) + 1] = bitselect(s[(j * 5) + 1] ^ s[(j * 5) + 3], s[(j * 5) + 1], s[(j * 5) + 2]);
			s[(j * 5) + 2] = bitselect(s[(j * 5) + 2] ^ s[(j * 5) + 4], s[(j * 5) + 2], s[(j * 5) + 3]);
			s[(j * 5) + 3] = bitselect(s[(j * 5) + 3] ^ v, s[(j * 5) + 3], s[(j * 5) + 4]);
			s[(j * 5) + 4] = bitselect(s[(j * 5) + 4] ^ w, s[(j * 5) + 4], v);
		}
		
		s[0] ^= keccakf_1600_rc[i];
	}
};

#define Round1024_1(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pA, pB, pC, pD, pE, pF) \
{ \
	p0 += p1; \
	p2 += p3; \
	p4 += p5; \
	p6 += p7; \
	p8 += p9; \
	p10 += p11; \
	p12 += p13; \
	p14 += p15; \
	p1 = ROL64(p1, 55) ^ p0; \
	p3 = ROL64(p3, 43) ^ p2; \
	p5 = ROL64(p5, 37) ^ p4; \
	p7 = ROL64(p7, 40) ^ p6; \
	p9 = ROL64(p9, 16) ^ p8; \
	p11 = ROL64(p11, 22) ^ p10; \
	p13 = ROL64(p13, 38) ^ p12; \
	p15 = ROL64(p15, 12) ^ p14; \
	p0 += p9; \
	p2 += p13; \
	p6 += p11; \
	p4 += p15; \
	p10 += p7; \
	p12 += p3; \
	p14 += p5; \
	p8 += p1; \
	p9 = ROL64(p9, 25) ^ p0; \
	p13 = ROL64(p13, 25) ^ p2; \
	p11 = ROL64(p11, 46) ^ p6; \
	p15 = ROL64(p15, 13) ^ p4; \
	p7 = ROL64(p7, 14) ^ p10; \
	p3 = ROL64(p3, 13) ^ p12; \
	p5 = ROL64(p5, 52) ^ p14; \
	p1 = ROL64(p1, 57) ^ p8; \
	p0 += p7; \
	p2 += p5; \
	p4 += p3; \
	p6 += p1; \
	p12 += p15; \
	p14 += p13; \
	p8 += p11; \
	p10 += p9; \
	p7 = ROL64(p7, 33) ^ p0; \
	p5 = ROL64(p5, 8) ^ p2; \
	p3 = ROL64(p3, 18) ^ p4; \
	p1 = ROL64(p1, 57) ^ p6; \
	p15 = ROL64(p15, 21) ^ p12; \
	p13 = ROL64(p13, 12) ^ p14; \
	p11 = ROL64(p11, 32) ^ p8; \
	p9 = ROL64(p9, 54) ^ p10; \
	p0 += p15; \
	p2 += p11; \
	p6 += p13; \
	p4 += p9; \
	p14 += p1; \
	p8 += p5; \
	p10 += p3; \
	p12 += p7; \
	p15 = ROL64(p15, 34) ^ p0; \
	p11 = ROL64(p11, 43) ^ p2; \
	p13 = ROL64(p13, 25) ^ p6; \
	p9 = ROL64(p9, 60) ^ p4; \
	p1 = ROL64(p1, 44) ^ p14; \
	p5 = ROL64(p5, 9) ^ p8; \
	p3 = ROL64(p3, 59) ^ p10; \
	p7 = ROL64(p7, 34) ^ p12; \
}

#define Round1024_2(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pA, pB, pC, pD, pE, pF) \
{ \
	p0 += p1; \
	p2 += p3; \
	p4 += p5; \
	p6 += p7; \
	p8 += p9; \
	p10 += p11; \
	p12 += p13; \
	p14 += p15; \
	p1 =  ROL64(p1, 28) ^ p0; \
	p3 =  ROL64(p3, 7) ^ p2; \
	p5 =  ROL64(p5, 47) ^ p4; \
	p7 =  ROL64(p7, 48) ^ p6; \
	p9 =  ROL64(p9, 51) ^ p8; \
	p11 = ROL64(p11, 9) ^ p10; \
	p13 = ROL64(p13, 35) ^ p12; \
	p15 = ROL64(p15, 41) ^ p14; \
	p0 += p9; \
	p2 += p13; \
	p6 += p11; \
	p4 += p15; \
	p10 += p7; \
	p12 += p3; \
	p14 += p5; \
	p8 += p1; \
	p9 =  ROL64(p9, 17) ^ p0; \
	p13 = ROL64(p13, 6) ^ p2; \
	p11 = ROL64(p11, 18) ^ p6; \
	p15 = ROL64(p15, 25) ^ p4; \
	p7 =  ROL64(p7, 43) ^ p10; \
	p3 =  ROL64(p3, 42) ^ p12; \
	p5 =  ROL64(p5, 40) ^ p14; \
	p1 =  ROL64(p1, 15) ^ p8; \
	p0 += p7; \
	p2 += p5; \
	p4 += p3; \
	p6 += p1; \
	p12 += p15; \
	p14 += p13; \
	p8 += p11; \
	p10 += p9; \
	p7 =  ROL64(p7, 58) ^ p0; \
	p5 =  ROL64(p5, 7) ^ p2; \
	p3 =  ROL64(p3, 32) ^ p4; \
	p1 =  ROL64(p1, 45) ^ p6; \
	p15 = ROL64(p15, 19) ^ p12; \
	p13 = ROL64(p13, 18) ^ p14; \
	p11 = ROL64(p11, 2) ^ p8; \
	p9 =  ROL64(p9, 56) ^ p10; \
	p0 += p15; \
	p2 += p11; \
	p6 += p13; \
	p4 += p9; \
	p14 += p1; \
	p8 += p5; \
	p10 += p3; \
	p12 += p7; \
	p15 = ROL64(p15, 47) ^ p0; \
	p11 = ROL64(p11, 49) ^ p2; \
	p13 = ROL64(p13, 27) ^ p6; \
	p9 =  ROL64(p9, 58) ^ p4; \
	p1 =  ROL64(p1, 37) ^ p14; \
	p5 =  ROL64(p5, 48) ^ p8; \
	p3 =  ROL64(p3, 53) ^ p10; \
	p7 =  ROL64(p7, 56) ^ p12; \
}

#define Round1024_a(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pA, pB, pC, pD, pE, pF, b, i, j) \
{ \
	p0 += b[(i + 0 + j) % 17]; \
	p1 += b[(i + 1 + j) % 17]; \
	p2 += b[(i + 2 + j) % 17]; \
	p3 += b[(i + 3 + j) % 17]; \
	p4 += b[(i + 4 + j) % 17]; \
	p5 += b[(i + 5 + j) % 17]; \
	p6 += b[(i + 6 + j) % 17]; \
	p7 += b[(i + 7 + j) % 17]; \
	p8 += b[(i + 8 + j) % 17]; \
	p9 += b[(i + 9 + j) % 17]; \
	p10 += b[(i + 10 + j) % 17]; \
	p11 += b[(i + 11 + j) % 17]; \
	p12 += b[(i + 12 + j) % 17]; \
	p13 += b[(i + 13 + j) % 17] + t[(i + j) % 3]; \
	p14 += b[(i + 14 + j) % 17] + t[(i + 1 + j) % 3]; \
	p15 += b[(i + 15 + j) % 17] + (ulong)(i + j); \
}

inline void Skein1024(ulong *p, ulong *t, ulong *b) {

	ulong p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15;

	p0 = p[0]; p1 = p[1]; p2 = p[2]; p3 = p[3]; p4 = p[4]; p5 = p[5]; p6 = p[6]; p7 = p[7];
	p8 = p[8]; p9 = p[9]; p10 = p[10]; p11 = p[11]; p12 = p[12]; p13 = p[13]; p14 = p[14]; p15 = p[15];

	#pragma unroll 1
	for (int i = 1; i < 21; i += 2)
	{
		Round1024_1(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
		Round1024_a(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, b, i, 0);

		Round1024_2(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
		Round1024_a(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, b, i, 1);
	}

	p[0] = p0; p[1] = p1; p[2] = p2; p[3] = p3; p[4] = p4; p[5] = p5; p[6] = p6; p[7] = p7;
	p[8] = p8; p[9] = p9; p[10] = p10; p[11] = p11; p[12] = p12; p[13] = p13; p[14] = p14; p[15] = p15;
}

__attribute__((reqd_work_group_size(WORKSIZE, 1, 1)))
__kernel void sk1024(__global ulong* uMessage, __constant ulong* c_hv, const ulong startNonce, __global ulong* hash, const ulong target)
{
	//RX 470
	//Driver 17.10.2
	//Skein (2 rounds) = 184 MH/s
	//Keccak (3 rounds) = 119 MH/s

	ulong nonce = startNonce + (ulong)get_global_id(0);
	ulong h[17];
	ulong t[3];
	ulong p[16];

	for (int i = 0; i<10; i++) p[i] = uMessage[i + 16] + c_hv[i];

	p[10] = nonce + c_hv[10];

	t[0] = 0xd8;
	t[1] = 0xb000000000000000;
	t[2] = 0xb0000000000000d8;

	p[11] = c_hv[11];
	p[12] = c_hv[12];
	p[13] = c_hv[13] + t[0];
	p[14] = c_hv[14] + t[1];
	p[15] = c_hv[15];

	#pragma unroll
	for (int i = 0; i < 17; i++) h[i] = c_hv[i];

	Skein1024(p, t, h);

	for (int i = 0; i < 10; i++) p[i] ^= uMessage[i + 16];

	p[10] ^= nonce;
	h[16] = 0x5555555555555555ULL;

	for (int i = 0; i < 16; i += 2) {
		h[i] = p[i];
		h[i + 1] = p[i + 1];
		h[16] = xor3(h[16], h[i], h[i + 1]);
	}

	t[0] = 0x08;
	t[1] = 0xff00000000000000;
	t[2] = 0xff00000000000008;

	p[13] += t[0];
	p[14] += t[1];

	Skein1024(p, t, h);

	ulong state[25];

	for(int i = 0; i < 9; ++i) state[i] = p[i];

	for (int i = 9; i<25; i++) state[i] = 0;

	Keccak1600(&state);

	for (int i = 0; i<7; i++) state[i] ^= p[i+9];

	state[7] ^= (0x05);
	state[8] ^= (1ULL << 63);

	Keccak1600(&state);
	Keccak1600(&state);

	barrier(CLK_GLOBAL_MEM_FENCE);

	if (state[6] <= target) {hash[0] = nonce;}
};

#endif //SK1024_CL
