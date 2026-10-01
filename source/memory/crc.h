/*
CRC.H

A checksum of 32 bits. As in Halo 1.
*/

#ifndef __CRC_H
#define __CRC_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	CRC_TABLE_SIZE= 256
};

/* what a checksum starts as */
#define CRC_NEW 0xFFFFFFFF

/* ---------- prototypes/CRC.C */

void crc_build_table(unsigned long *table);
void crc_checksum_buffer(const void *buffer, long size, unsigned long *crc_reference);

/* ---------- globals */

extern boolean crc_table_initialized;
extern unsigned long crc_table[CRC_TABLE_SIZE];

#endif
