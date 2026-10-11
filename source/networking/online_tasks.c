/*
ONLINE_TASKS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The tasks of the service of games by the network. The names of the functions
are guesses.
*/

#include "online_tasks.h"

#include "interface/user_interface_controller.h"
#include "networking/game_voice.h"
#include "networking/network_blob.h"
#include "networking/network_observer.h"
#include "networking/network_session.h"
#include "networking/transport.h"
#include "networking/voice_mute_list.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0008c550: a task of the kind 0x21 is told that it is done with.
0x0006c560: a task does what it has to each time around; whether the others
are to. 0x0008bf70: and so does what the tasks have of their own */
void online_presence_unknown_0008c550(long index);

/* 0x0008cc40: something is told that a task of the second kind could not
go on. 0x0008e5e0, of network_blob_task.c: what did a task of the blocks
ended. 0x0007f410: something of the network is told that someone is signed
in. 0x0018fa94: which of them is the first that something is so of,
or NONE. 0x0008c660, 0x0008d4d0: two more things that are told that someone
signed in */
void online_friends_startup(void);
void unknown_0007f410(void);
long __attribute__((stdcall)) user_interface_controllers_first_online_user(const struct online_user *users);
void unknown_0008c660(void);

/* 0x00190da5: a gamepad is told that no one is signed in */
void __attribute__((stdcall)) user_interface_controller_sign_out(long user_index);

/* 0x003a0798, of the console's libraries: what a player of this console is
at, in twelve bytes: a kind, what something is known by, and some bits;
nothing if it is not known. 0x0008e750: the things that a player has, each
of 0x40 bytes, and how many */
long __attribute__((stdcall)) unknown_003a0798(long user_index, byte *state, long *unknown);
void __attribute__((stdcall)) unknown_0008e750(long user_index, byte *entries, long *count);
void __attribute__((stdcall)) network_blob_task_completed(long status, long handle);
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

/* 0x003a0671: the players that are signed in, or NULL. 0x003a0666: how
changing them went, for each. 0x003a0636: something of the players, by
number. 0x003a0774: the others are told what a player is at. 0x003314b0: the
time since the console started */
const struct online_user *XOnlineGetLogonUsers(void);
long __attribute__((stdcall)) XOnlineGetServiceInfo(unsigned long identifier, void *info);
long __attribute__((stdcall)) XOnlineChangeLogonUsersTaskGetResults(long handle, long *results);
long __attribute__((stdcall)) XOnlineGetUsers(long unknown, long *value);
long __attribute__((stdcall)) XOnlineNotificationSetState(long user_index, long flags, unsigned long session_low,
	unsigned long session_high, long size, byte *state);
unsigned long GetTickCount(void);

/* 0x0033039a: the language of the console. 0x003a07cf: the name of a game,
in a language. 0x003a0827: the invitation to a game that the player took
before the game started. 0x003cd344: an address of the network is let go
of */
long XGetLanguage(void);
long __attribute__((stdcall)) XOnlineFriendsGetTitleName(unsigned long title, long language, long size, wchar *name);
long __attribute__((stdcall)) XOnlineFriendsGetAcceptedGameInvite(void *invite);
long __attribute__((stdcall)) XNetUnregisterInAddr(unsigned long in_address);
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* 0x003a065b: the players that are signed in become others; a task of the
libraries does it */
long __attribute__((stdcall)) XOnlineChangeLogonUsers(const struct online_user *users, long unknown, long *handle);

/* 0x003a0641: players sign in to the service of games, for some services;
a task of the libraries does it */
long __attribute__((stdcall)) XOnlineLogon(const struct online_user *users, const unsigned long *services,
	long service_count, long unknown, long *handle);

/* 0x003a069d: the service of games is asked whether there is new content
for the game; a task of the libraries does it */
long __attribute__((stdcall)) XOnlineOfferingIsNewContentAvailable(long unknown, long unknown2, long *handle);

/* ---------- structures */

/* what the libraries say of a service: what it is, and where it is */
struct online_service_info
{
	unsigned long identifier;
	unsigned long address;
	word port;
	word unknownA;
};

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

/* 0x0051055d: a task of the kind 0xD ended in a way of its own */
boolean unknown_0051055d;

/* 0x00477058, 0x0047d8f4, 0x00479780: three things that have a block,
which start to be read when someone signs in */
struct network_blob unknown_00477058;
struct network_blob unknown_0047d8f4;
struct network_blob unknown_00479780;

/* 0x004cf8e4: the observer of the network, for something that measures it.
0x004cf95c: someone is signed in, for the same thing */
byte *unknown_004cf8e4;
boolean unknown_004cf95c;

/* 0x00467178: the services that the game asks for when it signs in */
struct online_service unknown_00467178[NUMBER_OF_ONLINE_SERVICES]=
{
	{2, 1, 0}, {4, 1, 0}, {6, 1, 0}, {7, 1, 0}, {8, 1, 0}, {9, 1, 0}, {13, 1, 0}, {15, 1, 0}, {16, 1, 0}, {18, 1, 0},
	{19, 1, 0}, {20, 1, 0}, {0x4D530064, 0, 0}
};

/* 0x0006bc4c, 0x0006bedc, as the two functions have them written out: the
two texts of the game that go with each kind of task; one kind has none */
static const long online_task_type_strings[NUMBER_OF_ONLINE_TASK_TYPES]=
{
	0x0700061F, 0x14000620, 0x11000621, 0x15000622, 0x10000623, 0x1C000624,
	0x0E000625, 0x10000626, 0x10000627, 0x16000628, 0x10000629, 0x1200062A,
	0x1600062B, 0x1900062C, 0x1400062D, 0x1600062E, 0x1200062F, 0x11000630,
	0x0B000631, 0x0B000632, 0x0D000633, 0x13000634, 0x15000635, 0x11000636,
	0x0D000637, 0x0D000638, 0x0D000639, 0x1400063A, 0x2100063B, 0x1500063C,
	0x1F00063D, 0x1400063E, 0x1800063F, 0x0A000640, 0x10000641, 0x0E000642,
	0x1A000643, 0x1C000644, 0x1A000645, 0x1D000646, NONE, 0x1F000647,
	0x21000648, 0x20000649, 0x2200064A, 0x0F00064B
};
static const long online_task_type_other_strings[NUMBER_OF_ONLINE_TASK_TYPES]=
{
	0x0700064C, 0x1400064D, 0x1100064E, 0x1500064F, 0x10000650, 0x1C000651,
	0x0E000652, 0x10000653, 0x10000654, 0x16000655, 0x10000656, 0x12000657,
	0x16000658, 0x19000659, 0x1400065A, 0x1600065B, 0x1200065C, 0x1100065D,
	0x0B00065E, 0x0B00065F, 0x0D000660, 0x13000661, 0x15000662, 0x11000663,
	0x0D000664, 0x0D000665, 0x0D000666, 0x14000667, 0x21000668, 0x15000669,
	0x1F00066A, 0x1400066B, 0x1800066C, 0x0A00066D, 0x1000066E, 0x0E00066F,
	0x1A000670, 0x1C000671, 0x1A000672, 0x1D000673, NONE, 0x1F000674,
	0x21000675, 0x20000676, 0x22000677, 0x0F000678
};

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
			online_presence_unknown_0008c550(index);
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
		if (!online_task_update((task->identifier<<16)|absolute_index, task))
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

/* what is said of how signing in went for some players, from what it says
of each */
static inline __attribute__((always_inline)) boolean online_users_result_add(
	long user_result,
	long *result)
{
	if (user_result)
	{
		if (user_result==ONLINE_USER_RESULT_UNKNOWN1512F0)
		{
			*result= _online_users_result_unknown1;
		}
		else if (user_result==(long)ONLINE_USER_RESULT_UNKNOWN80151200)
		{
			*result= _online_users_result_unknown2;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0006c6c0: how signing in went for the players that are signed in: 2 if
for one of them it ended in the second way, 1 if in the first, and nothing
if not */
long online_users_get_result(
	void)
{
	struct online_user users[MAXIMUM_ONLINE_USERS];
	long result= _online_users_result_none;
	long index;

	online_get_logon_users(users);
	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		if ((users[index].xuid[0]|users[index].xuid[1]) && online_users_result_add(users[index].result, &result))
		{
			break;
		}
	}

	return result;
}

/* 0x0006c750: the same, of the task that changes who is signed in */
long online_task_users_get_result(
	const struct online_task *task)
{
	long results[MAXIMUM_ONLINE_USERS];
	long result= _online_users_result_none;
	long index;

	XOnlineChangeLogonUsersTaskGetResults(task->handle, results);
	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		if (online_users_result_add(results[index], &result))
		{
			break;
		}
	}

	return result;
}

/* 0x0006c7a0: the numbers of the services that the game asks for */
void online_services_get_identifiers(
	unsigned long *identifiers)
{
	long index;

	for (index= 0; index<NUMBER_OF_ONLINE_SERVICES; index++)
	{
		identifiers[index]= unknown_00467178[index].identifier;
	}
}

/* 0x0006c7c0: what signing in is at, and whether there is a task that signs
in */
boolean __attribute__((stdcall)) online_logon_get_state(
	long *state_out)
{
	boolean there= unknown_00467214!=NONE;

	*state_out= online_tasks_get_logon_state(unknown_00467214);

	return there;
}

/* 0x0006c7e0: whether someone is signed in to the service of games. The
game has this, and not the call, in most of the places that ask */
boolean online_signed_in(
	void)
{
	return unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in;
}

/* 0x0006c800: something of the players that the libraries say, by a number;
nothing if they cannot. The time is asked for before and after, and not
used */
long __attribute__((stdcall)) online_unknown_0006c800(
	long unknown)
{
	long value;
	long result;

	GetTickCount();
	result= XOnlineGetUsers(unknown, &value);
	GetTickCount();

	return result<0 ? 0 : value;
}

/* 0x0006c830: whether something has the first bit of its byte of 0x1C and
neither of the first two of its byte of 0x8 */
boolean online_unknown_0006c830(
	const byte *unknown)
{
	return !(unknown[8]&3) && (unknown[0x1C]&1);
}

/* 0x0006c850: one number for another */
byte online_unknown_0006c850(
	byte code)
{
	switch (code)
	{
	case 2:
		return 5;
	case 3:
		return 6;
	case 6:
		return 9;
	case 7:
		return 10;
	case 8:
		return 1;
	case 9:
		return 2;
	case 10:
		return 3;
	case 11:
		return 4;
	}

	return 0;
}

/* 0x0006ced0: the four players that are signed in, as the libraries have
them; all nothing if they have none */
void online_get_logon_users(
	struct online_user *users)
{
	const struct online_user *logged;

	memset(users, 0, MAXIMUM_ONLINE_USERS*sizeof(struct online_user));
	logged= XOnlineGetLogonUsers();
	if (logged)
	{
		memcpy(users, logged, MAXIMUM_ONLINE_USERS*sizeof(struct online_user));
	}
}

/* 0x0006d040: the others are told what a player is at, if someone is signed
in: its number, some bits, the session that it is in, if any, and four bytes
more */
void __attribute__((stdcall)) online_notification_set_state(
	const unsigned long *session_identifier,
	long user_index,
	byte *state,
	long flags)
{
	if (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in)
	{
		unsigned long low= 0;
		unsigned long high= 0;

		if (session_identifier)
		{
			low= session_identifier[0];
			high= session_identifier[1];
		}
		XOnlineNotificationSetState(user_index, flags, low, high, ONLINE_NOTIFICATION_STATE_SIZE, state);
	}
}

/* 0x0006c450: a task goes on a little, and its bits say how it went: if it
could not, it is not going on and is done, badly (0x24); with 0x1500F1 it
has its bit of 0x2; with nothing or 0x1500F2 it is as it was; and with
anything else it is not going on and is done (0x4). What the libraries said */
long online_task_step(
	struct online_task *task)
{
	long result= online_task_continue(task);

	if (result<0)
	{
		task->flags= (task->flags&~_online_task_unknown1_bit)|_online_task_unknown4_bit|_online_task_unknown20_bit;
	}
	else if (result==ONLINE_TASK_UNKNOWN1500F1)
	{
		task->flags|= _online_task_unknown2_bit;
	}
	else if (result!=0 && result!=ONLINE_TASK_DONE)
	{
		task->flags= (task->flags&~_online_task_unknown1_bit)|_online_task_unknown4_bit;
	}

	return result;
}

/* 0x0006c4a0: a task of the storage ended, with what the libraries said:
the task of the blocks that it did is told (network_blob_task_completed) how,
in the same numbers as when something of the storage cannot start, and
nothing if it went well */
void online_task_storage_done(
	long result,
	long index)
{
	long status;

	switch ((unsigned long)result)
	{
	case 0:
	case ONLINE_TASK_UNKNOWN1500F0:
		status= 0;
		break;
	case 0x8007000E:
	case 0x80150002:
	case 0x80150003:
	case 0x80150005:
	case 0x80150006:
	case 0x80150008:
		status= 2;
		break;
	case 0x80150004:
	case 0x8015C002:
		status= 3;
		break;
	case 0x8015C006:
	case 0x8015C008:
	case 0x8015C009:
		status= 4;
		break;
	case 0x8015C004:
	case 0x8015C007:
		status= 5;
		break;
	default:
		status= 1;
		break;
	}
	network_blob_task_completed(status, index);
}

/* 0x0006c560: what a task does each time around; whether the others are to
go on. One that is going on and not done does what its kind has: the one
that signs in and the one that changes who is signed in, their own; the
others go on a little (online_task_step), and then one of the second kind
tells of it if it could not, one of the kind 0xD sets a byte if it is done
with 0x153101, and one of the storage that is not going on any more tells
the task of the blocks. Then a task that is done and has its bit of 0x8 has
the one of 0x10 instead, and one that has the one of 0x10 is done with: so
there is one time around between the two */
boolean online_task_update(
	long index,
	struct online_task *task)
{
	boolean result= TRUE;

	if ((task->flags&_online_task_unknown1_bit) && !(task->flags&_online_task_unknown4_bit))
	{
		long said;

		switch (task->type)
		{
		case _online_task_type0:
			result= online_task_logon_update(task, index);
			break;
		case _online_task_type1:
			online_task_change_users_update(task);
			break;
		case _online_task_type2:
			said= online_task_step(task);
			if (said<0 && said!=(long)ONLINE_TASK_NOT_SIGNED_IN)
			{
				online_friends_startup();
			}
			break;
		case _online_task_type_d:
			said= online_task_step(task);
			if ((task->flags&_online_task_unknown4_bit) && said==ONLINE_TASK_UNKNOWN153101)
			{
				unknown_0051055d= TRUE;
			}
			break;
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
			said= online_task_step(task);
			if (!(task->flags&_online_task_unknown1_bit))
			{
				online_task_storage_done(said, index);
			}
			break;
		default:
			online_task_step(task);
			break;
		}
	}
	if (task->flags&_online_task_unknown4_bit)
	{
		if (task->flags&_online_task_unknown8_bit)
		{
			task->flags= (task->flags&~_online_task_unknown8_bit)|_online_task_unknown10_bit;
		}
		else if (task->flags&_online_task_unknown10_bit)
		{
			online_task_close(index);
		}
	}

	return result;
}

/* the task that signs in, or changes who is, ended and not well: what it
ended at, if it is not known, is 10 */
static inline __attribute__((always_inline)) void online_task_logon_failed(
	struct online_task *task)
{
	task->flags|= _online_task_unknown4_bit|_online_task_unknown20_bit;
	if (task->unknown10==_online_logon_state_going || task->unknown10==_online_session_signed_in)
	{
		task->unknown10= _online_logon_state_none;
	}
}

/* 0x0006c2a0: what the task that signs in does each time around; whether
the other tasks are to go on, which they are not when it is done or signing
in could not be. Without the network of the console it is done. It goes on a
little, and what signing in is at is looked at: when someone is signed in
for the first time, the observer and something that measures the network
are told, each thing that asked is told its third thing
(transport_unknown_0008d730), and what is done then is done
(online_logon_complete): if that cannot be, the task ends, but the others go on */
boolean __attribute__((stdcall)) online_task_logon_update(
	struct online_task *task,
	long index)
{
	long state;

	if (!(unknown_004d8b18 && unknown_004d8b19))
	{
		task->flags|= _online_task_unknown4_bit;
	}
	if (task->flags&_online_task_unknown4_bit)
	{
		return FALSE;
	}
	online_task_continue(task);
	state= online_tasks_get_logon_state(index);
	if (state==_online_logon_state_going)
	{
		return TRUE;
	}
	if (state!=_online_session_signed_in)
	{
		online_task_logon_failed(task);

		return FALSE;
	}
	if (!(task->flags&_online_task_unknown2_bit))
	{
		unknown_004cf8e4[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET]= TRUE;
		unknown_004cf95c= TRUE;
		unknown_0007f410();
		transport_unknown_0008d730();
		if (online_logon_complete(task))
		{
			task->flags|= _online_task_unknown2_bit;
		}
		else
		{
			online_task_logon_failed(task);
		}
	}

	return TRUE;
}

/* 0x0006c360: what the task that changes who is signed in does each time
around; whether it goes on well. It goes on a little: while the libraries
say that it goes on, it is at nothing; when they say that it is done, it is
at 1 and what is done when someone is signed in is done; and when they say
why it could not be, it ends at what that is. With 0x80151300 the players
are looked at (online_task_users_get_result) */
boolean online_task_change_users_update(
	struct online_task *task)
{
	long result= online_task_continue(task);

	switch ((unsigned long)result)
	{
	case 0:
	case ONLINE_TASK_USERS_GOING:
		task->unknown10= _online_logon_state_going;
		return TRUE;
	case ONLINE_TASK_USERS_DONE:
		task->flags|= _online_task_unknown4_bit;
		task->unknown10= _online_session_signed_in;
		if (online_logon_complete(task))
		{
			return TRUE;
		}
		online_task_logon_failed(task);
		return FALSE;
	case ONLINE_TASK_USERS_LOOK:
		task->unknown10= online_task_users_get_result(task)==_online_users_result_unknown2 ? _online_logon_state2 :
			_online_logon_state_none;
		break;
	case 0x80151001:
		task->unknown10= _online_logon_state4;
		break;
	case 0x80151002:
		task->unknown10= _online_logon_state3;
		break;
	case 0x80151003:
		task->unknown10= _online_logon_state8;
		break;
	case 0x80151006:
		task->unknown10= _online_logon_state6;
		break;
	default:
		task->unknown10= _online_logon_state_none;
		break;
	}
	task->flags|= _online_task_unknown4_bit|_online_task_unknown20_bit;

	return FALSE;
}

/* 0x0006bff0: what is done when someone is signed in, of the task that did
it; whether it could be, and the task is at what says so: 5 if no one is
signed in after all, and 10 if the libraries have no players or the service
does not have something that the game cannot do without. If signing in ended
in the second way for a player, the task ends at 2. Each service that the
game asks for is seen to be there or not; the players are looked at; the
voice starts (game_voice_start); and, for the task that signs in, the three
blocks of unknown_00477058, unknown_0047d8f4 and unknown_00479780 start to
be read, for the first player that something is so of */
boolean __attribute__((stdcall)) online_logon_complete(
	struct online_task *task)
{
	const struct online_user *users;
	long state= _online_session_signed_in;
	long users_result;
	long index;

	if (!(unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in))
	{
		state= _online_logon_state5;
		goto done;
	}
	users= XOnlineGetLogonUsers();
	if (!users)
	{
		state= _online_logon_state_none;
		goto done;
	}
	users_result= task->type==_online_task_type0 ? online_users_get_result() : online_task_users_get_result(task);
	if (users_result==_online_users_result_unknown1)
	{
		unknown_0050944f= TRUE;
	}
	else if (users_result==_online_users_result_unknown2)
	{
		task->flags|= _online_task_unknown20_bit;
		state= _online_logon_state2;
		goto done;
	}
	for (index= 0; index<NUMBER_OF_ONLINE_SERVICES; index++)
	{
		if (XOnlineGetServiceInfo(unknown_00467178[index].identifier, NULL)>=0)
		{
			unknown_00467178[index].available= TRUE;
		}
		else
		{
			unknown_00467178[index].available= FALSE;
			if (unknown_00467178[index].required)
			{
				state= _online_logon_state_none;
				goto done;
			}
		}
	}
	online_users_start_mute_lists(users);
	online_users_set_state(users);
	game_voice_start();
	index= user_interface_controllers_first_online_user(users);
	unknown_0008c660();
	if (task->type==_online_task_type0)
	{
		struct network_blob_request request= {{0, 0, 0}};

		network_blob_start_read(&unknown_00477058, index, request, 0);
		network_blob_start_read(&unknown_0047d8f4, index, request, 0);
		network_blob_start_read(&unknown_00479780, index, request, 0);
		online_friends_startup();
		online_new_content_check_start();
	}
done:
	task->unknown10= state;

	return state==_online_session_signed_in;
}

/* 0x0006c170: for each player that is signed in well, and is one like the
others, the list of the players that it has muted starts over and is asked
for (voice_mute_list_stop, voice_mute_list_startup, voice_mute_list_request) */
void online_users_start_mute_lists(
	const struct online_user *users)
{
	long index;

	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		const struct online_user *user= &users[index];

		if ((user->xuid[0]|user->xuid[1]) && user->name[0] && user->result>=0 && !(user->flags&_online_user_unknown_bits))
		{
			voice_mute_list_stop(index);
			voice_mute_list_startup(index);
			voice_mute_list_request(index);
		}
	}
}

/* 0x0006c1d0: the others are told that each player that is signed in well,
and is one like the others, is at nothing in particular: of the first kind,
in no session, and with no first time (online_notification_set_state, which
the game has inside) */
void __attribute__((stdcall)) online_users_set_state(
	const struct online_user *users)
{
	long index;

	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		const struct online_user *user= &users[index];

		if ((user->xuid[0]|user->xuid[1]) && user->name[0] && user->result>=0 && !(user->flags&_online_user_unknown_bits))
		{
			unsigned long session_identifier[2]= {0, 0};
			struct online_state state;
			unsigned long encoded;

			state.kind= 1;
			state.unknown4= 0;
			state.unknown8= 0;
			state.time= 0;
			state.second_time= 0x7FFF;
			online_state_encode(&encoded, &state);
			online_notification_set_state(session_identifier, index, (byte *)&encoded, 0);
		}
	}
}

/* a time as a number of seven bits: itself up to 60, then one more each 5
up to 180, then one more each 10 up to 600, and 126 from there */
static inline __attribute__((always_inline)) byte online_state_time(
	short time)
{
	if (time<60)
	{
		return (byte)time;
	}
	if (time<180)
	{
		return (byte)((time-60)/5+60);
	}
	if (time<600)
	{
		return (byte)((time-180)/10+84);
	}

	return ONLINE_STATE_TIME_MAXIMUM;
}

/* 0x0006cf00: what a player is at, as the four bytes that the others are
told: 0xF1 and 0xFC; the first time, or with none the second and 127 more;
and a byte with a number for the kind in its low bits, the first number in
its bits of 0x30 and the second in the ones of 0xC0 */
void online_state_encode(
	unsigned long *encoded,
	const struct online_state *state)
{
	byte *bytes= (byte *)encoded;
	byte time= 0;
	byte kind= 0;

	*encoded= ONLINE_STATE_SIGNATURE;
	if (state->time>0)
	{
		time= online_state_time(state->time);
	}
	else if (state->second_time>0)
	{
		time= (byte)(online_state_time(state->second_time)+ONLINE_STATE_SECOND_TIME);
	}
	bytes[2]= time;
	switch (state->kind)
	{
	case 1:
		kind= 0;
		break;
	case 2:
		kind= 4;
		break;
	case 3:
		kind= 8;
		break;
	case 4:
		kind= 12;
		break;
	case 5:
		kind= 1;
		break;
	case 6:
		kind= 3;
		break;
	}
	bytes[3]= kind;
	*encoded= (*encoded&0x3FFFFFFF)|((unsigned long)state->unknown8<<30);
	*encoded= (*encoded&~0x30000000UL)|(((unsigned long)state->unknown4<<28)&0x30000000);
}

/* 0x0006d080: what the service of games says that a player of this console
is at; whether it is known. Not for a player that is not like the others,
nor if no one is signed in. Of the things that the player has, the one that
is known by the same as what the service says is given too, if there is one */
boolean online_user_get_state(
	long user_index,
	struct online_user_state *state)
{
	byte entries[MAXIMUM_ONLINE_USER_ENTRIES][ONLINE_USER_ENTRY_SIZE];
	byte said[12];
	unsigned long flags;
	long unknown;
	long count;
	long index;

	if (USER_INTERFACE_CONTROLLER(user_index)->user.flags&_online_user_unknown_bits)
	{
		return FALSE;
	}
	if (!(unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in))
	{
		return FALSE;
	}
	if (!unknown_003a0798(user_index, said, &unknown))
	{
		return FALSE;
	}
	switch (said[0])
	{
	case 2:
		state->kind= 1;
		break;
	case 3:
		state->kind= 2;
		break;
	case 4:
		state->kind= 3;
		break;
	case 5:
		state->kind= 4;
		break;
	case 6:
		state->kind= 5;
		break;
	case 7:
		state->kind= 6;
		break;
	default:
		state->kind= 0;
		break;
	}
	memcpy(&state->identifier, said+1, sizeof(state->identifier));
	memset(state->entry, 0, sizeof(state->entry));
	memcpy(&flags, said+5, sizeof(flags));
	state->unknown48= (flags>>3)&1;
	state->unknown49= (flags>>2)&1;
	state->unknown4a= (flags>>8)&1;
	state->unknown4b= (flags&0xFF000000)!=0;
	count= MAXIMUM_ONLINE_USER_ENTRIES;
	unknown_0008e750(user_index, entries[0], &count);
	for (index= 0; index<count; index++)
	{
		if (*(const unsigned long *)(entries[index]+ONLINE_USER_ENTRY_IDENTIFIER_OFFSET)==state->identifier)
		{
			memcpy(state->entry, entries[index], sizeof(state->entry));
			break;
		}
	}

	return TRUE;
}

/* 0x0006cb60: no one is signed in to the service of games any more: if
there was a task that signs in, the tasks of the blocks all end, the online
tasks are all done with, and there is no such task; each gamepad is told;
the observer and what measures the network are told; each thing that asked
is told its third thing (transport_unknown_0008d730, which the game has
inside); and the three blocks that are read when someone signs in are not
good any more */
void online_logoff(
	void)
{
	long index;

	if (unknown_00467214!=NONE)
	{
		for (index= 0; index<MAXIMUM_NETWORK_BLOB_TASKS; index++)
		{
			network_blob_task_end(index, _network_blob_task_no_memory);
		}
		online_tasks_close_all();
		unknown_00467214= NONE;
	}
	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		user_interface_controller_sign_out(index);
	}
	unknown_004cf8e4[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET]= FALSE;
	unknown_004cf95c= FALSE;
	unknown_0007f410();
	transport_unknown_0008d730();
	unknown_00479780.flags&= ~_network_blob_good_bit;
	unknown_0047d8f4.flags&= ~_network_blob_good_bit;
	unknown_00477058.flags&= ~_network_blob_good_bit;
}

/* 0x0006b590: the name of a game, in the language of the console, as the
service of games has it; a question mark if it does not */
void __attribute__((stdcall)) online_title_get_name(
	unsigned long title,
	wchar *name,
	long size)
{
	if (XOnlineFriendsGetTitleName(title, XGetLanguage(), size, name)<0)
	{
		wcsncpy(name, (const wchar *)u"?", size-1);
		name[size-1]= 0;
	}
}

/* 0x0006d210: the invitation to a game that the player took before the game
started; whether there is one */
boolean online_get_accepted_game_invite(
	void *invite)
{
	return XOnlineFriendsGetAcceptedGameInvite(invite)>=0;
}

/* 0x0006d220: if the service of games has the service of 0x14, as was seen
when someone signed in, the address of the network that it has is let go
of */
void online_service_unregister_address(
	void)
{
	struct online_service_info info;
	boolean available= FALSE;
	long index;

	for (index= 0; index<NUMBER_OF_ONLINE_SERVICES; index++)
	{
		if (unknown_00467178[index].identifier==_online_service14)
		{
			available= unknown_00467178[index].available;
		}
	}
	if (available && XOnlineGetServiceInfo(_online_service14, &info)>=0)
	{
		XNetUnregisterInAddr(info.address);
	}
}

/* 0x0006cc10: the player of a gamepad is taken out of the ones that are
signed in to the service of games: its list of friends and its tasks are
done with, and a task starts that signs in the others again without it; the
task, or NONE if it could not start. If no one else is signed in, everyone
is signed out. Nothing if no one was signed in */
long __attribute__((stdcall)) online_logon_remove_user(
	long user_index)
{
	struct online_user users[MAXIMUM_ONLINE_USERS];
	struct online_task *task;
	long task_index;
	long count;
	long index;

	if (unknown_00467214==NONE)
	{
		return NONE;
	}
	task_index= online_task_find(user_index, _online_task_type_friends);
	if (task_index!=NONE)
	{
		online_task_close(task_index);
	}
	user_interface_controller_close_tasks(user_index);
	memset(users, 0, sizeof(users));
	count= 0;
	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if (index!=user_index && ((controller->flags>>_user_interface_controller_online_bit)&1))
		{
			users[index]= controller->user;
			count++;
		}
	}
	if (count<=0)
	{
		online_logoff();

		return NONE;
	}
	task_index= online_task_new_signed_in();
	task= (struct online_task *)datum_try_and_get(online_tasks, task_index);
	if (!task)
	{
		return NONE;
	}
	online_service_unregister_address();
	if (XOnlineChangeLogonUsers(users, 0, &task->handle)>=0)
	{
		task->flags= _online_task_unknown1_bit;
		task->type= _online_task_type1;
		task->owner= user_index;

		return task_index;
	}
	online_task_close(task_index);

	return NONE;
}

/* 0x0006ba80: the text of the game that goes with a task, by its kind;
nothing if there is no such task, and NONE for a kind that has none */
long online_task_get_string(
	long index)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, index);

	if (!task)
	{
		return 0;
	}
	if ((unsigned long)task->type>=NUMBER_OF_ONLINE_TASK_TYPES)
	{
		return NONE;
	}

	return online_task_type_strings[task->type];
}

/* 0x0006bd10: the other text of the game that goes with a task, as the
function before this one */
long online_task_get_other_string(
	long index)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, index);

	if (!task)
	{
		return 0;
	}
	if ((unsigned long)task->type>=NUMBER_OF_ONLINE_TASK_TYPES)
	{
		return NONE;
	}

	return online_task_type_other_strings[task->type];
}

/* 0x0006c8b0: the player of a gamepad signs in to the service of games; the
task that does it, or NONE if it could not start. If no one was signed in, it
is a task that signs in, for the services of the game, and it is the task
that the game keeps (unknown_00467214); if someone was, a task that signs in
again those that are with this one more. Nothing if the network is not up
(unknown_004d8b18, unknown_004d8b19) */
long online_logon_add_user(
	const struct online_user *user,
	long user_index)
{
	struct online_user users[MAXIMUM_ONLINE_USERS];
	unsigned long services[NUMBER_OF_ONLINE_SERVICES];
	struct online_task *task;
	long task_index;
	long index;

	if (!unknown_004d8b18 || !unknown_004d8b19)
	{
		return NONE;
	}
	if (unknown_00467214==NONE)
	{
		memset(users, 0, sizeof(users));
		users[user_index]= *user;
		unknown_00467214= online_task_new();
		task= online_task_get(unknown_00467214);
		if (!task)
		{
			return NONE;
		}
		online_services_get_identifiers(services);
		unknown_0050944f= FALSE;
		if (XOnlineLogon(users, services, NUMBER_OF_ONLINE_SERVICES, 0, &task->handle)>=0)
		{
			task->flags= _online_task_unknown1_bit;
			task->type= _online_task_type0;
			task->owner= NONE;

			return unknown_00467214;
		}
		online_task_close(unknown_00467214);
		unknown_00467214= NONE;

		return NONE;
	}
	memset(users, 0, sizeof(users));
	users[user_index]= *user;
	for (index= 0; index<MAXIMUM_ONLINE_USERS; index++)
	{
		const struct user_interface_controller *controller= USER_INTERFACE_CONTROLLER(index);

		if (index!=user_index && ((controller->flags>>_user_interface_controller_online_bit)&1))
		{
			users[index]= controller->user;
		}
	}
	task_index= online_task_new_signed_in();
	task= (struct online_task *)datum_try_and_get(online_tasks, task_index);
	if (!task)
	{
		return NONE;
	}
	unknown_0050944f= FALSE;
	online_service_unregister_address();
	if (XOnlineChangeLogonUsers(users, 0, &task->handle)>=0)
	{
		task->flags= _online_task_unknown1_bit;
		task->type= _online_task_type1;
		task->owner= user_index;

		return task_index;
	}
	online_task_close(task_index);

	return NONE;
}

/* 0x0008d4d0: a task starts that asks the service of games whether there is
new content for the game, of the kind 0xD and of no one's; the task, or NONE
if it could not start. The game has it far from the rest of this file */
long online_new_content_check_start(
	void)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		unknown_0051055d= FALSE;
		if (XOnlineOfferingIsNewContentAvailable(NONE, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		task->flags= _online_task_unknown1_bit|_online_task_unknown8_bit;
		task->type= _online_task_type_d;
		task->owner= NONE;
	}

	return task_index;
}
