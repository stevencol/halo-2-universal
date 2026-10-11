/*
ONLINE_PRESENCE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The online tasks of the kind 0x21: what other players of the service of
games are at. The names are guesses.
*/

#include "online_presence.h"

#include "networking/online_tasks.h"

#include <string.h>

/* ---------- prototypes */

/* of the console's libraries, which do not say their names: 0x003a8316
starts a task; the others take what the libraries know the task by */
long __attribute__((stdcall)) unknown_003a8316(long owner, long unknown, long *handle);
long __attribute__((stdcall)) unknown_003a695e(long handle, long unknown, long unknown2, long unknown3);
long __attribute__((stdcall)) unknown_003a6f9b(long handle);
long __attribute__((stdcall)) unknown_003a6fa6(long handle, long unknown, unsigned long count, byte *entries);
long __attribute__((stdcall)) unknown_003a6969(long handle);

/* 0x003a075e: whether a game is this one, or one that counts as it */
long __attribute__((stdcall)) XOnlineTitleIdIsSameTitle(unsigned long title);

/* ---------- globals */

/* 0x00467214, of main_game.c: the task that signs in to the service of
games, or NONE */
extern long unknown_00467214;

/* ---------- code */

/* whether someone is signed in to the service of games, as the game has it
written out in each function of this file */
static inline __attribute__((always_inline)) boolean online_presence_signed_in(
	void)
{
	return unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in;
}

/* a task, if there is one and someone is signed in; NULL if not */
static inline __attribute__((always_inline)) const struct online_task *online_presence_task_get(
	long task_index)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);

	return (task && online_presence_signed_in()) ? task : NULL;
}

/* 0x0008c280: what a player is at, as the game has it, from what the
libraries say and the game that it plays; as online_friend_state_from_library
does it for a friend, but for the bits */
long __attribute__((stdcall)) online_presence_state_from_library(
	unsigned long library_state,
	unsigned long title)
{
	long state= 0;
	boolean same_title= FALSE;

	if (library_state&0x1)
	{
		same_title= XOnlineTitleIdIsSameTitle(title) && online_presence_signed_in();
		state= 0x1;
	}
	if (library_state&0x10)
	{
		state|= 0x8;
	}
	if (library_state&0x80000000)
	{
		state|= 0x10;
	}
	if (library_state&0x2)
	{
		state|= 0x4;
	}
	if (library_state&0x8000000)
	{
		state|= 0x80;
		if (same_title)
		{
			state|= 0x40;
		}
		else
		{
			state&= ~0x40;
		}
	}
	if (library_state&0x8)
	{
		state|= 0x2;
	}
	if (library_state&0x100)
	{
		state|= 0x400;
	}
	if (library_state&0x200)
	{
		state|= 0x1000;
	}
	if (library_state&0x400)
	{
		state|= 0x2000;
	}
	if (library_state&0x800)
	{
		state|= 0x200;
	}

	return state;
}

/* 0x0008c320: a task of the kind 0x21 starts for a player; the task, or
NONE if it could not start */
long online_presence_start(
	long owner)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (unknown_003a8316(owner, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		task->flags= _online_task_unknown1_bit;
		task->type= _online_task_type_presence;
		task->owner= owner;
	}

	return task_index;
}

/* 0x0008c3a0: the libraries are given three things for a task; nothing if
there is no such task, or no one is signed in */
void __attribute__((stdcall)) online_presence_unknown_0008c3a0(
	long task_index,
	long unknown,
	long unknown2,
	long unknown3)
{
	const struct online_task *task= online_presence_task_get(task_index);

	if (task)
	{
		unknown_003a695e(task->handle, unknown, unknown2, unknown3);
	}
}

/* 0x0008c410: the libraries are told something of a task; nothing if there
is no such task, or no one is signed in */
void online_presence_unknown_0008c410(
	long task_index)
{
	const struct online_task *task= online_presence_task_get(task_index);

	if (task)
	{
		unknown_003a6f9b(task->handle);
	}
}

/* 0x0008c470: what the players of a task are at now: so many things of
0x3D bytes, with what each is at as the game has it. Nothing if there is no
such task, no one is signed in, the task is not at its second or third step,
or the libraries do not say */
void __attribute__((stdcall)) online_presence_get_latest(
	long task_index,
	long unknown,
	unsigned long count,
	byte *entries)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);

	memset(entries, 0, count*ONLINE_PRESENCE_SIZE);
	if (task && online_presence_signed_in())
	{
		long status= online_task_get_status(task_index);

		if ((status==_online_task_status1 || status==_online_task_status2) &&
			unknown_003a6fa6(task->handle, unknown, count, entries)>=0)
		{
			unsigned long index;

			for (index= 0; index<count; index++)
			{
				byte *entry= entries+index*ONLINE_PRESENCE_SIZE;
				unsigned long library_state;
				unsigned long title;
				long state;

				memcpy(&library_state, entry+ONLINE_PRESENCE_STATE_OFFSET, sizeof(library_state));
				memcpy(&title, entry+ONLINE_PRESENCE_TITLE_OFFSET, sizeof(title));
				state= online_presence_state_from_library(library_state, title);
				memcpy(entry+ONLINE_PRESENCE_STATE_OFFSET, &state, sizeof(state));
			}
		}
	}
}

/* 0x0008c550: a task of the kind 0x21 is told that it is done with; nothing
if there is no such task, or no one is signed in */
void online_presence_unknown_0008c550(
	long task_index)
{
	const struct online_task *task= online_presence_task_get(task_index);

	if (task)
	{
		unknown_003a6969(task->handle);
	}
}
