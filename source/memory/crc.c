/*
CRC.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's crc.c.
*/

#include "crc.h"

/* ---------- constants */

#define CRC_POLYNOMIAL 0xEDB88320

/* ---------- globals */

/* 0x0055e755 */
boolean crc_table_initialized;

/* 0x0055e788 */
unsigned long crc_table[CRC_TABLE_SIZE];

/* ---------- code */

/* 0x00163c00 */
void crc_build_table(
	unsigned long *table)
{
	unsigned long index;

	for (index= 0; index<CRC_TABLE_SIZE; index++)
	{
		unsigned long value= index;
		long bit;

		for (bit= 8; bit>0; bit--)
		{
			if (value&1)
			{
				value= (value>>1)^CRC_POLYNOMIAL;
			}
			else
			{
				value>>= 1;
			}
		}
		table[index]= value;
	}
}

/* 0x00163ba0: goes on with a checksum, with the bytes of a buffer. In Halo 1
the checksum is the first argument */
void crc_checksum_buffer(
	const void *buffer,
	long size,
	unsigned long *crc_reference)
{
	const byte *bytes= buffer;
	unsigned long crc;

	if (!crc_table_initialized)
	{
		crc_build_table(crc_table);
		crc_table_initialized= TRUE;
	}

	crc= *crc_reference;
	for (; size>0; size--)
	{
		crc= (crc>>8)^crc_table[(*bytes++^crc)&0xFF];
	}
	*crc_reference= crc;
}
