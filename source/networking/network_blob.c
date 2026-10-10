/*
NETWORK_BLOB.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A block of bytes that starts with a checksum of the rest, a version and its
size, and how it is seen to be good. The name of the file is a guess, and so
are the names of its functions.
*/

#include "network_blob.h"

#include "memory/crc.h"

/* ---------- code */

/* 0x000812a0: the checksum of a block of a size: the one of all of it after
its first four bytes, which are where it is kept */
unsigned long network_blob_checksum(
	const struct network_blob_header *blob,
	long size)
{
	unsigned long checksum= 0xFFFFFFFF;

	crc_checksum_buffer((const byte *)blob+sizeof(blob->checksum), size-(long)sizeof(blob->checksum), &checksum);

	return checksum;
}

/* 0x00081230: a block of a size is seen to: 1 if it is good, and if not
why. It has to be of 0x10 bytes or more, and of the size that it says (10);
of the size of its kind, but that a block of the fourth kind can be shorter
(9); of the version of its kind, not older (11) nor newer (12); and its
checksum has to be the one that it has (8) */
long network_blob_verify(
	unsigned long size,
	const struct network_blob_header *blob,
	const byte *definition)
{
	unsigned long definition_size;
	unsigned long version;

	if (size<NETWORK_BLOB_HEADER_SIZE || blob->size!=size)
	{
		return _network_blob_bad_size;
	}
	definition_size= *(const unsigned long *)(definition+NETWORK_BLOB_DEFINITION_SIZE_OFFSET);
	if (size!=definition_size)
	{
		if (*(const word *)(definition+NETWORK_BLOB_DEFINITION_KIND_OFFSET)!=_network_blob_definition_kind4 ||
			size>definition_size)
		{
			return _network_blob_wrong_size;
		}
	}
	version= *(const unsigned long *)(*(const byte *const *)definition+NETWORK_BLOB_DEFINITION_UNKNOWN0_VERSION_OFFSET);
	if (blob->version<version)
	{
		return _network_blob_too_old;
	}
	if (blob->version>version)
	{
		return _network_blob_too_new;
	}

	return network_blob_checksum(blob, (long)size)==blob->checksum ? _network_blob_good : _network_blob_bad_checksum;
}
