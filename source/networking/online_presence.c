/*
ONLINE_PRESENCE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The online tasks of the kind 0x21: what other players of the service of
games are at. The names are guesses.
*/

#include "online_presence.h"

#include "interface/user_interface_controller.h"
#include "networking/online_tasks.h"
#include "networking/online_teams.h"

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
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);
unsigned long GetTickCount(void);

/* ---------- globals */

/* 0x00467214, of main_game.c: the task that signs in to the service of
games, or NONE */
extern long unknown_00467214;

/* 0x004cf768: for how many seconds a name that the game remembers is good.
0x00510548, 0x0051054c, of main.c: the time is not the console's, and what
it is */
long unknown_004cf768;
extern boolean unknown_00510548;
extern unsigned long unknown_0051054c;

/* ---------- code */

/* the time, as the game has it written out where it wants it: the console's,
or the one that main.c has in its place */
static inline __attribute__((always_inline)) unsigned long online_presence_time(
	void)
{
	return unknown_00510548 ? unknown_0051054c : GetTickCount();
}

/* what is being asked for has no team: it is known, as of now, and the task
is done with */
static inline __attribute__((always_inline)) void online_presence_names_finish(
	byte *names)
{
	byte *current= ONLINE_PRESENCE_NAMES_CURRENT(names);
	long task_index;

	*(unsigned long *)(current+ONLINE_PRESENCE_NAME_TIME_OFFSET)= online_presence_time();
	*(long *)(current+ONLINE_PRESENCE_NAME_STATE_OFFSET)= _online_presence_name_known;
	task_index= ONLINE_PRESENCE_NAMES_TASK(names);
	ONLINE_PRESENCE_NAMES_CURRENT(names)= NULL;
	online_task_close(task_index);
	ONLINE_PRESENCE_NAMES_TASK(names)= NONE;
}

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

/* 0x0008c150: the name of a player, and who it is as the game has it, from
what the game remembers; whether it has them. A name that is too old is not
given, and is to be asked for again; if nothing is remembered of the player,
the place that has no name or the oldest one is taken for it, to be asked
for. With the number that is not nothing, an old name is given all the same
and no place is taken */
boolean __attribute__((stdcall)) online_presence_names_find(
	byte *names,
	long unknown,
	const unsigned long *user,
	wchar *name,
	byte *identifier)
{
	byte *entry= names;
	byte *oldest= NULL;
	boolean found= FALSE;
	boolean result= FALSE;
	long index;

	name[0]= 0;
	for (index= 0; index<MAXIMUM_ONLINE_PRESENCE_NAMES; index++, entry+= ONLINE_PRESENCE_NAME_SIZE)
	{
		if (!oldest || *(const unsigned long *)(entry+ONLINE_PRESENCE_NAME_TIME_OFFSET)<
			*(const unsigned long *)(oldest+ONLINE_PRESENCE_NAME_TIME_OFFSET))
		{
			long state= *(const long *)(entry+ONLINE_PRESENCE_NAME_STATE_OFFSET);

			if (state==_online_presence_name_known || state==_online_presence_name_free)
			{
				oldest= entry;
			}
		}
		if (user && user[0]==*(const unsigned long *)entry && user[1]==*(const unsigned long *)(entry+4))
		{
			found= TRUE;
			break;
		}
	}
	if (found && *(const long *)(entry+ONLINE_PRESENCE_NAME_STATE_OFFSET)==_online_presence_name_known)
	{
		long maximum_age= unknown_004cf768*1000;

		if (unknown==0)
		{
			unsigned long then= *(const unsigned long *)(entry+ONLINE_PRESENCE_NAME_TIME_OFFSET);
			unsigned long now= unknown_00510548 ? unknown_0051054c : GetTickCount();

			if ((long)(now-then)>maximum_age)
			{
				*(long *)(entry+ONLINE_PRESENCE_NAME_STATE_OFFSET)= _online_presence_name_asked;

				return FALSE;
			}
		}
		wcsncpy(name, (const wchar *)(entry+ONLINE_PRESENCE_NAME_TEXT_OFFSET), ONLINE_PRESENCE_NAME_TEXT_LENGTH);
		name[ONLINE_PRESENCE_NAME_TEXT_LENGTH]= 0;
		memcpy(identifier, entry+ONLINE_PRESENCE_NAME_IDENTIFIER_OFFSET, ONLINE_PRESENCE_NAME_IDENTIFIER_SIZE);
		result= TRUE;
	}
	if (unknown==0 && !found && oldest)
	{
		memcpy(oldest, user, ONLINE_PRESENCE_NAME_IDENTIFIER_SIZE);
		*(long *)(oldest+ONLINE_PRESENCE_NAME_STATE_OFFSET)= _online_presence_name_asked;
	}

	return result;
}

/* 0x0008bf70: each time around: if a task asks for the teams of one of the
players that the game remembers, and it has them, the first team and its
name are kept for the player (nothing if it has none, or the task could not
go on), and the task is done with; and if no task asks, one starts for the
first player that waits to be asked for, in the name of the first gamepad
whose player has a profile and is not a guest */
void online_presence_names_update(
	byte *names)
{
	if (ONLINE_PRESENCE_NAMES_TASK(names)!=NONE)
	{
		long status= online_task_get_status(ONLINE_PRESENCE_NAMES_TASK(names));

		if (status!=_online_task_status0)
		{
			byte *current;

			if (status>_online_task_status0 && status<=_online_task_status2)
			{
				struct online_xuid teams[MAXIMUM_ONLINE_PRESENCE_TEAMS];
				byte information[ONLINE_TEAM_SIZE];
				long count= MAXIMUM_ONLINE_PRESENCE_TEAMS;

				online_teams_enumerate_get_results(ONLINE_PRESENCE_NAMES_TASK(names), &count, teams);
				if (count>0)
				{
					wchar *text;

					online_team_get(ONLINE_PRESENCE_NAMES_TASK(names), &teams[0], information);
					current= ONLINE_PRESENCE_NAMES_CURRENT(names);
					text= (wchar *)(current+ONLINE_PRESENCE_NAME_TEXT_OFFSET);
					wcsncpy(text, (const wchar *)(information+sizeof(struct online_xuid)), ONLINE_PRESENCE_NAME_TEXT_LENGTH);
					text[ONLINE_PRESENCE_NAME_TEXT_LENGTH]= 0;
					memcpy(ONLINE_PRESENCE_NAMES_CURRENT(names)+ONLINE_PRESENCE_NAME_IDENTIFIER_OFFSET, &teams[0],
						ONLINE_PRESENCE_NAME_IDENTIFIER_SIZE);
				}
				else
				{
					current= ONLINE_PRESENCE_NAMES_CURRENT(names);
					*(wchar *)(current+ONLINE_PRESENCE_NAME_TEXT_OFFSET)= 0;
					current= ONLINE_PRESENCE_NAMES_CURRENT(names);
					memset(current+ONLINE_PRESENCE_NAME_IDENTIFIER_OFFSET, 0, 2*sizeof(long));
				}
			}
			else
			{
				current= ONLINE_PRESENCE_NAMES_CURRENT(names);
				*(wchar *)(current+ONLINE_PRESENCE_NAME_TEXT_OFFSET)= 0;
				current= ONLINE_PRESENCE_NAMES_CURRENT(names);
				memset(current+ONLINE_PRESENCE_NAME_IDENTIFIER_OFFSET, 0, 2*sizeof(long));
			}
			online_presence_names_finish(names);
		}
	}
	if (ONLINE_PRESENCE_NAMES_TASK(names)==NONE)
	{
		long controller_index= user_interface_controllers_first_profile(1);

		if (controller_index>=0 && controller_index<MAXIMUM_GAMEPADS)
		{
			long index;

			for (index= 0; index<MAXIMUM_ONLINE_PRESENCE_NAMES; index++)
			{
				byte *entry= names+index*ONLINE_PRESENCE_NAME_SIZE;

				if (*(const long *)(entry+ONLINE_PRESENCE_NAME_STATE_OFFSET)==_online_presence_name_asked)
				{
					long task_index= online_teams_enumerate_start((const struct online_xuid *)entry, controller_index);

					ONLINE_PRESENCE_NAMES_TASK(names)= task_index;
					if (task_index!=NONE)
					{
						ONLINE_PRESENCE_NAMES_CURRENT(names)= entry;
						*(long *)(entry+ONLINE_PRESENCE_NAME_STATE_OFFSET)= _online_presence_name_asking;
					}
					break;
				}
			}
		}
	}
}
