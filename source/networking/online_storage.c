/*
ONLINE_STORAGE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The storage of the service of games by the network. The name of the file is
a guess, and so are the names of its functions.
*/

#include "online_storage.h"

#include "networking/online_tasks.h"

/* ---------- prototypes */

/* 0x000b4120: the name that something has in the storage, from what it is
asked with; whether it could be made, and how long it is */
boolean __attribute__((stdcall)) unknown_000b4120(long unknown0, struct network_blob_request request, long unknown4,
	word *path, long *length);

/* 0x003ad176, of the console's libraries: something starts to be written in
the storage. 0x003ab6ea: and to be read from it. 0x003ab6f9: how reading
something from it went. 0x003ad1a6: how far something of the storage is, in
hundredths */
long __attribute__((stdcall)) unknown_003ad176(long facility, long owner, const word *path, long unknownC, long unknown10,
	void *memory, unsigned long size, long unknown1C, long unknown20, long *handle_out);
long __attribute__((stdcall)) unknown_003ab6ea(long facility, long owner, const word *path, void *memory,
	unsigned long size, long unknown14, long unknown18, long *handle_out);
long __attribute__((stdcall)) unknown_003ab6f9(long handle, long *unknown4, unsigned long *size_out,
	unsigned long *read_out, long *unknown10, long *unknown14);
long __attribute__((stdcall)) XOnlineStorageGetProgress(long handle, unsigned long *percent, long *unknown8,
	long *unknownC);

/* ---------- code */

/* 0x000b42b0: a block starts to be written in the storage, from memory, by
a new task whose number is given; what starting says. It cannot without a
name for it (unknown_000b4120) or without a task, which there is only if
someone is signed in. If the libraries do not start, the task is done with,
and what they say is one of the ways that it could not */
long __attribute__((stdcall)) online_storage_write_start(
	long unknown0,
	long *task_out,
	long owner,
	struct network_blob_request request,
	long unknown4,
	void *memory,
	unsigned long size)
{
	word path[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	long length= MAXIMUM_ONLINE_STORAGE_PATH_LENGTH;
	struct online_task *task;
	long facility;
	long result;
	long index;
	long hr;

	*task_out= NONE;
	switch (unknown0)
	{
	case 0:
		facility= _online_storage_facility4;
		break;
	case 1:
		facility= _online_storage_facility5;
		break;
	default:
		facility= _online_storage_facility3;
		break;
	}
	if (!unknown_000b4120(unknown0, request, unknown4, path, &length))
	{
		return _online_storage_failed;
	}
	index= online_task_new_signed_in();
	task= online_task_get(index);
	if (!task)
	{
		return _online_storage_failed;
	}
	hr= unknown_003ad176(facility, owner, path, 0, 0, memory, size, 0, 0, &task->handle);
	if (hr>=0)
	{
		*task_out= index;
		task->flags= _online_task_started_bits;
		task->type= _online_task_type_storage_write;
		task->owner= owner;

		return _online_storage_started;
	}
	switch ((unsigned long)hr)
	{
	case 0x8007000E:
	case 0x80150002:
	case 0x80150003:
	case 0x80150005:
	case 0x80150006:
	case 0x80150008:
		result= _online_storage_unknown2;
		break;
	case 0x80150004:
	case 0x8015C002:
		result= _online_storage_unknown3;
		break;
	case 0x8015C006:
	case 0x8015C008:
	case 0x8015C009:
		result= _online_storage_unknown4;
		break;
	default:
		result= _online_storage_failed;
		break;
	}
	online_task_close(index);

	return result;
}

/* 0x000b4450: a block starts to be read from the storage, into memory, by a
new task whose number is given; what starting says. As
online_storage_write_start, but what the libraries say when they do not
start is not the same for all that they say */
long __attribute__((stdcall)) online_storage_read_start(
	long unknown0,
	long owner,
	struct network_blob_request request,
	long unknown4,
	void *memory,
	unsigned long size,
	long *task_out)
{
	word path[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	long length= MAXIMUM_ONLINE_STORAGE_PATH_LENGTH;
	struct online_task *task;
	long facility;
	long result;
	long index;
	long hr;

	*task_out= NONE;
	switch (unknown0)
	{
	case 0:
		facility= _online_storage_facility4;
		break;
	case 1:
		facility= _online_storage_facility5;
		break;
	default:
		facility= _online_storage_facility3;
		break;
	}
	if (!unknown_000b4120(unknown0, request, unknown4, path, &length))
	{
		return _online_storage_failed;
	}
	index= online_task_new_signed_in();
	task= online_task_get(index);
	if (!task)
	{
		return _online_storage_failed;
	}
	hr= unknown_003ab6ea(facility, owner, path, memory, size, 0, 0, &task->handle);
	if (hr>=0)
	{
		*task_out= index;
		task->flags= _online_task_started_bits;
		task->type= _online_task_type_storage_read;
		task->owner= owner;

		return _online_storage_started;
	}
	switch ((unsigned long)hr)
	{
	case 0x8007000E:
	case 0x80150002:
	case 0x80150003:
	case 0x80150005:
	case 0x80150006:
	case 0x80150008:
		result= _online_storage_unknown2;
		break;
	case 0x80150004:
	case 0x8015C002:
		result= _online_storage_unknown3;
		break;
	case 0x8015C006:
	case 0x8015C008:
	case 0x8015C009:
		result= _online_storage_unknown4;
		break;
	case 0x8015C004:
	case 0x8015C007:
		result= _online_storage_unknown5;
		break;
	default:
		result= _online_storage_failed;
		break;
	}
	online_task_close(index);

	return result;
}

/* 0x000b45f0: how reading a block from the storage went, of its task: how
much was read and how much there was. If the libraries cannot say, the task
has its bit of 0x20. It always says nothing itself, and does not look at
whether there is such a task */
long __attribute__((stdcall)) online_storage_read_get_results(
	long index,
	unsigned long *read_out,
	unsigned long *size_out)
{
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, index);
	long unknown4= 0;
	long unknown10;
	long unknown14;

	if (unknown_003ab6f9(task->handle, &unknown4, size_out, read_out, &unknown10, &unknown14)<0)
	{
		task->flags|= _online_task_unknown20_bit;
	}

	return 0;
}

/* 0x000b49a0: how far a task of the storage is, from 0 to 1; whether it is
known */
boolean online_storage_get_progress(
	long index,
	real *progress)
{
	const struct online_task *task;
	unsigned long percent;
	long double value;

	*progress= 0.0f;
	task= (const struct online_task *)datum_try_and_get(online_tasks, index);
	if (!task || XOnlineStorageGetProgress(task->handle, &percent, NULL, NULL)<0)
	{
		return FALSE;
	}
	/* as the game's processor does it: the number as one with a sign, and
	then what a number without one has more */
	value= (long double)(long)percent;
	if ((long)percent<0)
	{
		value+= (long double)4294967296.0f;
	}
	*progress= (real)(value*(long double)0.01f);

	return TRUE;
}
