/*
ONLINE_TASKS.H

The tasks of the service of games by the network: each is something that the
console's libraries do little by little, which the game keeps a number for.
The game calls their table "online tasks". The names of the functions are
guesses.
*/

#ifndef __ONLINE_TASKS_H
#define __ONLINE_TASKS_H

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	MAXIMUM_ONLINE_TASKS= 24,
	/* the bits of a task, of which what it is at is made */
	_online_task_unknown1_bit= 0x1,
	_online_task_unknown2_bit= 0x2,
	_online_task_unknown4_bit= 0x4,
	_online_task_unknown20_bit= 0x20,
	/* what a task is at, from its bits; the last is for one that there is
	not */
	_online_task_status0= 0,
	_online_task_status1,
	_online_task_status2,
	_online_task_status3,
	_online_task_status4,
	_online_task_status_none,
	/* the kinds of task that are done with in a way of their own; the third
	is a list of friends. What the libraries say when a task has no more to
	do */
	_online_task_type2= 2,
	_online_task_type_friends= 3,
	_online_task_type21= 0x21,
	ONLINE_TASK_DONE= 0x1500F2,
	/* what a task is looked for with when it does not matter whose it is,
	as is so with NONE; and what the session says when someone is signed in */
	ONLINE_TASK_ANY_OWNER_= 0xFF,
	_online_session_signed_in= 1,
	/* the size of something of the tasks that starts as nothing */
	ONLINE_TASKS_UNKNOWN_004771C8_SIZE= 0x2580,
	/* the kinds of task that are done with last when all are, and one that
	waits for another kind */
	_online_task_type0= 0,
	_online_task_type1= 1,
	_online_task_type_b= 0xB,
	_online_task_type_c= 0xC
};

/* ---------- structures */

/* a task: its bits, its kind, whose it is, and what the libraries know it by,
which is nothing or NONE when they do not */
struct online_task
{
	short identifier;
	word flags;
	long type;
	/* whose it is: the number of a gamepad */
	long owner;
	long handle;
	long unknown10;
};

typedef char verify_online_task_size[sizeof(struct online_task) == 0x14 ? 1 : -1];

/* what goes over the tasks: the one that it is at, and where */
struct online_task_iterator
{
	struct online_task *task;
	struct data_iterator iterator;
};

/* ---------- prototypes/ONLINE_TASKS.C */

boolean online_task_iterator_next(struct online_task_iterator *iterator);
void online_tasks_initialize(void);
void online_tasks_dispose(void);
long online_task_get_status(long index);
void online_task_close(long index);
long online_task_new(void);
long online_task_new_signed_in(void);
void online_task_restart(long index);
long online_task_get_type(long index);
long __attribute__((stdcall)) online_task_find(long owner, long type);
long __attribute__((stdcall)) online_task_exists(long owner, long type);
struct online_task *online_task_get(long index);
void online_tasks_update(void);
long online_tasks_unknown_0006b550(void);
void online_tasks_close_all(void);

/* ---------- globals */

extern struct data_array *online_tasks;

#endif
