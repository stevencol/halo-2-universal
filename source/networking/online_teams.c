/*
ONLINE_TEAMS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The online tasks of the teams of the service of games. The names of the
functions are guesses.
*/

#include "online_teams.h"

#include <string.h>

/* ---------- prototypes */

/* of the console's libraries, which do not say their names: each starts a
task of the teams, but for 0x003aeba5 and 0x003aebb0, which give what a task
found, and 0x003afc50 and 0x003afca3, which give one thing of what it found */
long __attribute__((stdcall)) unknown_003afc26(long owner, struct online_xuid user, long unknown, long *handle);
long __attribute__((stdcall)) unknown_003aeba5(long handle, long *count, struct online_xuid *teams);
long __attribute__((stdcall)) unknown_003afc50(long handle, struct online_xuid team, byte *information);
long __attribute__((stdcall)) unknown_003afb80(long owner, long unknown, byte *unknown2, long number, long unknown3,
	long *handle);
long __attribute__((stdcall)) unknown_003afb8f(long owner, struct online_xuid team, long unknown, long *handle);
long __attribute__((stdcall)) unknown_003a8407(long owner, struct online_xuid team, long answer, long unknown,
	long *handle);
long __attribute__((stdcall)) unknown_003afbf1(long owner, struct online_xuid team, struct online_xuid unknown,
	long unknown2, long *handle);
long __attribute__((stdcall)) unknown_003afc76(long owner, struct online_xuid team, long unknown, long unknown2,
	long *handle);
long __attribute__((stdcall)) unknown_003aebb0(long handle, long *count, struct online_xuid *members);
long __attribute__((stdcall)) unknown_003afca3(long handle, struct online_xuid member, byte *information);

/* ---------- globals */

/* 0x00467214, of main_game.c: the task that signs in to the service of
games, or NONE */
extern long unknown_00467214;

/* ---------- code */

/* whether someone is signed in to the service of games, as the game has it
written out in each function that asks a task of this file for what it
found */
static inline __attribute__((always_inline)) boolean online_teams_signed_in(
	void)
{
	return unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in;
}

/* a new task that has started is of a kind and of an owner */
static inline __attribute__((always_inline)) void online_teams_task_started(
	struct online_task *task,
	long type,
	long owner)
{
	task->flags= _online_task_unknown1_bit;
	task->type= type;
	task->owner= owner;
}

/* 0x000abc70: a task starts that looks for the teams that a player is in;
the task, or NONE if it could not start */
long __attribute__((stdcall)) online_teams_enumerate_start(
	const struct online_xuid *user,
	long owner)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (unknown_003afc26(owner, *user, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_teams_enumerate, owner);
	}

	return task_index;
}

/* 0x000abd00: the teams that a task found, and how many: no more than the
number says when it is asked. None if there is no such task, no one is
signed in, or the task is not at its second or third step */
void __attribute__((stdcall)) online_teams_enumerate_get_results(
	long task_index,
	long *count,
	struct online_xuid *teams)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);
	long found= 0;

	if (task && online_teams_signed_in() &&
		(online_task_get_status(task_index)==_online_task_status1 ||
		online_task_get_status(task_index)==_online_task_status2))
	{
		found= *count;
		unknown_003aeba5(task->handle, &found, teams);
	}
	*count= found;
}

/* 0x000abdb0: what the libraries say of one of the teams that a task
found; nothing where the function before this one gives none */
void __attribute__((stdcall)) online_team_get(
	long task_index,
	const struct online_xuid *team,
	byte *information)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);

	memset(information, 0, ONLINE_TEAM_SIZE);
	if (task && online_teams_signed_in() &&
		(online_task_get_status(task_index)==_online_task_status1 ||
		online_task_get_status(task_index)==_online_task_status2))
	{
		unknown_003afc50(task->handle, *team, information);
	}
}

/* 0x000abe60: a task of the teams starts, of the kind 0x18; the task, or
NONE if it could not start */
long __attribute__((stdcall)) online_team_unknown_000abe60_start(
	long owner,
	long unknown)
{
	byte unknown2[ONLINE_TEAM_UNKNOWN_000ABE60_SIZE];
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		long none= NONE;
		word nothing= 0;

		memcpy(unknown2, &none, sizeof(none));
		memcpy(unknown2+sizeof(none), &nothing, sizeof(nothing));
		if (unknown_003afb80(owner, unknown, unknown2, ONLINE_TEAM_UNKNOWN_000ABE60_NUMBER, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_team_unknown18, owner);
	}

	return task_index;
}

/* 0x000abf10: a task of a team starts, of the kind 0x19; the task, or NONE
if it could not start */
long __attribute__((stdcall)) online_team_unknown_000abf10_start(
	const struct online_xuid *team,
	long owner)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (unknown_003afb8f(owner, *team, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_team_unknown19, owner);
	}

	return task_index;
}

/* 0x000abfa0: a task starts that gives an answer of three to something of
a team, of the kind 0x1D; the task, or NONE if it could not start */
long __attribute__((stdcall)) online_team_unknown_000abfa0_start(
	const struct online_xuid *team,
	long owner,
	long answer)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		long said;

		switch (answer)
		{
		case 0:
			said= 0;
			break;
		case 1:
			said= 1;
			break;
		default:
			said= NUMBER_OF_ONLINE_TEAM_ANSWERS-1;
			break;
		}
		if (unknown_003a8407(owner, *team, said, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_team_unknown1D, owner);
	}

	return task_index;
}

/* 0x000ac050: a task of a team and of someone starts, of the kind 0x1F; the
task, or NONE if it could not start */
long __attribute__((stdcall)) online_team_unknown_000ac050_start(
	const struct online_xuid *unknown,
	long owner,
	const struct online_xuid *team)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (unknown_003afbf1(owner, *team, *unknown, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_team_unknown1F, owner);
	}

	return task_index;
}

/* 0x000ac110: a task starts that looks for the members of a team; the
task, or NONE if it could not start */
long __attribute__((stdcall)) online_team_members_enumerate_start(
	const struct online_xuid *team,
	long owner)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (unknown_003afc76(owner, *team, 1, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		online_teams_task_started(task, _online_task_type_team_members_enumerate, owner);
	}

	return task_index;
}

/* 0x000ac1b0: the members that a task found, and how many: no more than
the number says when it is asked. None if there is no such task, no one is
signed in, or the task is not at its third step */
void __attribute__((stdcall)) online_team_members_enumerate_get_results(
	long task_index,
	long *count,
	struct online_xuid *members)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);
	long found= 0;

	if (task && online_teams_signed_in() && online_task_get_status(task_index)==_online_task_status2)
	{
		found= *count;
		unknown_003aebb0(task->handle, &found, members);
	}
	*count= found;
}

/* 0x000ac250: what the libraries say of one of the members that a task
found; nothing where the function before this one gives none, or if the
libraries do not say. Its bits become a number: 3 with the five lowest, 2
with the third to the fifth, and if not the fifth */
void __attribute__((stdcall)) online_team_member_get(
	long task_index,
	const struct online_xuid *member,
	byte *information)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);

	memset(information, 0, ONLINE_TEAM_MEMBER_SIZE);
	if (task && online_teams_signed_in() && online_task_get_status(task_index)==_online_task_status2)
	{
		if (unknown_003afca3(task->handle, *member, information)>=0)
		{
			unsigned long flags;

			memcpy(&flags, information+ONLINE_TEAM_MEMBER_FLAGS_OFFSET, sizeof(flags));
			if ((flags&0x1F)==0x1F)
			{
				flags= 3;
			}
			else if ((flags&0x1C)==0x1C)
			{
				flags= 2;
			}
			else
			{
				flags= (flags>>4)&1;
			}
			memcpy(information+ONLINE_TEAM_MEMBER_FLAGS_OFFSET, &flags, sizeof(flags));
		}
		else
		{
			memset(information, 0, ONLINE_TEAM_MEMBER_SIZE);
		}
	}
}
