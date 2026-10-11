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
#include "memory/data.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0006b640: what does a task is told to stop */
void unknown_0006b640(long handle);

/* 0x00080e10, of network_blob.c: something takes the memory of its block;
whether it could, and where it is and how much. 0x000b4670, 0x000b42b0: a
block starts to be written, one of the third kind and one of the others;
0x000b4810, 0x000b4450: and to be read. Each gives what does it, and says
nothing if it started and why not if it did not. 0x000b45f0: how what read a
block ended, with how much it read and how much there was */
boolean __attribute__((stdcall)) unknown_00080e10(struct network_blob *blob, void **memory_out,
	unsigned long *size_out);
long __attribute__((stdcall)) unknown_000b4670(long unknown0, long unknown10, struct network_blob_request request,
	long unknown4, long unknownC, long *handle_out);
long __attribute__((stdcall)) unknown_000b42b0(long unknown0, long *handle_out, long unknown10,
	struct network_blob_request request, long unknown4, void *memory, unsigned long size);
long __attribute__((stdcall)) unknown_000b4810(long unknown0, long *handle_out, long unknown10,
	struct network_blob_request request, long unknown4, long unknownC);
long __attribute__((stdcall)) unknown_000b4450(long unknown0, long unknown10, struct network_blob_request request,
	long unknown4, void *memory, unsigned long size, long *handle_out);
long __attribute__((stdcall)) unknown_000b45f0(long handle, unsigned long *read_out, unsigned long *size_out);

/* ---------- globals */

/* 0x004d8c28: the tasks. 0x004d8ba8: the two lists of them, each with the
ones that it has first and NONE after them */
struct network_blob_task unknown_004d8c28[MAXIMUM_NETWORK_BLOB_TASKS];
long unknown_004d8ba8[NUMBER_OF_NETWORK_BLOB_TASK_QUEUES][MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT];

/* 0x004402a4: how many tasks of each list can go on at once: one.
0x004cf78c: the things of what does the tasks */
long unknown_004402a4[NUMBER_OF_NETWORK_BLOB_TASK_QUEUES]= {1, 1};
struct data_array *unknown_004cf78c;

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

/* 0x0008e240: the tasks that wait are started, in each list from the first
and while fewer go on than can. The ones that write and the ones that read
start in their own way, and a block of the third kind in another; one of the
fourth kind takes its memory first (unknown_00080e10), and if it cannot, no
more of the list are started now. A task that cannot start ends
(network_blob_task_end), with what says why, and no more of its list are
started now either */
void network_blob_tasks_update(
	void)
{
	long queue;

	for (queue= 0; queue<NUMBER_OF_NETWORK_BLOB_TASK_QUEUES; queue++)
	{
		long limit= unknown_004402a4[queue];
		long *list= unknown_004d8ba8[queue];
		long active= 0;
		boolean stopped= FALSE;
		long index;

		for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT; index++)
		{
			if (list[index]!=NONE && unknown_004d8c28[list[index]].handle!=NONE)
			{
				active++;
			}
		}
		for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT && active<limit && !stopped; index++)
		{
			long task= list[index];
			struct network_blob_task *entry;
			struct network_blob *blob;
			const struct network_blob_functions *functions;
			long handle= NONE;
			long status;
			long result;

			if (task==NONE)
			{
				continue;
			}
			entry= &unknown_004d8c28[task];
			if (entry->handle!=NONE)
			{
				continue;
			}
			blob= entry->blob;
			functions= blob->functions;
			if (queue==_network_blob_task_queue_write)
			{
				if (blob->kind==_network_blob_kind3)
				{
					status= unknown_000b4670(functions->unknown0[0], blob->unknown10, blob->request, functions->unknown0[1],
						functions->unknownC, &handle);
				}
				else
				{
					status= unknown_000b42b0(functions->unknown0[0], &handle, blob->unknown10, blob->request,
						functions->unknown0[1], entry->memory, entry->size);
				}
			}
			else
			{
				if (blob->kind==_network_blob_kind3)
				{
					status= unknown_000b4810(functions->unknown0[0], &handle, blob->unknown10, blob->request,
						functions->unknown0[1], functions->unknownC);
				}
				else
				{
					if (blob->kind==_network_blob_kind4 && !unknown_00080e10(blob, &entry->memory, &entry->size))
					{
						stopped= TRUE;
						continue;
					}
					status= unknown_000b4450(functions->unknown0[0], blob->unknown10, blob->request, functions->unknown0[1],
						entry->memory, entry->size, &handle);
				}
			}
			if (status==0)
			{
				active++;
				entry->handle= handle;
				continue;
			}
			switch (status)
			{
			case 2:
			case 3:
			case 4:
			case 5:
				result= status+1;
				break;
			default:
				result= _network_blob_task_failed;
				break;
			}
			network_blob_task_end(task, result);
			if (index<MAXIMUM_NETWORK_BLOB_TASK_QUEUE_COUNT-1)
			{
				index--;
			}
			stopped= TRUE;
		}
	}
}

/* 0x0008e5e0: what did a task ended, and says how: the task ends
(network_blob_task_end) with what that is for it. If it ended well, a block
that was not read ends well; one that was read has to be no more than there
is room for (9) and all there (7), and ends with what looking at it says
(network_blob_verify). If it did not, what it says from 2 to 6 is one more
for the task, and anything else is 2. The game does not look at whether a
task has what ended */
void __attribute__((stdcall)) network_blob_task_completed(
	long status,
	long handle)
{
	struct network_blob_task *entry;
	unsigned long read= 0;
	unsigned long size= 0;
	long task= NONE;
	long kind;
	long result;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASKS; index++)
	{
		if (unknown_004d8c28[index].blob && unknown_004d8c28[index].handle==handle)
		{
			task= index;
			break;
		}
	}
	entry= &unknown_004d8c28[task];
	kind= *(const long *)((const byte *)unknown_004cf78c->data+(entry->handle&0xFFFF)*NETWORK_BLOB_OPERATION_SIZE+
		NETWORK_BLOB_OPERATION_KIND_OFFSET);
	if (status==0 && kind==_network_blob_operation_read)
	{
		status= unknown_000b45f0(handle, &read, &size);
	}
	switch (status)
	{
	case 0:
		if (kind!=_network_blob_operation_read)
		{
			result= _network_blob_task_done;
		}
		else if (read>entry->size)
		{
			result= _network_blob_task_too_big;
		}
		else if (read!=size)
		{
			result= _network_blob_task_again;
		}
		else
		{
			result= network_blob_verify(size, (const struct network_blob_header *)entry->memory, entry->blob);
		}
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		result= status+1;
		break;
	default:
		result= _network_blob_task_failed;
		break;
	}
	network_blob_task_end(task, result);
}
