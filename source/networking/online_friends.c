/*
ONLINE_FRIENDS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The friends of a player in the service of games. The names are guesses.
*/

#include "online_friends.h"

#include "networking/online_tasks.h"

#include <string.h>

/* ---------- prototypes */

/* 0x003a075e, of the console's libraries: whether a game is this one, or
one that counts as it */
long __attribute__((stdcall)) XOnlineTitleIdIsSameTitle(unsigned long title);
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* 0x003a07da, 0x003a0806, 0x003a0811, 0x003a07cf, of the console's
libraries: a friend is not one any more; someone who asks to be a friend is
answered; an invitation to a game is answered; and the name of a game, in a
language. 0x0033039a: the language of the console */
long __attribute__((stdcall)) XOnlineFriendsRemove(long owner, const byte *online_friend);
long __attribute__((stdcall)) XOnlineFriendsAnswerRequest(long owner, const byte *online_friend, long answer);
long __attribute__((stdcall)) XOnlineFriendsAnswerGameInvite(long owner, const byte *online_friend, long answer);
long __attribute__((stdcall)) XOnlineFriendsGetTitleName(unsigned long title, long language, long size, wchar *name);
long XGetLanguage(void);

/* 0x003a07ae, 0x003a07c4, of the console's libraries: a task starts that
keeps the list of the friends of a player; and the friends that there are
now, and how many */
long __attribute__((stdcall)) XOnlineFriendsEnumerate(long owner, long unknown, long *handle);
long __attribute__((stdcall)) XOnlineFriendsGetLatest(long owner, long maximum_count, byte *friends);

/* 0x003a07a3: the friends of the libraries start; a task of theirs does
it */
long __attribute__((stdcall)) XOnlineFriendsStartup(long unknown, long *handle);

/* 0x003a0769: whether a game is of who made this one */
long __attribute__((stdcall)) XOnlineTitleIdIsSamePublisher(unsigned long title);

/* 0x003a0890: a player says something of another to the service of games; a
task of the libraries does it */
long __attribute__((stdcall)) XOnlineFeedbackSend(long owner, struct online_xuid target, long type,
	const wchar *nickname, long unknown, long *handle);

/* ---------- globals */

/* 0x00467214, of main_game.c: the task that signs in to the service of
games, or NONE */
extern long unknown_00467214;

/* ---------- code */

/* whether someone is signed in to the service of games, as the game has it
written out in each function of this file that asks the libraries */
static inline __attribute__((always_inline)) boolean online_friends_signed_in(
	void)
{
	return unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_session_signed_in;
}

/* the copy of a friend that the libraries are given: of what it is at, only
the highest bit */
static inline __attribute__((always_inline)) void online_friend_copy_for_library(
	byte *copy,
	const byte *online_friend)
{
	unsigned long state;

	memcpy(copy, online_friend, ONLINE_FRIEND_SIZE);
	memcpy(&state, copy+ONLINE_FRIEND_STATE_OFFSET, sizeof(state));
	state&= ONLINE_FRIEND_STATE_UNKNOWN80000000;
	memcpy(copy+ONLINE_FRIEND_STATE_OFFSET, &state, sizeof(state));
}

/* 0x0008c540: whether a game is this one, or one that counts as it */
boolean online_title_is_same_title(
	unsigned long title)
{
	return XOnlineTitleIdIsSameTitle(title)!=0;
}

/* 0x0008d190: whether the game that a friend plays is this one */
boolean online_friend_is_same_title(
	const byte *online_friend)
{
	return XOnlineTitleIdIsSameTitle(*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET))!=0;
}

/* 0x0008cc00: the 32 texts of something are nothing */
void network_motd_texts_clear(
	byte *unknown)
{
	wchar *text= (wchar *)(unknown+ONLINE_UNKNOWN_0008CC00_TEXTS_OFFSET);
	long index;

	for (index= 0; index<NUMBER_OF_ONLINE_UNKNOWN_0008CC00_TEXTS; index++)
	{
		text[0]= 0;
		text+= ONLINE_UNKNOWN_0008CC00_TEXT_LENGTH+1;
	}
}

/* 0x0008cc20: one of those texts becomes another, cut to what fits */
void network_motd_text_copy(
	wchar *destination,
	const wchar *source)
{
	wcsncpy(destination, source, ONLINE_UNKNOWN_0008CC00_TEXT_LENGTH);
	destination[ONLINE_UNKNOWN_0008CC00_TEXT_LENGTH]= 0;
}

/* 0x0008d1f0: who a friend is and its name, as the game keeps them: 0x56
bytes, of which the rest are nothing */
void online_friend_get_identity(
	const byte *online_friend,
	byte *identity)
{
	memset(identity, 0, ONLINE_FRIEND_IDENTITY_SIZE);
	memcpy(identity, online_friend, ONLINE_FRIEND_IDENTIFIER_SIZE);
	strncpy((char *)identity+ONLINE_FRIEND_NAME_OFFSET, (const char *)online_friend+ONLINE_FRIEND_NAME_OFFSET,
		ONLINE_FRIEND_NAME_LENGTH+1);
	identity[ONLINE_FRIEND_NAME_OFFSET+ONLINE_FRIEND_NAME_LENGTH]= 0;
}

/* 0x0008cf00: what a friend is at, as the game has it, from what the
libraries say and the game that it plays: whether that is this game is only
asked of one that has the first bit, and only said of one that has the bit
of 0x8000000 */
long __attribute__((stdcall)) online_friend_state_from_library(
	unsigned long library_state,
	unsigned long title)
{
	long state= 0;
	boolean same_title= FALSE;

	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN1)
	{
		same_title= XOnlineTitleIdIsSameTitle(title)!=0;
		state= _online_friend_unknown1;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN10)
	{
		state|= _online_friend_unknown8;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN80000000)
	{
		state|= _online_friend_unknown10;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN40000000)
	{
		state|= _online_friend_unknown20;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN2)
	{
		state|= _online_friend_unknown4;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN8000000)
	{
		state|= _online_friend_unknown80;
		if (same_title)
		{
			state|= _online_friend_same_title;
		}
		else
		{
			state&= ~_online_friend_same_title;
		}
	}
	if ((library_state&ONLINE_FRIEND_STATE_UNKNOWN4000000) && !(library_state&ONLINE_FRIEND_STATE_UNKNOWN20000000))
	{
		state|= _online_friend_unknown100;
	}
	if (library_state&ONLINE_FRIEND_STATE_UNKNOWN8)
	{
		state|= _online_friend_unknown2;
	}

	return state;
}

/* 0x0008cf80: what a friend is at, as the libraries have it, from what the
game has */
unsigned long online_friend_state_to_library(
	unsigned long state)
{
	unsigned long library_state= 0;

	if (state&_online_friend_unknown1)
	{
		library_state= ONLINE_FRIEND_STATE_UNKNOWN1;
	}
	if (state&_online_friend_unknown8)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN10;
	}
	if ((state&_online_friend_unknown10) || (state&_online_friend_unknown400))
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN80000000;
	}
	if (state&_online_friend_unknown20)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN40000000;
	}
	if (state&_online_friend_unknown4)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN2;
	}
	if (state&_online_friend_unknown80)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN8000000;
	}
	if (state&_online_friend_unknown100)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN4000000;
	}
	if (state&_online_friend_unknown2)
	{
		library_state|= ONLINE_FRIEND_STATE_UNKNOWN8;
	}

	return library_state;
}

/* 0x0008ceb0: what a friend is at, as the game has it */
long online_friend_get_state(
	const byte *online_friend)
{
	return online_friend_state_from_library(*(const unsigned long *)(online_friend+ONLINE_FRIEND_STATE_OFFSET),
		*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET));
}

/* 0x0008cec0: what the game says of a friend: who it is, what it is at, two
numbers and the game that it plays */
void online_friend_get_summary(
	const byte *online_friend,
	struct online_friend_summary *summary)
{
	memcpy(summary->identifier, online_friend, sizeof(summary->identifier));
	summary->state= online_friend_state_from_library(
		*(const unsigned long *)(online_friend+ONLINE_FRIEND_STATE_OFFSET),
		*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET));
	summary->unknown10= *(const long *)(online_friend+ONLINE_FRIEND_UNKNOWN28_OFFSET);
	summary->unknown14= *(const long *)(online_friend+ONLINE_FRIEND_UNKNOWN28_OFFSET+4);
	summary->title= *(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET);
	summary->unknown1C= FALSE;
}

/* 0x0008cdf0: a friend of a player is not one any more; nothing if no one
is signed in */
void __attribute__((stdcall)) online_friend_remove(
	const byte *online_friend,
	long owner)
{
	byte copy[ONLINE_FRIEND_SIZE];

	online_friend_copy_for_library(copy, online_friend);
	if (online_friends_signed_in())
	{
		XOnlineFriendsRemove(owner, copy);
	}
}

/* 0x0008ce40: someone who asks to be the friend of a player is given one of
three answers; nothing if no one is signed in */
void __attribute__((stdcall)) online_friend_answer_request(
	long answer,
	const byte *online_friend,
	long owner)
{
	byte copy[ONLINE_FRIEND_SIZE];
	long said;

	online_friend_copy_for_library(copy, online_friend);
	switch (answer)
	{
	case 0:
		said= 0;
		break;
	case 1:
		said= 1;
		break;
	default:
		said= NUMBER_OF_ONLINE_FRIEND_ANSWERS-1;
		break;
	}
	if (online_friends_signed_in())
	{
		XOnlineFriendsAnswerRequest(owner, copy, said);
	}
}

/* 0x0008d050: the invitation to a game of a friend of a player is given one
of three answers, if the friend has the two bits of one that invites; nothing
if no one is signed in */
void __attribute__((stdcall)) online_friend_answer_game_invite(
	const byte *online_friend,
	long owner,
	long answer)
{
	long answers[NUMBER_OF_ONLINE_FRIEND_ANSWERS]= { 1, 0, 2 };

	if (online_friends_signed_in())
	{
		unsigned long state= *(const unsigned long *)(online_friend+ONLINE_FRIEND_STATE_OFFSET);

		if ((state&ONLINE_FRIEND_STATE_UNKNOWN8000000) && (state&ONLINE_FRIEND_STATE_UNKNOWN1))
		{
			XOnlineFriendsAnswerGameInvite(owner, online_friend, answers[answer]);
		}
	}
}

/* 0x0008d1a0: the name of the game that a friend plays, in the language of
the console; nothing if it has not the first bit of what it is at, if no one
is signed in, or if the libraries do not know */
void __attribute__((stdcall)) online_friend_get_title_name(
	wchar *name,
	const byte *online_friend,
	short size)
{
	name[0]= 0;
	if ((online_friend_get_state(online_friend)&_online_friend_unknown1) && online_friends_signed_in())
	{
		if (XOnlineFriendsGetTitleName(*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET),
			XGetLanguage(), size, name)<0)
		{
			name[0]= 0;
		}
	}
}

/* 0x0008cce0: a task starts that keeps the list of the friends of a player;
the task, or NONE if it could not start */
long online_friends_enumerate_start(
	long owner)
{
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		if (XOnlineFriendsEnumerate(owner, 0, &task->handle)<0)
		{
			online_task_close(task_index);

			return NONE;
		}
		task->flags= _online_task_unknown1_bit;
		task->type= _online_task_type_friends;
		task->owner= owner;
	}

	return task_index;
}

/* 0x0008cd60: the friends that the player of a task has now, up to a
hundred, and how many; none if there is no such task, no one is signed in,
or the task is not at its second or third step */
short __attribute__((stdcall)) online_friends_get_latest(
	long task_index,
	byte *friends)
{
	const struct online_task *task= (const struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task && online_friends_signed_in() &&
		(online_task_get_status(task_index)==_online_task_status1 ||
		online_task_get_status(task_index)==_online_task_status2))
	{
		return (short)XOnlineFriendsGetLatest(task->owner, MAXIMUM_ONLINE_FRIENDS, friends);
	}

	return 0;
}

/* 0x0008cc40: the task of the friends, of the second kind and of no one's:
if there is one and it is not at its third step, that one; if it is, it
starts again; and if there is none, a new one starts. The task, or NONE if it
could not start. The game does not look at whether the task that it found is
one */
long online_friends_startup(
	void)
{
	long task_index= online_task_find(ONLINE_FRIENDS_TASK_OWNER, _online_task_type2);
	struct online_task *task;

	if (task_index!=NONE)
	{
		task= (struct online_task *)online_tasks->data+(task_index&0xFFFF);
		if (!(task->flags&(_online_task_unknown4_bit|_online_task_unknown20_bit)))
		{
			return task_index;
		}
		online_task_restart(task_index);
		task= (struct online_task *)online_tasks->data+(task_index&0xFFFF);
	}
	else
	{
		task_index= online_task_new_signed_in();
		if (task_index==NONE)
		{
			return NONE;
		}
		task= (struct online_task *)online_tasks->data+(task_index&0xFFFF);
	}
	if (XOnlineFriendsStartup(0, &task->handle)<0)
	{
		online_task_close(task_index);

		return NONE;
	}
	task->flags= _online_task_unknown1_bit;
	task->type= _online_task_type2;
	task->owner= ONLINE_FRIENDS_TASK_OWNER;

	return task_index;
}

/* 0x0008d3d0: a player says something of another to the service of games:
one of ten things, each of which is a number of the libraries'. A task of
the kind 0x16 does it; nothing if it could not start */
void __attribute__((stdcall)) online_feedback_send(
	const struct online_xuid *target,
	long owner,
	long kind)
{
	long types[NUMBER_OF_ONLINE_FEEDBACK_KINDS]= { 5, 6, 0, 1, 2, 3, 4, 12, 11, 10 };
	wchar nickname[2];
	long task_index= online_task_new_signed_in();
	struct online_task *task= (struct online_task *)datum_try_and_get(online_tasks, task_index);

	if (task)
	{
		nickname[0]= 0;
		nickname[1]= 0;
		if (XOnlineFeedbackSend(owner, *target, types[kind], nickname, 0, &task->handle)>=0)
		{
			task->flags= _online_task_unknown1_bit|_online_task_unknown8_bit;
			task->type= _online_task_type_feedback;
			task->owner= owner;
		}
		else
		{
			online_task_close(task_index);
		}
	}
}

/* 0x0008d230: what a friend is at in its game, from the four bytes that it
tells (what online_state_encode makes, the other way around); whether it
tells them. Only for a friend that has the first bit of what it is at and
plays a game of who made this one, and if someone is signed in. Its two
numbers of two bits are only given if it tells exactly four bytes */
boolean online_friend_get_online_state(
	const byte *online_friend,
	struct online_state *state)
{
	const byte *data= online_friend+ONLINE_FRIEND_STATE_DATA_OFFSET;
	unsigned long signature= ONLINE_STATE_SIGNATURE;
	byte code;
	byte flags;

	state->kind= 0;
	state->unknown4= 0;
	state->unknown8= 0;
	state->time= 0;
	state->second_time= 0;
	if (!(online_friend_state_from_library(*(const unsigned long *)(online_friend+ONLINE_FRIEND_STATE_OFFSET),
		*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET))&_online_friend_unknown1) ||
		!online_friends_signed_in() ||
		!XOnlineTitleIdIsSamePublisher(*(const unsigned long *)(online_friend+ONLINE_FRIEND_TITLE_OFFSET)) ||
		online_friend[ONLINE_FRIEND_STATE_DATA_SIZE_OFFSET]<ONLINE_FRIEND_STATE_DATA_SIZE ||
		memcmp(data, &signature, sizeof(word))!=0)
	{
		return FALSE;
	}
	code= data[2];
	if (code<=59)
	{
		state->time= code+1;
	}
	else if (code>=60 && code<=83)
	{
		state->time= (code-47)*5;
	}
	else if (code>=84 && code<=125)
	{
		state->time= (code-65)*10;
	}
	else if (code==ONLINE_STATE_TIME_MAXIMUM)
	{
		state->time= 600;
	}
	else if (code>=ONLINE_STATE_SECOND_TIME && code<=186)
	{
		state->second_time= code-ONLINE_STATE_TIME_MAXIMUM;
	}
	else if (code>=187 && code<=210)
	{
		state->second_time= (code-174)*5;
	}
	else if (code>=211 && code<=252)
	{
		state->second_time= (code-192)*10;
	}
	else
	{
		state->second_time= 600;
	}
	flags= data[3];
	if (flags&0x1)
	{
		state->kind= (flags&0x2) ? 6 : 5;
	}
	else if (flags&0x4)
	{
		state->kind= (flags&0x8) ? 4 : 2;
	}
	else
	{
		state->kind= (flags&0x8) ? 3 : 1;
	}
	if (online_friend[ONLINE_FRIEND_STATE_DATA_SIZE_OFFSET]==ONLINE_FRIEND_STATE_DATA_SIZE)
	{
		unsigned long encoded;

		memcpy(&encoded, data, sizeof(encoded));
		state->unknown4= (encoded>>28)&3;
		state->unknown8= encoded>>30;
	}

	return TRUE;
}
