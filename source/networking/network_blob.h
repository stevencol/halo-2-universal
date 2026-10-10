/*
NETWORK_BLOB.H

A block of bytes that starts with a checksum of the rest, a version and its
size, and how it is seen to be good. It is just before what the network takes
when a map is loaded; what the blocks are is not known yet. The name of the
file is a guess, and so are the names of its functions.
*/

#ifndef __NETWORK_BLOB_H
#define __NETWORK_BLOB_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* what seeing to a block says: it is good; its checksum is another; its
	size is not the one of its kind; it is too short, or its size is not the
	one that it says; its version is an older one, or a newer one */
	_network_blob_good= 1,
	_network_blob_bad_checksum= 8,
	_network_blob_wrong_size= 9,
	_network_blob_bad_size= 10,
	_network_blob_too_old= 11,
	_network_blob_too_new= 12,
	NETWORK_BLOB_HEADER_SIZE= 0x10,
	/* of what says what a block is to be: something first, with the version
	at 0x8; its kind, of which the fourth can be shorter than its size; and
	its size */
	NETWORK_BLOB_DEFINITION_KIND_OFFSET= 0x8,
	_network_blob_definition_kind4= 4,
	NETWORK_BLOB_DEFINITION_SIZE_OFFSET= 0x28,
	NETWORK_BLOB_DEFINITION_UNKNOWN0_VERSION_OFFSET= 0x8
};

/* ---------- structures */

struct network_blob_header
{
	unsigned long checksum;
	unsigned long version;
	unsigned long size;
};

/* ---------- prototypes/NETWORK_BLOB.C */

unsigned long network_blob_checksum(const struct network_blob_header *blob, long size);
long network_blob_verify(unsigned long size, const struct network_blob_header *blob, const byte *definition);

#endif
