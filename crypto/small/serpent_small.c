/*
    *
    * Copyright (c) 2010 
	* ntldr <ntldr@diskcryptor.net> PGP key ID - 0xC48251EB4F8E4E6E
    *

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "defines.h"
#include "serpent_small.h"

#define LTf(_b) { \
	_b[0] = ROL32(_b[0], 13);      \
	_b[2] = ROL32(_b[2], 3);       \
	_b[1] ^= _b[0] ^ _b[2];        \
	_b[3] ^= _b[2] ^ (_b[0] << 3); \
	_b[1] = ROL32(_b[1], 1);       \
	_b[3] = ROL32(_b[3], 7);       \
	_b[0] ^= _b[1] ^ _b[3];        \
	_b[2] ^= _b[3] ^ (_b[1] << 7); \
	_b[0] = ROL32(_b[0], 5);       \
	_b[2] = ROL32(_b[2], 22);      \
}

#define ITf(_b) { \
	_b[2] = ROR32(_b[2], 22);      \
	_b[0] = ROR32(_b[0], 5);       \
	_b[2] ^= _b[3] ^ (_b[1] << 7); \
	_b[0] ^= _b[1] ^ _b[3];        \
	_b[3] = ROR32(_b[3], 7);       \
	_b[1] = ROR32(_b[1], 1);       \
	_b[3] ^= _b[2] ^ (_b[0] << 3); \
	_b[1] ^= _b[0] ^ _b[2];        \
	_b[2] = ROR32(_b[2], 3);       \
	_b[0] = ROR32(_b[0], 13);      \
}

#define PHI 0x9E3779B9

typedef void (*sbox_p)(u32 b[4]);

static void sE1(u32 b[4])
{
	u32 t = b[1];
	b[3] ^= b[0]; 
	b[1] &= b[3];
	t ^= b[2];
	b[1] ^= b[0];
	b[0] |= b[3];
	b[0] ^= t;
	t ^= b[3];
	b[3] ^= b[2];
	b[2] |= b[1];
	b[2] ^= t;
	t = ~t;
	t |= b[1];
	b[1] ^= b[3];
	b[1] ^= t;
	b[3] |= b[0];
	b[1] ^= b[3];
	t ^= b[3];
	b[3] = b[0];
	b[0] = b[1];
	b[1] = t;
}

static void sE2(u32 b[4])
{
	u32 t;
	b[0] = ~b[0];
	b[2] = ~b[2];
	t = b[0];
	b[0] &= b[1];
	b[2] ^= b[0];
	b[0] |= b[3];
	b[3] ^= b[2];
	b[1] ^= b[0];
	b[0] ^= t;
	t |= b[1];
	b[1] ^= b[3];
	b[2] |= b[0];
	b[2] &= t;
	b[0] ^= b[1];
	b[1] &= b[2];
	b[1] ^= b[0];
	b[0] &= b[2];
	t ^= b[0];
	b[0] = b[2]; 
	b[2] = b[3]; 
	b[3] = b[1]; 
	b[1] = t;           
}

static void sE3(u32 b[4])
{
	u32 t = b[0];
    b[0] &= b[2];
    b[0] ^= b[3];
    b[2] ^= b[1];
    b[2] ^= b[0];
    b[3] |= t;
    b[3] ^= b[1];
    t ^= b[2];
    b[1] = b[3]; 
    b[3] |= t;
    b[3] ^= b[0];
    b[0] &= b[1];
    t ^= b[0];
    b[1] ^= b[3];
    b[1] ^= t;
    b[0] = b[2]; 
    b[2] = b[1];
    b[1] = b[3];
	b[3] = ~t;
}

static void sE4(u32 b[4])
{
	u32 t = b[0];
    b[0] |= b[3];
    b[3] ^= b[1];
    b[1] &= t;
    t ^= b[2];
    b[2] ^= b[3];
    b[3] &= b[0];
    t |= b[1];
    b[3] ^= t;
    b[0] ^= b[1];
    t &= b[0];
    b[1] ^= b[3];
    t ^= b[2];
    b[1] |= b[0];
    b[1] ^= b[2];
    b[0] ^= b[3];
    b[2] = b[1]; 
    b[1] |= b[3];
    b[0] ^= b[1];
    b[1] = b[2]; 
    b[2] = b[3]; 
    b[3] = t;
}

static void sE5(u32 b[4])
{
	u32 t;
	b[1] ^= b[3];
    b[3] = ~b[3];
    b[2] ^= b[3];
    b[3] ^= b[0];
    t = b[1];
    b[1] &= b[3];
    b[1] ^= b[2];
    t ^= b[3];
    b[0] ^= t;
    b[2] &= t;
    b[2] ^= b[0];
    b[0] &= b[1];
    b[3] ^= b[0];
    t |= b[1];
    t ^= b[0];
    b[0] |= b[3];
    b[0] ^= b[2];
    b[2] &= b[3];
    b[0] = ~b[0];
    t ^= b[2];
    b[2] = b[0]; 
    b[0] = b[1]; 
    b[1] = t; 
}

static void sE6(u32 b[4])
{
	u32 t;
	b[0] ^= b[1];
    b[1] ^= b[3];
    b[3] = ~b[3];
    t = b[1];
    b[1] &= b[0];
    b[2] ^= b[3];
    b[1] ^= b[2];
    b[2] |= t;
    t ^= b[3];
    b[3] &= b[1];
    b[3] ^= b[0];
    t ^= b[1];
    t ^= b[2];
    b[2] ^= b[0];
    b[0] &= b[3];
    b[2] = ~b[2];
    b[0] ^= t;
    t |= b[3];
    t ^= b[2];
    b[2] = b[0]; 
    b[0] = b[1]; 
    b[1] = b[3]; 
    b[3] = t;
}

static void sE7(u32 b[4])
{
	u32 t = b[3];
	b[2] = ~b[2];    
    b[3] &= b[0];
    b[0] ^= t;
    b[3] ^= b[2];
    b[2] |= t;
    b[1] ^= b[3];
    b[2] ^= b[0];
    b[0] |= b[1];
    b[2] ^= b[1];
    t ^= b[0];
    b[0] |= b[3];
    b[0] ^= b[2];
    t ^= b[3];
    t ^= b[0];
    b[3] = ~b[3];
    b[2] &= t;
    b[3] ^= b[2];
    b[2] = t;
}

static void sE8(u32 b[4])
{
	u32 t = b[1];
    b[1] |= b[2];
    b[1] ^= b[3];
    t ^= b[2];
    b[2] ^= b[1];
    b[3] |= t;
    b[3] &= b[0];
    t ^= b[2];
    b[3] ^= b[1];
    b[1] |= t;
    b[1] ^= b[0];
    b[0] |= t;
    b[0] ^= b[2];
    b[1] ^= t;
    b[2] ^= b[1];
    b[1] &= b[0];
    b[1] ^= t;
    b[2] = ~b[2];
    b[2] |= b[0];
    t ^= b[2];
    b[2] = b[1]; 
    b[1] = b[3]; 
    b[3] = b[0]; 
    b[0] = t;
}

static void sD1(u32 b[4])
{
	u32 t = b[1];
	b[2] = ~b[2];
    b[1] |= b[0];
    t = ~t;
    b[1] ^= b[2];
    b[2] |= t;
    b[1] ^= b[3];
    b[0] ^= t;
    b[2] ^= b[0];
    b[0] &= b[3];
    t ^= b[0];
    b[0] |= b[1];
    b[0] ^= b[2];
    b[3] ^= t;
    b[2] ^= b[1];
    b[3] ^= b[0];
    b[3] ^= b[1];
    b[2] &= b[3];
    t ^= b[2];
    b[2] = b[1]; 
    b[1] = t;
}

static void sD2(u32 b[4])
{
	u32 t = b[1];
    b[1] ^= b[3];
    b[3] &= b[1];
    t ^= b[2];
    b[3] ^= b[0];
    b[0] |= b[1];
    b[2] ^= b[3];
    b[0] ^= t;
    b[0] |= b[2];
    b[1] ^= b[3];
    b[0] ^= b[1];
    b[1] |= b[3];
    b[1] ^= b[0];
    t = ~t;
    t ^= b[1];
    b[1] |= b[0];
    b[1] ^= b[0];
    b[1] |= t;
    b[3] ^= b[1];
    b[1] = b[0]; 
    b[0] = t; 
    t = b[2]; 
    b[2] = b[3]; 
    b[3] = t;
}

static void sD3(u32 b[4])
{
	u32 t;
	b[2] ^= b[3];
    b[3] ^= b[0];
    t = b[3];
    b[3] &= b[2];
    b[3] ^= b[1];
    b[1] |= b[2];
    b[1] ^= t;
    t &= b[3];
    b[2] ^= b[3];
    t &= b[0];
    t ^= b[2];
    b[2] &= b[1];
    b[2] |= b[0];
    b[3] = ~b[3];
    b[2] ^= b[3];
    b[0] ^= b[3];
    b[0] &= b[1];
    b[3] ^= t;
    b[3] ^= b[0];
    b[0] = b[1]; 
    b[1] = t;
}

static void sD4(u32 b[4])
{
	u32 t = b[2];
    b[2] ^= b[1];
    b[0] ^= b[2];
    t &= b[2];
    t ^= b[0];
    b[0] &= b[1];
    b[1] ^= b[3];
    b[3] |= t;
    b[2] ^= b[3];
    b[0] ^= b[3];
    b[1] ^= t;
    b[3] &= b[2];
    b[3] ^= b[1];
    b[1] ^= b[0];
    b[1] |= b[2];
    b[0] ^= b[3];
    b[1] ^= t;
    b[0] ^= b[1];
    t = b[0]; 
    b[0] = b[2]; 
    b[2] = b[3]; 
    b[3] = t;
}

static void sD5(u32 b[4])
{
	u32 t = b[2];
    b[2] &= b[3];
    b[2] ^= b[1];
    b[1] |= b[3];
    b[1] &= b[0];
    t ^= b[2];
    t ^= b[1];
    b[1] &= b[2];
    b[0] = ~b[0];
    b[3] ^= t;
    b[1] ^= b[3];
    b[3] &= b[0];
    b[3] ^= b[2];
    b[0] ^= b[1];
    b[2] &= b[0];
    b[3] ^= b[0];
    b[2] ^= t;
    b[2] |= b[3];
    b[3] ^= b[0];
    b[2] ^= b[1];
    b[1] = b[3]; 
    b[3] = t;
}

static void sD6(u32 b[4])
{
	u32 t = b[3];
	b[1] = ~b[1];    
    b[2] ^= b[1];
    b[3] |= b[0];
    b[3] ^= b[2];
    b[2] |= b[1];
    b[2] &= b[0];
    t ^= b[3];
    b[2] ^= t;
    t |= b[0];
    t ^= b[1];
    b[1] &= b[2];
    b[1] ^= b[3];
    t ^= b[2];
    b[3] &= t;
    t ^= b[1];
    b[3] ^= t;
    t = ~t;
    b[3] ^= b[0];
    b[0] = b[1]; 
    b[1] = t; 
    t = b[3]; 
    b[3] = b[2]; 
    b[2] = t;
}

static void sD7(u32 b[4])
{
	u32 t = b[2];
	b[0] ^= b[2];    
    b[2] &= b[0];
    t ^= b[3];
    b[2] = ~b[2];
    b[3] ^= b[1];
    b[2] ^= b[3];
    t |= b[0];
    b[0] ^= b[2];
    b[3] ^= t;
    t ^= b[1];
    b[1] &= b[3];
    b[1] ^= b[0];
    b[0] ^= b[3];
    b[0] |= b[2];
    b[3] ^= b[1];
    t ^= b[0];
    b[0] = b[1]; 
    b[1] = b[2]; 
    b[2] = t;
}

static void sD8(u32 b[4])
{
	u32 t = b[2];
    b[2] ^= b[0];
    b[0] &= b[3];
    t |= b[3];
    b[2] = ~b[2];
    b[3] ^= b[1];
    b[1] |= b[0];
    b[0] ^= b[2];
    b[2] &= t;
    b[3] &= t;
    b[1] ^= b[2];
    b[2] ^= b[0];
    b[0] |= b[2];
    t ^= b[1];
    b[0] ^= b[3];
    b[3] ^= t;
    t |= b[0];
    b[3] ^= b[2];
    t ^= b[2];
    b[2] = b[1]; 
    b[1] = b[0]; 
    b[0] = b[3]; 
    b[3] = t;
}

static void __declspec(noinline) KXf(u32 *keys, u32 round, u32 b[4])
{
	b[0] ^= keys[4*round  ];
	b[1] ^= keys[4*round+1];
	b[2] ^= keys[4*round+2];
	b[3] ^= keys[4*round+3];
}

static sbox_p se_tab[] = { sE1, sE2, sE3, sE4, sE5, sE6, sE7, sE8 };
static sbox_p sd_tab[] = { sD1, sD2, sD3, sD4, sD5, sD6, sD7, sD8 };

void serpent256_set_key(const unsigned char *key, serpent256_key *skey)
{
	u32 W[140], i;

	mincpy(W, key, SERPENT_KEY_SIZE);

	for(i = 8; i != 140; ++i) {
		W[i] = ROL32(W[i-8] ^ W[i-5] ^ W[i-3] ^ W[i-1] ^ PHI ^ (i-8), 11);
	}
	for (i = 8; i <= 136; i += 4) {
		se_tab[7 - (((i / 4) + 2) % 8)](&W[i]);
	}
	mincpy(skey->expkey, W+8, SERPENT_EXPKEY_WORDS*sizeof(u32));
}

void serpent256_encrypt(const unsigned char *in, unsigned char *out, serpent256_key *key)
{
	u32 b[4];
	int i;
	
	autocpy(b, in, SERPENT_BLOCK_SIZE);

	for (i = 0; i < 32; i++) {
		KXf(key->expkey, i, b); se_tab[i % 8](b);	
		if (i == 31) KXf(key->expkey, 32, b); else LTf(b);
	}
	autocpy(out, b, SERPENT_BLOCK_SIZE);
}

void serpent256_decrypt(const unsigned char *in, unsigned char *out, serpent256_key *key)
{
	u32 b[4]; 
	int i;
	
	autocpy(b, in, SERPENT_BLOCK_SIZE);

	for (i = 31; i >= 0; i--) {
		if (i == 31) KXf(key->expkey, 32, b); else ITf(b); 
		sd_tab[i % 8](b); KXf(key->expkey, i, b);
	}
	autocpy(out, b, SERPENT_BLOCK_SIZE);
}