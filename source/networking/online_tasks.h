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
	_online_task_type_c= 0xC,
	/* what going on with a task says when the libraries are not asked: there
	is no such task, and no one is signed in */
	ONLINE_TASK_NOT_THERE= 0x80004005,
	ONLINE_TASK_NOT_SIGNED_IN= 0x80151000,
	/* what signing in is at: going on, done (_online_session_signed_in), and
	the ways that it ends in when it could not be */
	_online_logon_state_going= 0,
	_online_logon_state2= 2,
	_online_logon_state3,
	_online_logon_state4,
	_online_logon_state5,
	_online_logon_state6,
	_online_logon_state7,
	_online_logon_state8,
	_online_logon_state_none= 10,
	/* how many players can be signed in, and how many services the game
	asks for when it signs in. What the libraries say of a player whose
	signing in ended in the two ways that are looked for; and what is said
	of all of them: nothing, one ended in the first way, or one in the
	second */
	MAXIMUM_ONLINE_USERS= 4,
	NUMBER_OF_ONLINE_SERVICES= 13,
	ONLINE_USER_RESULT_UNKNOWN1512F0= 0x1512F0,
	ONLINE_USER_RESULT_UNKNOWN80151200= 0x80151200,
	_online_users_result_none= 0,
	_online_users_result_unknown1,
	_online_users_result_unknown2,
	/* how much is said of what a player is at, to the others */
	ONLINE_NOTIFICATION_STATE_SIZE= 4,
	/* two more bits of a task: it is to be done with the next time around,
	and now. More kinds: one that sets a byte when it ends in a way, and
	the four of the storage. And more that the libraries say of a task that
	goes on: it has something, and something of the kind 0xD */
	_online_task_unknown8_bit= 0x8,
	_online_task_unknown10_bit= 0x10,
	_online_task_type_d= 0xD,
	_online_task_type_storage_first= 0x29,
	_online_task_type_storage_last= 0x2C,
	ONLINE_TASK_UNKNOWN1500F0= 0x1500F0,
	ONLINE_TASK_UNKNOWN1500F1= 0x1500F1,
	ONLINE_TASK_UNKNOWN153101= 0x153101,
	/* what the libraries say of the task that changes who is signed in:
	it goes on; it is done; and the players have to be looked at */
	ONLINE_TASK_USERS_GOING= 0x1513F0,
	ONLINE_TASK_USERS_DONE= 0x1513F1,
	ONLINE_TASK_USERS_LOOK= 0x80151300,
	/* the bits of a player that is signed in that say that it is not one
	like the others */
	_online_user_unknown_bits= 0x3,
	/* of the four bytes that say what a player is at: the first two, which
	are always the same; what its third says when a time is 600 or more;
	and what is added to it for the second of the two times */
	ONLINE_STATE_SIGNATURE= 0xFCF1,
	ONLINE_STATE_TIME_MAXIMUM= 126,
	ONLINE_STATE_SECOND_TIME= 127,
	/* of what the game keeps of each gamepad (unknown_0054ed50): what one
	is, and where its bits like the ones of a player that is signed in are.
	How many things something of a player can have, and what one is and
	where what it is known by is */
	ONLINE_CONTROLLER_SIZE= 0xC70,
	ONLINE_CONTROLLER_FLAGS_OFFSET= 0x8,
	MAXIMUM_ONLINE_USER_ENTRIES= 125,
	ONLINE_USER_ENTRY_SIZE= 0x40,
	ONLINE_USER_ENTRY_IDENTIFIER_OFFSET= 0x20
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
	/* of the task that signs in: what it ended at */
	long unknown10;
};

typedef char verify_online_task_size[sizeof(struct online_task) == 0x14 ? 1 : -1];

/* a player that is signed in, as the libraries have it: what it is known
by, and at the end how signing in went for it */
struct online_user
{
	unsigned long xuid[2];
	/* two bits that say that it is not a player like the others */
	byte flags;
	byte unknown9[3];
	/* it has a name: the first character of it */
	char name[0x60];
	long result;
};

typedef char verify_online_user_size[sizeof(struct online_user) == 0x70 ? 1 : -1];

/* what a player is at, before it is made four bytes: its kind, from 1 to 6;
two numbers of two bits; and two times, of which the second is said only
without the first */
struct online_state
{
	long kind;
	long unknown4;
	long unknown8;
	short time;
	short second_time;
};

/* what the service says that a player of this console is at: a kind, from
1 to 6 or nothing; what something is known by, and that thing if the player
has it; and four bits of it */
struct online_user_state
{
	long kind;
	unsigned long identifier;
	byte entry[ONLINE_USER_ENTRY_SIZE];
	boolean unknown48;
	boolean unknown49;
	boolean unknown4a;
	boolean unknown4b;
};

typedef char verify_online_user_state_size[sizeof(struct online_user_state) == 0x4C ? 1 : -1];

/* a service of the service of games, as the game asks for it */
struct online_service
{
	unsigned long identifier;
	/* signing in is no good without it */
	long required;
	/* the service of games has it, as was seen when someone signed in */
	boolean available;
};

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
void online_tasks_check_address(void);
long online_task_continue(struct online_task *task);
long online_task_step(struct online_task *task);
void online_task_storage_done(long result, long index);
boolean online_task_update(long index, struct online_task *task);
boolean __attribute__((stdcall)) online_task_logon_update(struct online_task *task, long index);
boolean online_task_change_users_update(struct online_task *task);
boolean __attribute__((stdcall)) online_logon_complete(struct online_task *task);
void online_users_start_mute_lists(const struct online_user *users);
void __attribute__((stdcall)) online_users_set_state(const struct online_user *users);
void online_state_encode(unsigned long *encoded, const struct online_state *state);
boolean online_user_get_state(long user_index, struct online_user_state *state);
void online_logoff(void);
long online_tasks_get_logon_state(long index);
long online_users_get_result(void);
long online_task_users_get_result(const struct online_task *task);
void online_services_get_identifiers(unsigned long *identifiers);
boolean __attribute__((stdcall)) online_logon_get_state(long *state_out);
boolean online_signed_in(void);
long __attribute__((stdcall)) online_unknown_0006c800(long unknown);
boolean online_unknown_0006c830(const byte *unknown);
byte online_unknown_0006c850(byte code);
void online_get_logon_users(struct online_user *users);
void __attribute__((stdcall)) online_notification_set_state(const unsigned long *session_identifier, long user_index,
	byte *state, long flags);

/* ---------- globals */

extern struct data_array *online_tasks;

#endif
