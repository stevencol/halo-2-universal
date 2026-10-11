/*
ONLINE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The tasks of the service of games by the network. The names of the functions
are guesses.
*/

#include "online_tasks.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0006bfa0: something of the tasks is looked at when they start.
0x0006b950: all of them are done with. 0x0006c670: a task goes on a little;
what the libraries say of it, which is less than nothing if it could not.
0x0008c550: a task of the kind 0x21 is told that it is done with */
void unknown_0006bfa0(void);
void unknown_0006b950(void);
long unknown_0006c670(struct online_task *task);
void unknown_0008c550(long index);

/* 0x003a07b9, 0x003a062b, of the console's libraries: the list of friends
is done with, and so is a task */
long __attribute__((stdcall)) XOnlineFriendsEnumerateFinish(long handle);
long __attribute__((stdcall)) XOnlineTaskClose(long handle);

/* ---------- globals */

/* 0x004cf78c: the table of the tasks. 0x004771c8: something of them, and
0x00479748: a task of its own, or NONE */
struct data_array *online_tasks;
byte unknown_004771c8[ONLINE_TASKS_UNKNOWN_004771C8_SIZE];
long unknown_00479748;

/* ---------- code */

/* 0x0006b380: what goes over the tasks goes to the next one; whether there
is one */
boolean online_task_iterator_next(
	struct online_task_iterator *iterator)
{
	iterator->task= (struct online_task *)data_iterator_next(&iterator->iterator);

	return iterator->task!=NULL;
}

/* 0x0006b3e0: the table of the tasks is made, for 24 of them, and is good
for use; and what else the tasks have starts as nothing */
void online_tasks_initialize(
	void)
{
	online_tasks= data_new("online tasks", MAXIMUM_ONLINE_TASKS, sizeof(struct online_task), 0, heap_allocation);
	data_make_valid(online_tasks);
	unknown_0006bfa0();
	memset(unknown_004771c8, 0, sizeof(unknown_004771c8));
	unknown_00479748= NONE;
}

/* 0x0006b450: the tasks are all done with (unknown_0006b950), their table
is let go of, and so is the task of unknown_00479748 */
void online_tasks_dispose(
	void)
{
	unknown_0006b950();
	data_dispose(online_tasks);
	if (unknown_00479748!=NONE)
	{
		online_task_close(unknown_00479748);
		unknown_00479748= NONE;
	}
}

/* 0x0006b5d0: what a task is at, from its bits: with the bit of 0x4, 2 or,
with the one of 0x20 too, 3; if not, with the bit of 0x2, 1; if not, nothing
with the bit of 0x1 and 4 without it. 5 for a task that there is not */
long online_task_get_status(
	long index)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, index);

	if (!task)
	{
		return _online_task_status_none;
	}
	if (task->flags&_online_task_unknown4_bit)
	{
		return (task->flags&_online_task_unknown20_bit) ? _online_task_status3 : _online_task_status2;
	}
	if (task->flags&_online_task_unknown2_bit)
	{
		return _online_task_status1;
	}

	return (task->flags&_online_task_unknown1_bit) ? _online_task_status0 : _online_task_status4;
}

/* 0x0006b640: a task is done with: if the libraries know it, it is ended in
the way of its kind, and they are told to let go of it; and it is not in the
table any more. One of the second kind goes on until it has no more to do or
cannot go on; the list of friends is told to end, and goes on once more */
void online_task_close(
	long index)
{
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, index);

	if (!task)
	{
		return;
	}
	if (task->handle!=0 && task->handle!=NONE)
	{
		switch (task->type)
		{
		case _online_task_type2:
			{
				long result= 0;

				while (result!=ONLINE_TASK_DONE)
				{
					result= unknown_0006c670(task);
					if (result<0)
					{
						break;
					}
				}
			}
			break;
		case _online_task_type_friends:
			if (XOnlineFriendsEnumerateFinish(task->handle)>=0)
			{
				unknown_0006c670(task);
			}
			break;
		case _online_task_type21:
			unknown_0008c550(index);
			break;
		}
		XOnlineTaskClose(task->handle);
		task->handle= 0;
	}
	datum_delete(online_tasks, index);
}

/* 0x0006b6f0: a new task, of no kind and that the libraries do not know;
NONE if there is no room */
long online_task_new(
	void)
{
	long index= datum_new(online_tasks);

	if (index!=NONE)
	{
		struct online_task *task= (struct online_task *)online_tasks->data+(index&0xFFFF);

		task->handle= 0;
		task->type= NONE;
		task->unknown8= NONE;
		task->flags= 0;
	}

	return index;
}
