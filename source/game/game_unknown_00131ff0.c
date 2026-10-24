/*
GAME_UNKNOWN_00131FF0.C

a part of the game: two things of 0x2AE0 bytes that share 0x452E8 bytes of
the console's memory, half each.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "cache/cache_files.h"

#include <stdint.h>
#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00131FF0_THING_SIZE= 0x2AE0,
	GAME_UNKNOWN_00131FF0_MEMORY_SIZE= 0x22974
};

/* ---------- structures */

/* what a thing is made with: six shorts, of which nothing is known */
struct game_unknown_00131ff0_parameters
{
	short unknown[6];
};

/* ---------- globals */

/* 0x00510c44: the memory. 0x00547f88 and 0x0054aa68: the two things */
byte *unknown_00510c44;
byte unknown_00547f88[GAME_UNKNOWN_00131FF0_THING_SIZE];
byte unknown_0054aa68[GAME_UNKNOWN_00131FF0_THING_SIZE];

/* ---------- prototypes */

void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

/* 0x00134980: a thing is made, with its six shorts and its memory.
0x00134b20: it is closed */
void unknown_00134980(const struct game_unknown_00131ff0_parameters *parameters, byte *memory, byte *thing);
void unknown_00134b20(byte *thing);

/* ---------- code */

/* 0x00131ff0: the first of the functions of the part (game.c): the memory
is taken from the console (VirtualAlloc; if there is none the error is asked
for, and it goes on all the same), and the two things are made */
void game_unknown_00131ff0_initialize(
	void)
{
	struct game_unknown_00131ff0_parameters first= {{1, 0x80, 0x40, 0x100, 0x180, 0}};
	struct game_unknown_00131ff0_parameters second= {{6, 0x40, 1, 0x80, 0x60, 0x100}};
	byte *memory= VirtualAlloc(NULL, 2*GAME_UNKNOWN_00131FF0_MEMORY_SIZE, 0x101000, 4);

	if (!memory)
	{
		GetLastError();
	}
	unknown_00510c44= memory;
	unknown_00134980(&first, memory, unknown_00547f88);
	unknown_00134980(&second, unknown_00510c44+GAME_UNKNOWN_00131FF0_MEMORY_SIZE, unknown_0054aa68);
}

/* 0x001320b0: the second: the two things are closed, and the memory is given
back, if there is any (VirtualFree; if that fails, the error is asked for) */
void game_unknown_00131ff0_dispose(
	void)
{
	unknown_00134b20(unknown_00547f88);
	unknown_00134b20(unknown_0054aa68);
	if (unknown_00510c44)
	{
		if (!VirtualFree(unknown_00510c44, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_00510c44= NULL;
	}
}

/* 0x001321f0: one of the 512 bits at 0x21C of a thing */
long __attribute__((stdcall)) game_unknown_00131ff0_test_bit(
	long bit,
	const byte *thing)
{
	return (((const unsigned long *)(thing+0x21C))[bit>>5]&(1UL<<(bit&0x1F)))!=0;
}

/* 0x00132b10: seven bits from 0x0200 to 0x8000, by seven bytes */
long __attribute__((stdcall)) game_unknown_00131ff0_flags(
	boolean flag4000,
	boolean flag8000,
	boolean flag0800,
	boolean flag1000,
	boolean flag2000,
	boolean flag0400,
	boolean flag0200)
{
	return (flag4000 ? 0x4000 : 0) | (flag2000 ? 0x2000 : 0) | (flag1000 ? 0x1000 : 0) | (flag0800 ? 0x800 : 0)
		| (flag0400 ? 0x400 : 0) | (flag0200 ? 0x200 : 0) | (flag8000 ? 0x8000 : 0);
}

/* 0x001332b0: room for so many bits in the longs at 0x25C of a thing (0x200
of them, how many are taken at 0xA5C); the first long if there is no more
room, and nothing for no bits */
unsigned long *game_unknown_00131ff0_allocate_bits(
	long count,
	byte *thing)
{
	unsigned short used;
	long longs;

	if (count<=0)
	{
		return NULL;
	}
	used= *(const unsigned short *)(thing+0xA5C);
	longs= (count+0x1F)>>5;
	if (used+longs>=0x200)
	{
		return (unsigned long *)(thing+0x25C);
	}
	*(unsigned short *)(thing+0xA5C)= (unsigned short)(used+longs);
	return (unsigned long *)(thing+0x25C)+used;
}

/* 0x001348e0: a thing is emptied: the shorts at 0x04 of the four things it
points to, its bits at 0x21C, the longs taken at 0x25C and the shorts at
0xA5C and 0xA5E are 0, and its 0x80 longs at 0x1C are NONE */
void game_unknown_00131ff0_clear(
	byte *thing)
{
	long index;

	for (index= 0; index<4; index++)
	{
		*(short *)(*(byte *const *)(thing+0x0C+index*4)+0x04)= 0;
	}
	memset(thing+0x21C, 0, 0x40);
	memset(thing+0x25C, 0, *(const unsigned short *)(thing+0xA5C)*4);
	*(short *)(thing+0xA5C)= 0;
	*(short *)(thing+0xA5E)= 0;
	memset(thing+0x1C, 0xFF, 0x80*4);
}

/* 0x00134300: a new entry (0x10 bytes from 0xA60, as many taken as at 0xA5E;
0x200 at most) of a thing gets only one of its 128 bits; its index, or 0
when there is no room */
long __attribute__((stdcall)) game_unknown_00131ff0_new_entry(
	byte *thing,
	short bit)
{
	unsigned short index= *(const unsigned short *)(thing+0xA5E);
	unsigned long *bits;

	*(unsigned short *)(thing+0xA5E)= (unsigned short)(index+1);
	if (index>=0x200)
	{
		*(unsigned short *)(thing+0xA5E)= 0x200;
		return 0;
	}
	bits= (unsigned long *)(thing+index*0x10+0xA60);
	bits[3]= 0;
	bits[2]= 0;
	bits[1]= 0;
	bits[0]= 0;
	bits[bit>>5]|= 1UL<<(bit&0x1F);
	return index;
}

/* 0x004b9ef0: a kind (3 is looked at). 0x004b9f8c: a tag, or NONE.
0x004b9ffc: a real. 0x004ba004: a byte */
long unknown_004b9ef0;
long unknown_004b9f8c;
float unknown_004b9ffc;
byte unknown_004ba004;

/* 0x001332f0: some flags from others (the bits 9, 11, 13 and 14 moved), with
0x01 when a thing has the bit 7 of its byte at 0x2A60; for the kind 3 also
0x100 when the tag of 0x004b9f8c has not the bit 0x02 of its first byte,
0x004b9ffc is 0.0 and the bit 10 is not set, and 0x80 when 0x004ba004 is not
set or the bit 10 is */
unsigned long __attribute__((stdcall)) game_unknown_00131ff0_flags_from(
	unsigned short flags,
	const byte *thing)
{
	unsigned long result= (((flags>>2)&0x2C00)|(flags&0x4000))>>9 | (flags&0x200)<<5;

	if ((signed char)thing[0x2A60]<0)
	{
		result|= 1;
	}
	if (unknown_004b9ef0==3)
	{
		if (unknown_004b9f8c!=NONE && unknown_004b9ffc==0.0f && !(flags&0x400)
			&& !(*(const byte *)(uintptr_t)global_tag_instances[unknown_004b9f8c&0xFFFF].address&2))
		{
			result|= 0x100;
		}
		if (!unknown_004ba004 || (flags&0x400))
		{
			result|= 0x80;
		}
	}

	return result;
}
