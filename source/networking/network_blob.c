/*
NETWORK_BLOB.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A block of bytes that starts with a checksum of the rest, a version and its
size, and how it is seen to be good. The name of the file is a guess, and so
are the names of its functions.
*/

#include "network_blob.h"

#include "cache/texture_cache.h"
#include "memory/crc.h"
#include "networking/online_storage.h"

#include <string.h>

/* ---------- prototypes */

/* ---------- globals */

/* 0x0055e700: how many times something was tried again, or left, after it
could not be done */
long unknown_0055e700;

/* ---------- code */

/* 0x000812a0: the checksum of a block of a size: the one of all of it after
its first four bytes, which are where it is kept */
unsigned long network_blob_checksum(
	const struct network_blob_header *block,
	long size)
{
	unsigned long checksum= 0xFFFFFFFF;

	crc_checksum_buffer((const byte *)block+sizeof(block->checksum), size-(long)sizeof(block->checksum), &checksum);

	return checksum;
}

/* 0x00081230: a block of a size is seen to: 1 if it is good, and if not
why. It has to be of 0x10 bytes or more, and of the size that it says (10);
of the size of its kind, but that a block of the fourth kind can be shorter
(9); of the version of its kind, not older (11) nor newer (12); and its
checksum has to be the one that it has (8) */
long network_blob_verify(
	unsigned long size,
	const struct network_blob_header *block,
	const struct network_blob *blob)
{
	unsigned long version;

	if (size<NETWORK_BLOB_HEADER_SIZE || block->size!=size)
	{
		return _network_blob_bad_size;
	}
	if (size!=blob->size)
	{
		if (blob->kind!=_network_blob_kind4 ||
			size>blob->size)
		{
			return _network_blob_wrong_size;
		}
	}
	version= blob->functions->version;
	if (block->version<version)
	{
		return _network_blob_too_old;
	}
	if (block->version>version)
	{
		return _network_blob_too_new;
	}

	return network_blob_checksum(block, (long)size)==block->checksum ? _network_blob_good : _network_blob_bad_checksum;
}

/* 0x00080e10: what has a block takes memory for it from the cache of
textures, as much as the block can be, waiting for long if it has to
(texture_cache_steal_memory, which the game has inside); whether it could,
and where the memory is and how much. Not if the block could not be even its
first sixteen bytes, or it has memory already. The cache takes the memory
back with network_blob_memory_stolen */
boolean __attribute__((stdcall)) network_blob_take_memory(
	struct network_blob *blob,
	void **memory_out,
	unsigned long *size_out)
{
	void *memory;

	if ((unsigned long)blob->capacity<NETWORK_BLOB_HEADER_SIZE || blob->memory)
	{
		return FALSE;
	}
	memory= texture_cache_steal_memory(_texture_cache_steal_wait_long, blob->capacity, (long)blob, NULL,
		(texture_cache_stolen_memory_proc)network_blob_memory_stolen);
	blob->memory= memory;
	if (!memory)
	{
		return FALSE;
	}
	blob->size= (unsigned long)blob->capacity;
	if (memory_out)
	{
		*memory_out= memory;
	}
	if (size_out)
	{
		*size_out= (unsigned long)blob->capacity;
	}

	return TRUE;
}

/* 0x00080f30: how far the task of something is, from 0 to 1; whether it is
known, which it is not without a task. A task that has not started is at
nothing */
boolean network_blob_get_progress(
	const struct network_blob *blob,
	real *progress)
{
	long handle;

	if (blob->task==NONE)
	{
		return FALSE;
	}
	handle= unknown_004d8c28[blob->task].handle;
	if (handle!=NONE)
	{
		return online_storage_get_progress(handle, progress);
	}
	*progress= 0.0f;

	return TRUE;
}

/* 0x00080f70: something starts to read its block, if it is doing nothing;
whether it does, or has no need to: one of a kind whose block is good, and
was not made here, is not read again. What it is asked with is kept. While
it reads, the block is not good, but the one of the fourth kind stays as it
was */
boolean __attribute__((stdcall)) network_blob_start_read(
	struct network_blob *blob,
	long unknown10,
	struct network_blob_request request,
	word options)
{
	boolean has_kind;

	if (blob->state!=_network_blob_idle)
	{
		return FALSE;
	}
	has_kind= blob->kind!=_network_blob_kind_none;
	blob->unknown10= unknown10;
	blob->request= request;
	blob->options= options;
	if (has_kind && (blob->flags&_network_blob_good_bit) && !(blob->flags&_network_blob_made_bit))
	{
		return TRUE;
	}
	blob->task= network_blob_task_start_read(blob, blob->memory, blob->size);
	if (blob->task==NONE)
	{
		return FALSE;
	}
	blob->state= _network_blob_reading;
	if (blob->kind!=_network_blob_kind4)
	{
		blob->flags&= ~_network_blob_good_bit;
	}

	return TRUE;
}

/* 0x00080ff0: something starts to write its block, if it is doing nothing;
whether it does */
boolean __attribute__((stdcall)) network_blob_start_write(
	struct network_blob *blob,
	long unknown10,
	struct network_blob_request request,
	word options)
{
	if (blob->state!=_network_blob_idle)
	{
		return FALSE;
	}
	blob->unknown10= unknown10;
	blob->request= request;
	blob->options= options;
	blob->task= network_blob_task_start_write(blob, blob->memory, blob->size);
	if (blob->task==NONE)
	{
		return FALSE;
	}
	blob->state= _network_blob_writing;

	return TRUE;
}

/* 0x00081050: the task of something ended, in a way. It does nothing now,
and has no task.
- Well: a block that was written loses two bits, and one that was read is
  good and not made here.
- With 6 when it was read, it has its bit of 0x4.
- With 7, unless it was started with the bit that says not to, the same
  thing starts again.
Then its function for what it did is told, if it has one; if that says no,
its bits are as they were before, with the bit of 0x4 if it got it. If it
ended well and the function did not say no, that is all. If not, and it was
not started with the bit that says to do nothing: its fifth function makes
the block here, and if it does the block is good and made here; if it does
not, or there is no such function, what it did is started again, and the
count of unknown_0055e700 has one more */
void network_blob_task_finished(
	struct network_blob *blob,
	long result)
{
	word flags= blob->flags;
	boolean succeeded= FALSE;

	blob->last_state= blob->state;
	blob->result= (short)result;
	blob->state= _network_blob_idle;
	blob->task= NONE;
	if (result==_network_blob_task_done)
	{
		succeeded= TRUE;
		if (blob->last_state==_network_blob_writing)
		{
			blob->flags= flags&(word)~(_network_blob_unknown4_bit|_network_blob_unknown10_bit);
		}
		else if (blob->last_state==_network_blob_reading)
		{
			blob->flags= (flags&(word)~(_network_blob_unknown4_bit|_network_blob_made_bit|_network_blob_unknown10_bit))|
				_network_blob_good_bit;
		}
	}
	else if (blob->last_state==_network_blob_reading && result==_network_blob_task_unknown6)
	{
		blob->flags= flags|_network_blob_unknown4_bit;
	}
	else if (result==_network_blob_task_again && !(blob->options&_network_blob_option_once_bit))
	{
		if (blob->last_state==_network_blob_reading)
		{
			network_blob_start_read(blob, blob->unknown10, blob->request, blob->options);
		}
		else if (blob->last_state==_network_blob_writing)
		{
			network_blob_start_write(blob, blob->unknown10, blob->request, blob->options);
		}
	}
	if (blob->functions)
	{
		boolean (__attribute__((stdcall)) *told)(struct network_blob *blob)= NULL;
		boolean known= TRUE;

		flags|= blob->flags&_network_blob_unknown4_bit;
		if (blob->last_state==_network_blob_writing)
		{
			told= blob->functions->written;
		}
		else if (blob->last_state==_network_blob_reading)
		{
			told= blob->functions->read;
		}
		else
		{
			known= FALSE;
		}
		if (known && told && !told(blob))
		{
			blob->flags= flags;
			succeeded= FALSE;
		}
	}
	if (succeeded)
	{
		return;
	}
	if (blob->options&_network_blob_option_no_recovery_bit)
	{
		return;
	}
	if (blob->functions && blob->functions->make)
	{
		boolean made= blob->functions->make(blob);

		if (made)
		{
			blob->flags|= _network_blob_good_bit|_network_blob_made_bit;
		}
		else
		{
			blob->flags&= ~(_network_blob_good_bit|_network_blob_made_bit);
		}
		if (made)
		{
			return;
		}
	}
	unknown_0055e700++;
	if (blob->last_state==_network_blob_reading)
	{
		network_blob_start_read(blob, blob->unknown10, blob->request, blob->options);
	}
	else if (blob->last_state==_network_blob_writing)
	{
		network_blob_start_write(blob, blob->unknown10, blob->request, blob->options);
	}
}

/* 0x000812d0: the cache of textures takes back the memory that the block of
something has: its task, if it is of a kind and has one, is told so, and it
has no memory. What has the block is the second thing that it is given */
void __attribute__((stdcall)) network_blob_memory_stolen(
	long unknown,
	struct network_blob *blob)
{
	(void)unknown;
	if (blob->kind!=_network_blob_kind_none && blob->task!=NONE)
	{
		network_blob_task_end(blob->task, _network_blob_task_no_memory);
	}
	if (blob->memory)
	{
		texture_cache_return_memory(blob->memory);
		blob->memory= NULL;
		blob->size= 0;
	}
}
/* 0x00080de0: 16 bytes from a blob: its number at 0x4C and its four bytes
at 0x50, and zeros */
void network_blob_unknown_00080de0(
	byte *result,
	const byte *blob)
{
	memset(result, 0, 16);
	*(long *)result= *(const long *)(blob+0x4C);
	result[4]= blob[0x50];
	result[5]= blob[0x51];
	result[6]= blob[0x52];
	result[7]= blob[0x53];
}
