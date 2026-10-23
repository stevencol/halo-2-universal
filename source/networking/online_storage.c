/*
ONLINE_STORAGE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The storage of the service of games by the network. The name of the file is
a guess, and so are the names of its functions.
*/

#include "online_storage.h"

#include "networking/online_tasks.h"

#include <stddef.h>

/* ---------- prototypes */

/* 0x003207ed, of the console's libraries: a text is copied, up to so many
characters. 0x003ab6b1: the name that something has in the storage, from the
part that it is in, whose it is and its own name; less than nothing if it
could not be made */
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);
long __attribute__((stdcall)) unknown_003ab6b1(long facility, long unknown4, long unknown8, unsigned long xuid_low,
	unsigned long xuid_high, const wchar *name, word *path, long *length);

/* 0x003ad176, of the console's libraries: something starts to be written in
the storage. 0x003ab6ea: and to be read from it. 0x003ab6f9: how reading
something from it went. 0x003ad1a6: how far something of the storage is, in
hundredths. 0x003ad149, 0x003ab6db: something starts to be written from
what the game says of a block of the third kind, and to be read to it */
long __attribute__((stdcall)) unknown_003ad176(long facility, long owner, const word *path, long unknownC, long unknown10,
	void *memory, unsigned long size, long unknown1C, long unknown20, long *handle_out);
long __attribute__((stdcall)) unknown_003ab6ea(long facility, long owner, const word *path, void *memory,
	unsigned long size, long unknown14, long unknown18, long *handle_out);
long __attribute__((stdcall)) unknown_003ad149(long facility, long owner, const word *path, long unknownC,
	long unknown10, long unknown14, long unknown18, long unknown1C, long *handle_out);
long __attribute__((stdcall)) XOnlineStorageDownload(long facility, long owner, const word *path, long unknownC,
	long unknown10, long unknown14, long *handle_out);
long __attribute__((stdcall)) unknown_003ab6f9(long handle, long *unknown4, unsigned long *size_out,
	unsigned long *read_out, long *unknown10, long *unknown14);
long __attribute__((stdcall)) XOnlineStorageGetProgress(long handle, unsigned long *percent, long *unknown8,
	long *unknownC);

/* ---------- globals */

/* 0x0050944e, of online_tasks.c: the console has one of two addresses.
0x004672dc: what the names of what is kept for all the players start with on
such a console */
extern boolean unknown_0050944e;
const wchar *unknown_004672dc= (const wchar *)u"test";

/* ---------- code */

/* which part of the storage something is in. The game has this in each
function that starts something */
static inline __attribute__((always_inline)) long online_storage_facility(
	long unknown0)
{
	switch (unknown0)
	{
	case 0:
		return _online_storage_facility4;
	case 1:
		return _online_storage_facility5;
	}

	return _online_storage_facility3;
}

/* what starting says, from what the libraries say when they do not start.
The game has this in each function that starts something, but the one that
writes from memory, which has its own */
static inline __attribute__((always_inline)) long online_storage_start_result(
	long hr)
{
	switch ((unsigned long)hr)
	{
	case 0x8007000E:
	case 0x80150002:
	case 0x80150003:
	case 0x80150005:
	case 0x80150006:
	case 0x80150008:
		return _online_storage_unknown2;
	case 0x80150004:
	case 0x8015C002:
		return _online_storage_unknown3;
	case 0x8015C006:
	case 0x8015C008:
	case 0x8015C009:
		return _online_storage_unknown4;
	case 0x8015C004:
	case 0x8015C007:
		return _online_storage_unknown5;
	}

	return _online_storage_failed;
}

/* 0x000b4120: the name that something has in the storage, from what it is
asked with and its own name; whether it could be made, and how long it is.
What is kept for all the players (the fourth part) has the number of the
build of the game before its name, or "test" on a console with one of two
addresses (online_tasks_check_address), so that each build has its own. What
is a player's own (the third part) is asked for with what the player is
known by; the rest, with two numbers */
boolean __attribute__((stdcall)) online_storage_make_path(
	long unknown0,
	struct network_blob_request request,
	const wchar *name,
	word *path,
	long *length)
{
	wchar prefix[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	wchar full_name[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	long facility= online_storage_facility(unknown0);
	unsigned long xuid_low= 0;
	unsigned long xuid_high= 0;
	long unknown4= 0;
	long unknown8= 0;
	long prefix_length;

	if (facility==_online_storage_facility4)
	{
		if (!unknown_0050944e)
		{
			usnzprintf(prefix, MAXIMUM_ONLINE_STORAGE_PATH_LENGTH, (const wchar *)u"%d", ONLINE_STORAGE_BUILD_NUMBER);
		}
		else
		{
			wcsncpy(prefix, unknown_004672dc, MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1);
			prefix[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1]= 0;
		}
	}
	else
	{
		wcsncpy(prefix, (const wchar *)u"", MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1);
		prefix[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1]= 0;
	}
	for (prefix_length= 0; prefix_length<MAXIMUM_ONLINE_STORAGE_PATH_LENGTH && prefix[prefix_length]; prefix_length++)
	{
	}
	if (prefix_length>0)
	{
		usnzprintf(full_name, MAXIMUM_ONLINE_STORAGE_PATH_LENGTH, (const wchar *)u"%s/%s", prefix, name);
	}
	else
	{
		wcsncpy(full_name, name, MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1);
		full_name[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH-1]= 0;
	}
	if (facility==_online_storage_facility3)
	{
		xuid_low= (unsigned long)request.unknown0[0];
		xuid_high= (unsigned long)request.unknown0[1];
	}
	else
	{
		unknown4= request.unknown0[0];
		unknown8= request.unknown0[1];
	}

	return unknown_003ab6b1(facility, unknown4, unknown8, xuid_low, xuid_high, full_name, path, length)>=0;
}

/* 0x000b42b0: a block starts to be written in the storage, from memory, by
a new task whose number is given; what starting says. It cannot without a
name for it (online_storage_make_path) or without a task, which there is only if
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
	if (!online_storage_make_path(unknown0, request, (const wchar *)unknown4, path, &length))
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
	if (!online_storage_make_path(unknown0, request, (const wchar *)unknown4, path, &length))
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
	result= online_storage_start_result(hr);
	online_task_close(index);

	return result;
}

/* 0x000b4670: a block of the third kind starts to be written to the storage,
by a new task whose number is given; what starting says. It has no memory:
the libraries are given a number of its functions instead */
long __attribute__((stdcall)) online_storage_write_file_start(
	long unknown0,
	long owner,
	struct network_blob_request request,
	long unknown4,
	long unknownC,
	long *task_out)
{
	word path[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	long length= MAXIMUM_ONLINE_STORAGE_PATH_LENGTH;
	struct online_task *task;
	long facility= online_storage_facility(unknown0);
	long result;
	long index;
	long hr;

	*task_out= NONE;
	if (!online_storage_make_path(unknown0, request, (const wchar *)unknown4, path, &length))
	{
		return _online_storage_failed;
	}
	index= online_task_new_signed_in();
	task= online_task_get(index);
	if (!task)
	{
		return _online_storage_failed;
	}
	hr= unknown_003ad149(facility, owner, path, 0, 0, unknownC, 0, 0, &task->handle);
	if (hr>=0)
	{
		*task_out= index;
		task->flags= _online_task_started_bits;
		task->type= _online_task_type_storage_file;
		task->owner= owner;

		return _online_storage_started;
	}
	result= online_storage_start_result(hr);
	online_task_close(index);

	return result;
}

/* 0x000b4810: and starts to be read from it */
long __attribute__((stdcall)) online_storage_read_file_start(
	long unknown0,
	long *task_out,
	long owner,
	struct network_blob_request request,
	long unknown4,
	long unknownC)
{
	word path[MAXIMUM_ONLINE_STORAGE_PATH_LENGTH];
	long length= MAXIMUM_ONLINE_STORAGE_PATH_LENGTH;
	struct online_task *task;
	long facility= online_storage_facility(unknown0);
	long result;
	long index;
	long hr;

	*task_out= NONE;
	if (!online_storage_make_path(unknown0, request, (const wchar *)unknown4, path, &length))
	{
		return _online_storage_failed;
	}
	index= online_task_new_signed_in();
	task= online_task_get(index);
	if (!task)
	{
		return _online_storage_failed;
	}
	hr= XOnlineStorageDownload(facility, owner, path, unknownC, 0, 0, &task->handle);
	if (hr>=0)
	{
		*task_out= index;
		task->flags= _online_task_started_bits;
		task->type= _online_task_type_storage_file;
		task->owner= owner;

		return _online_storage_started;
	}
	result= online_storage_start_result(hr);
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

/* ---------- two helpers of the service of games that are after these */

enum
{
	NUMBER_OF_ONLINE_ERROR_MESSAGES= 40,
	/* what is said for an error that is not in the table */
	ONLINE_ERROR_MESSAGE_DEFAULT= 0x6F
};

struct online_error_message
{
	unsigned long error;
	long message;
};

/* 0x004672e0: what is said for each error of the service of games (a number
of the interface, it seems) */
const struct online_error_message unknown_004672e0[NUMBER_OF_ONLINE_ERROR_MESSAGES]=
{
	{0x80152100, 0x5E}, {0x80152101, 0x5F}, {0x80152102, 0x60}, {0x80152103, 0x61},
	{0x80152104, 0x62}, {0x80152105, 0x63}, {0x80152106, 0x64}, {0x80152108, 0x65},
	{0x80152109, 0x66}, {0x8015210A, 0x67}, {0x8015210B, 0x68}, {0x8015210C, 0x5D},
	{0x8015210D, 0x69}, {0x8015210E, 0x6A}, {0x8015210F, 0x6B}, {0x80152110, 0x6C},
	{0x80152111, 0x5C}, {0x80152112, 0x6D}, {0x80152002, 0x8F}, {0x80152003, 0x90},
	{0x80152004, 0x91}, {0x80152006, 0x92}, {0x80152007, 0x93}, {0x80152008, 0x94},
	{0x80152009, 0xB6}, {0x8015200A, 0x95}, {0x80151000, 0x23}, {0x80151001, 0x23},
	{0x80151002, 0x24}, {0x80151003, 0x25}, {0x80151004, 0x38}, {0x80151005, 0x26},
	{0x80151006, 0x9A}, {0x80151101, 0x9B}, {0x001512F0, 0x28}, {0x80151200, 0x27},
	{0x8015B004, 0x9E}, {0x8015B005, 0x9F}, {0x8015B006, 0xA0}, {0x80151300, 0x9B}
};

/* 0x000b4a20: what is said for an error of the service of games */
long online_error_get_message(
	unsigned long error)
{
	long index;

	for (index= 0; index<NUMBER_OF_ONLINE_ERROR_MESSAGES; index++)
	{
		if (unknown_004672e0[index].error==error)
		{
			return unknown_004672e0[index].message;
		}
	}

	return ONLINE_ERROR_MESSAGE_DEFAULT;
}

/* 0x000b4a40: seven numbers of 64 bits as seven of 32 (the low half of
each) */
void online_copy_low_halves(
	unsigned long *destination,
	const unsigned long *source)
{
	long index;

	for (index= 0; index<7; index++)
	{
		destination[index]= 0;
	}
	for (index= 0; index<7; index++)
	{
		destination[index]= source[2*index];
	}
}
