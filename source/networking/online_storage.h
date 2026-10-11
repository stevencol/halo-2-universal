/*
ONLINE_STORAGE.H

The storage of the service of games by the network: where the blocks of
network_blob.c are kept, and read from and written to by online tasks. The
name of the file is a guess, and so are the names of its functions.
*/

#ifndef __ONLINE_STORAGE_H
#define __ONLINE_STORAGE_H

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "networking/network_blob.h"

/* ---------- constants */

enum
{
	/* what starting to read or write says: it started; it could not; and
	three ways that the service says why not */
	_online_storage_started= 0,
	_online_storage_failed,
	_online_storage_unknown2,
	_online_storage_unknown3,
	_online_storage_unknown4,
	/* which part of the storage something is in, from the first number of
	the functions of what has a block: the fourth for 0, the fifth for 1,
	and the third for the others */
	_online_storage_facility3= 3,
	_online_storage_facility4= 4,
	_online_storage_facility5= 5,
	/* how long the name of something in the storage can be, in characters;
	the kind of the task that writes; and the bits that it starts with */
	MAXIMUM_ONLINE_STORAGE_PATH_LENGTH= 0x100,
	_online_task_type_storage_write= 0x29,
	_online_task_started_bits= 0x1
};

/* ---------- prototypes/ONLINE_STORAGE.C */

long __attribute__((stdcall)) online_storage_write_start(long unknown0, long *task_out, long owner,
	struct network_blob_request request, long unknown4, void *memory, unsigned long size);
long __attribute__((stdcall)) online_storage_read_get_results(long index, unsigned long *read_out,
	unsigned long *size_out);
boolean online_storage_get_progress(long index, real *progress);

#endif
