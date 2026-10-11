/*
ONLINE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The tasks of the service of games by the network. The names of the functions
are guesses.
*/

#include "online_tasks.h"

#include "networking/transport.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0008c550: a task of the kind 0x21 is told that it is done with.
0x0006c560: a task does what it has to each time around; whether the others
are to. 0x0008bf70: and so does what the tasks have of their own */
void unknown_0008c550(long index);
boolean unknown_0006c560(long index, struct online_task *task);
void unknown_0008bf70(byte *unknown);

/* 0x003a11ae, of the console's libraries: something of the service of
games, from 1 to 3 */
long unknown_003a11ae(void);

/* 0x003a07b9, 0x003a062b, of the console's libraries: the list of friends
is done with, and so is a task. 0x003a0620: a task goes on a little.
0x003a0650: how signing in went */
long __attribute__((stdcall)) XOnlineFriendsEnumerateFinish(long handle);
long __attribute__((stdcall)) XOnlineTaskClose(long handle);
long __attribute__((stdcall)) XOnlineTaskContinue(long handle);
long __attribute__((stdcall)) XOnlineLogonTaskGetResults(long handle);

/* ---------- globals */

/* 0x004cf78c: the table of the tasks. 0x004771c8: something of them, and
0x00479748: a task of its own, or NONE */
struct data_array *online_tasks;
byte unknown_004771c8[ONLINE_TASKS_UNKNOWN_004771C8_SIZE];
long unknown_00479748;

/* 0x00467214, of main_game.c: the task that signs in to the service of
games, or NONE. 0x00467218: what signing in was last at. 0x0050944f: it
ended in a way of its own. 0x0043ff84: two addresses of consoles, and
0x0050944e: this console has one of them */
extern long unknown_00467214;
long unknown_00467218= _online_logon_state_none;
boolean unknown_0050944f;
const byte unknown_0043ff84[2][TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_SIZE]=
{
	{0x00, 0x0D, 0x3A, 0x5D, 0xD1, 0xF1},
	{0x00, 0x50, 0xF2, 0x10, 0x52, 0x20}
};
boolean unknown_0050944e;

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
	online_tasks_check_address();
	memset(unknown_004771c8, 0, sizeof(unknown_004771c8));
	unknown_00479748= NONE;
}

/* 0x0006b450: the tasks are all done with (online_tasks_close_all), their table
is let go of, and so is the task of unknown_00479748 */
void online_tasks_dispose(
	void)
{
	online_tasks_close_all();
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
					result= online_task_continue(task);
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
				online_task_continue(task);
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
		task->owner= NONE;
		task->flags= 0;
	}

	return index;
}

/* 0x0006b730: a new task, if someone is signed in to the service of games;
NONE if not, or there is no room */
long online_task_new_signed_in(
	void)
{
	if (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in)
	{
		return online_task_new();
	}

	return NONE;
}

/* 0x0006b780: a task is done with and is new again, in the same place and
of the same kind and owner. The game does not look at whether there is such
a task */
void online_task_restart(
	long index)
{
	struct online_task *task= (struct online_task *)online_tasks->data+(index&0xFFFF);
	long type= task->type;
	long owner= task->owner;

	online_task_close(index);
	index= datum_new_at_index(online_tasks, index);
	task= (struct online_task *)online_tasks->data+(index&0xFFFF);
	task->type= type;
	task->owner= owner;
	task->flags= 0;
}

/* 0x0006b7e0: the kind of a task. The game does not look at whether there
is such a task */
long online_task_get_type(
	long index)
{
	return ((const struct online_task *)online_tasks->data)[index&0xFFFF].type;
}

/* whether a task is of a kind and of an owner: any owner, with NONE or 0xFF */
static inline __attribute__((always_inline)) boolean online_task_matches(
	const struct online_task *task,
	long owner,
	long type)
{
	return task->type==type && (task->owner==owner || owner==NONE || owner==ONLINE_TASK_ANY_OWNER_);
}

/* 0x0006b800: the first task of a kind and of an owner, or NONE */
long __attribute__((stdcall)) online_task_find(
	long owner,
	long type)
{
	long absolute_index= NONE;

	for (;;)
	{
		const struct online_task *task;

		absolute_index= data_next_absolute_index(online_tasks, absolute_index+1);
		if (absolute_index==NONE)
		{
			return NONE;
		}
		task= (const struct online_task *)online_tasks->data+absolute_index;
		if (online_task_matches(task, owner, type))
		{
			return (task->identifier<<16)|absolute_index;
		}
	}
}

/* 0x0006b890: whether there is a task of a kind and of an owner */
long __attribute__((stdcall)) online_task_exists(
	long owner,
	long type)
{
	long absolute_index= NONE;

	for (;;)
	{
		absolute_index= data_next_absolute_index(online_tasks, absolute_index+1);
		if (absolute_index==NONE)
		{
			return FALSE;
		}
		if (online_task_matches((const struct online_task *)online_tasks->data+absolute_index, owner, type))
		{
			return TRUE;
		}
	}
}

/* 0x0006b910: a task, or NULL if there is not one of that number */
struct online_task *online_task_get(
	long index)
{
	return (struct online_task *)datum_try_and_get(online_tasks, index);
}

/* 0x0006b4a0: what the tasks do each time around, if the session of the
service of games is there and at nothing or signed in: each task in turn,
until one says that the others are not to; and then what the tasks have of
their own */
void online_tasks_update(
	void)
{
	long absolute_index= NONE;
	long state;

	if (unknown_00467214==NONE)
	{
		return;
	}
	state= online_tasks_get_logon_state(unknown_00467214);
	if (state<0 || state>_online_session_signed_in)
	{
		return;
	}
	for (;;)
	{
		struct online_task *task;

		absolute_index= data_next_absolute_index(online_tasks, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		task= (struct online_task *)online_tasks->data+absolute_index;
		if (!unknown_0006c560((task->identifier<<16)|absolute_index, task))
		{
			break;
		}
	}
	unknown_0008bf70(unknown_004771c8);
}

/* 0x0006b550: something of the service of games that the libraries say,
from 1 to 3, if someone is signed in; nothing if not, or if they say
anything else */
long online_tasks_unknown_0006b550(
	void)
{
	if (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in)
	{
		switch (unknown_003a11ae())
		{
		case 1:
			return 1;
		case 2:
			return 2;
		case 3:
			return 3;
		}
	}

	return 0;
}

/* 0x0006b950: all the tasks are done with, in an order: the table is gone
over until it has none, and each time a task is done with unless it has to
wait. One of the kind 0x21 waits until no more than four are left; of the
second kind, three; of the first, two; and of the kind 0, until it is the
only one. One of the kind 0xB waits while there is one of the kind 0xC */
void online_tasks_close_all(
	void)
{
	while (online_tasks->actual_count>0)
	{
		long absolute_index= NONE;

		for (;;)
		{
			const struct online_task *task;
			boolean close;
			long index;

			absolute_index= data_next_absolute_index(online_tasks, absolute_index+1);
			if (absolute_index==NONE)
			{
				break;
			}
			task= (const struct online_task *)online_tasks->data+absolute_index;
			index= (task->identifier<<16)|absolute_index;
			switch (task->type)
			{
			case _online_task_type0:
				close= online_tasks->actual_count==1;
				break;
			case _online_task_type1:
				close= online_tasks->actual_count<=2;
				break;
			case _online_task_type2:
				close= online_tasks->actual_count<=3;
				break;
			case _online_task_type_b:
				close= !online_task_exists(ONLINE_TASK_ANY_OWNER_, _online_task_type_c);
				break;
			case _online_task_type21:
				close= online_tasks->actual_count<=4;
				break;
			default:
				close= TRUE;
				break;
			}
			if (close)
			{
				online_task_close(index);
			}
		}
	}
}

/* 0x0006bfa0: whether what the console is known by in its own network (the
six bytes of unknown_004cf7cc) is one of two that the game has */
void online_tasks_check_address(
	void)
{
	boolean found= FALSE;
	long index;

	for (index= 0; index<2; index++)
	{
		if (memcmp(unknown_004cf7cc, unknown_0043ff84[index], sizeof(unknown_0043ff84[index]))==0)
		{
			found= TRUE;
			break;
		}
	}
	unknown_0050944e= found;
}

/* 0x0006c670: a task goes on a little; what the libraries say of it. They
are not asked for a task that there is not or that they do not know
(0x80004005); nor, unless someone is signed in, and for a task of the kind
0 unless the network of the console is there (0x80151000) */
long online_task_continue(
	struct online_task *task)
{
	if (!task || task->handle==0 || task->handle==NONE)
	{
		return (long)ONLINE_TASK_NOT_THERE;
	}
	if ((task->type==_online_task_type0 && transport_available()) ||
		(unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in))
	{
		return XOnlineTaskContinue(task->handle);
	}

	return (long)ONLINE_TASK_NOT_SIGNED_IN;
}

/* what signing in ended at is kept by its task */
static inline __attribute__((always_inline)) long online_tasks_logon_ended(
	struct online_task *task,
	long state)
{
	task->flags|= _online_task_unknown20_bit;
	task->unknown10= state;
	unknown_00467218= state;

	return state;
}

/* 0x0006cd50: what signing in to the service of games is at, from its task:
nothing while it goes on, 1 when someone is signed in, and from 2 on when it
ended in another way, which its task keeps. Without a task it is what it was
last. A task that the libraries do not know is at 10, and without the
network of the console it ends at 5 */
long online_tasks_get_logon_state(
	long index)
{
	struct online_task *task= index!=NONE ? online_task_get(index) : NULL;
	long result;

	if (!task)
	{
		return unknown_00467218;
	}
	if (task->type==_online_task_type1 || (task->flags&_online_task_unknown20_bit))
	{
		return task->unknown10;
	}
	if (task->handle==0 || task->handle==NONE)
	{
		unknown_00467218= _online_logon_state_none;

		return _online_logon_state_none;
	}
	if (!transport_available())
	{
		return online_tasks_logon_ended(task, _online_logon_state5);
	}
	result= XOnlineLogonTaskGetResults(task->handle);
	switch ((unsigned long)result)
	{
	case 0:
		unknown_00467218= _online_logon_state_going;
		return _online_logon_state_going;
	case 0x1510F0:
		unknown_00467218= _online_session_signed_in;
		return _online_session_signed_in;
	case 0x1512F0:
		unknown_0050944f= TRUE;
		break;
	case 0x80151200:
		return online_tasks_logon_ended(task, _online_logon_state2);
	case 0x80151001:
		return online_tasks_logon_ended(task, _online_logon_state4);
	case 0x80151002:
		return online_tasks_logon_ended(task, _online_logon_state3);
	case 0x80151003:
		return online_tasks_logon_ended(task, _online_logon_state8);
	case 0x80151004:
		return online_tasks_logon_ended(task, _online_logon_state5);
	case 0x80151005:
		return online_tasks_logon_ended(task, _online_logon_state7);
	case 0x80151006:
		return online_tasks_logon_ended(task, _online_logon_state6);
	}

	return online_tasks_logon_ended(task, _online_logon_state_none);
}
