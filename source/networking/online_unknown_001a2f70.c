/*
ONLINE_UNKNOWN_001A2F70.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A part of the game that uses the tasks of the service of games
(online_tasks.c): it ends one that something keeps, says whether another
is going on, and keeps a thing of 0x6A2 bytes with a byte that says that
it is there. What it is for is not known, and the name of the file is only
the address of its first function.
*/

#include "cseries/cseries.h"
#include "networking/online_tasks.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* in what 0x001a2f70 is given: a task, and what going on with it said */
	ONLINE_UNKNOWN_001A2F70_TASK_OFFSET= 0x61C,
	ONLINE_UNKNOWN_001A2F70_RESULT_OFFSET= 0x620,

	/* the thing that is kept */
	ONLINE_UNKNOWN_0046E814_SIZE= 0x6A2,

	/* what online_task_get_status says of a task that is going on */
	ONLINE_UNKNOWN_TASK_STATUS1= 1,
	ONLINE_UNKNOWN_TASK_STATUS2= 2
};

/* ---------- globals */

/* 0x0046e7b8: the gamepad whose player this is for, or NONE */
long unknown_0046e7b8= NONE;

/* 0x0046e7bc: whether the task of 0x0046e7d4 counts. 0x0046e7d4: a task */
long unknown_0046e7bc;
long unknown_0046e7d4= NONE;

/* 0x0046e814: the thing that is kept. 0x0046eeb6: whether it is there, and
a byte that goes with it */
byte unknown_0046e814[ONLINE_UNKNOWN_0046E814_SIZE];
byte unknown_0046eeb6[2];

/* ---------- code */

/* 0x001a2f70: the task that something keeps, if it has one, goes on once
(online_task_continue, or ONLINE_TASK_NOT_THERE if there is no such task),
what it says is kept, and the task is closed and forgotten */
void online_unknown_001a2f70_end_task(
	byte *data)
{
	long *task_index= (long *)(data+ONLINE_UNKNOWN_001A2F70_TASK_OFFSET);
	long *result= (long *)(data+ONLINE_UNKNOWN_001A2F70_RESULT_OFFSET);
	struct online_task *task;

	if (*task_index==NONE)
	{
		return;
	}
	task= online_task_get(*task_index);
	*result= task ? online_task_continue(task) : (long)ONLINE_TASK_NOT_THERE;
	online_task_close(*task_index);
	*task_index= NONE;
}

/* 0x001a3230: whether the task of 0x0046e7d4 is going on (its status is the
first or the second), if there is one and 0x0046e7bc says that it counts */
boolean online_unknown_001a3230_task_going_on(
	void)
{
	long status;

	if (unknown_0046e7d4==NONE || !unknown_0046e7bc)
	{
		return FALSE;
	}
	status= online_task_get_status(unknown_0046e7d4);

	return status==ONLINE_UNKNOWN_TASK_STATUS1 || status==ONLINE_UNKNOWN_TASK_STATUS2;
}

/* 0x001a325a: the byte that goes with the thing that is kept, if it is
there; 1 if not */
byte online_unknown_001a325a_get_byte(
	void)
{
	return unknown_0046eeb6[0] ? unknown_0046eeb6[1] : 1;
}

/* 0x001a3269: the thing that is kept, or zeros if it is not there; whether
it is there */
boolean online_unknown_001a3269_get(
	byte *thing)
{
	if (unknown_0046eeb6[0])
	{
		memcpy(thing, unknown_0046e814, ONLINE_UNKNOWN_0046E814_SIZE);
	}
	else
	{
		memset(thing, 0, ONLINE_UNKNOWN_0046E814_SIZE);
	}

	return unknown_0046eeb6[0];
}

/* 0x001a3294: the thing that is kept is not there any more, and is zeros */
void online_unknown_001a3294_clear(
	void)
{
	unknown_0046eeb6[0]= FALSE;
	memset(unknown_0046e814, 0, ONLINE_UNKNOWN_0046E814_SIZE);
}

/* 0x001a2fb4: the number of 0x610 of something is cleared, and its task
ended (online_unknown_001a2f70_end_task) */
void online_unknown_001a2f70_clear_and_end_task(
	byte *data)
{
	*(long *)(data+0x610)= 0;
	online_unknown_001a2f70_end_task(data);
}

/* 0x001a353a: the name of a game (online_title_get_name), or nothing if no
game is given */
void __attribute__((stdcall)) online_unknown_001a2f70_get_title_name(
	wchar *name,
	unsigned long title,
	long size)
{
	name[0]= 0;
	if (title)
	{
		online_title_get_name(title, name, size);
	}
}
