/*
NETWORK_BLOB_TASK.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The tasks that read and write the blocks of network_blob.c: there can be 32,
and each is in one of two lists, of the ones that write and of the ones that
read, from when it is asked for to when it ends. The name of the file is a
guess, and so are the names of its functions.
*/

#include "network_blob.h"

#include "memory/crc.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0006b640: what does a task is told to stop */
void unknown_0006b640(long handle);

/* ---------- globals */

/* 0x004d8c28: the tasks. 0x004d8ba8: the two lists of them, each with the
ones that it has first and NONE after them */
struct network_blob_task unknown_004d8c28[MAXIMUM_NETWORK_BLOB_TASKS];
long unknown_004d8ba8[NUMBER_OF_NETWORK_BLOB_TASK_QUEUES][MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT];

/* ---------- code */

/* 0x0008e0f0: a task ends, in a way: what does it is told to stop, if it
had started; what has the block is told (network_blob_task_finished); the
block of the fourth kind lets go of its memory, if it has no task now; and
the task is free and in no list: what is after it in its list moves up */
void __attribute__((stdcall)) network_blob_task_end(
	long task,
	long result)
{
	struct network_blob_task *entry= &unknown_004d8c28[task];
	long queue;
	long index;

	if (!entry->blob)
	{
		return;
	}
	if (entry->handle!=NONE)
	{
		unknown_0006b640(entry->handle);
	}
	network_blob_task_finished(entry->blob, result);
	if (entry->blob->kind==_network_blob_kind4 && entry->blob->task==NONE)
	{
		network_blob_memory_stolen(0, entry->blob);
	}
	entry->blob= NULL;
	entry->handle= NONE;
	entry->size= 0;
	entry->memory= NULL;
	for (queue= 0; queue<NUMBER_OF_NETWORK_BLOB_TASK_QUEUES; queue++)
	{
		for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT; index++)
		{
			if (unknown_004d8ba8[queue][index]==task)
			{
				unknown_004d8ba8[queue][index]= NONE;
			}
			if (index>0 && unknown_004d8ba8[queue][index-1]==NONE)
			{
				unknown_004d8ba8[queue][index-1]= unknown_004d8ba8[queue][index];
				unknown_004d8ba8[queue][index]= NONE;
			}
		}
	}
}

/* 0x0008e1e0: the first task that is free, or NONE */
long network_blob_task_find_free(
	void)
{
	long task= NONE;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASKS && task==NONE; index++)
	{
		if (!unknown_004d8c28[index].blob)
		{
			task= index;
		}
	}

	return task;
}

/* 0x0008e210: no task is in a list, and all are free */
void network_blob_tasks_reset(
	void)
{
	long index;

	memset(unknown_004d8ba8, 0xFF, sizeof(unknown_004d8ba8));
	for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASKS; index++)
	{
		unknown_004d8c28[index].blob= NULL;
		unknown_004d8c28[index].handle= NONE;
		unknown_004d8c28[index].size= 0;
		unknown_004d8c28[index].memory= NULL;
	}
}

/* the first place of a list that has no task, or NONE */
static inline __attribute__((always_inline)) long network_blob_task_queue_find_free(
	long queue)
{
	long place= NONE;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT && place==NONE; index++)
	{
		if (unknown_004d8ba8[queue][index]==NONE)
		{
			place= index;
		}
	}

	return place;
}

/* 0x0008e500: a task is to write the block of something, of a size, from
memory; which task. A block of the first or the second kind gets its
checksum first: the one of all of it, with nothing where it is kept. The
game does not look at whether there is a task and a place in the list */
long __attribute__((stdcall)) network_blob_task_start_write(
	struct network_blob *blob,
	void *memory,
	unsigned long size)
{
	long task= network_blob_task_find_free();
	long place= network_blob_task_queue_find_free(_network_blob_task_queue_write);

	if (blob->kind==_network_blob_kind1 || blob->kind==_network_blob_kind2)
	{
		*(unsigned long *)memory= 0xFFFFFFFF;
		crc_checksum_buffer(memory, (long)size, (unsigned long *)memory);
	}
	unknown_004d8c28[task].size= size;
	unknown_004d8c28[task].memory= memory;
	unknown_004d8c28[task].blob= blob;
	unknown_004d8c28[task].handle= NONE;
	unknown_004d8ba8[_network_blob_task_queue_write][place]= task;

	return task;
}

/* 0x0008e580: a task is to read the block of something, of a size, into
memory; which task */
long __attribute__((stdcall)) network_blob_task_start_read(
	struct network_blob *blob,
	void *memory,
	unsigned long size)
{
	long task= network_blob_task_find_free();
	long place= network_blob_task_queue_find_free(_network_blob_task_queue_read);

	unknown_004d8c28[task].handle= NONE;
	unknown_004d8c28[task].size= size;
	unknown_004d8c28[task].blob= blob;
	unknown_004d8c28[task].memory= memory;
	unknown_004d8ba8[_network_blob_task_queue_read][place]= task;

	return task;
}
