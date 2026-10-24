/*
ONLINE_UNKNOWN_000B36C0.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A part of the game that uses the service of games when it has the service
0x4D530064 (what is asked for when signing in), with a table of eight
things of 0x4A bytes that each register a key of the network. What it is
for is not known, and the name of the file is only the address of its
first function.
*/

#include "cseries/cseries.h"

#include "networking/online_tasks.h"

/* ---------- constants */

enum
{
	ONLINE_UNKNOWN_000B36C0_SERVICE= 0x4D530064,
	NUMBER_OF_ONLINE_UNKNOWN_000B36C0_KEYS= 8,
	ONLINE_UNKNOWN_000B36C0_KEY_SIZE= 0x4A,
	/* in each: the key of the network */
	ONLINE_UNKNOWN_000B36C0_KEY_OFFSET= 0x30,

	/* the longest text that is measured */
	ONLINE_UNKNOWN_000B36C0_MAXIMUM_STRING_LENGTH= 0xFFF
};

/* ---------- prototypes */

/* of the console's libraries: a key and an address of the network are
forgotten */
long __attribute__((stdcall)) XNetUnregisterKey(const void *key);
long __attribute__((stdcall)) XNetUnregisterInAddr(unsigned long address);

/* ---------- globals */

/* 0x00467214 (main_game.c): the gamepad that is signed in, or NONE.
0x00467178 (online_tasks.c): the services that the game asks for when it
signs in */
extern long unknown_00467214;
extern struct online_service unknown_00467178[NUMBER_OF_ONLINE_SERVICES];

/* 0x00510588: the eight. 0x005107dc: the one in use, or NONE. 0x005107e0:
an address of the network that goes with it. 0x005107e4: how many are
registered */
byte unknown_00510588[NUMBER_OF_ONLINE_UNKNOWN_000B36C0_KEYS][ONLINE_UNKNOWN_000B36C0_KEY_SIZE];
long unknown_005107dc;
unsigned long unknown_005107e0;
long unknown_005107e4;

/* ---------- code */

/* 0x000b36c0: whether someone is signed in and the service of games has the
service 0x4D530064 (the last of the services with it says) */
long online_unknown_000b36c0_service_available(
	void)
{
	boolean available= FALSE;
	long index;

	if (unknown_00467214==NONE || online_tasks_get_logon_state(unknown_00467214)!=_online_session_signed_in)
	{
		return FALSE;
	}
	for (index= 0; index<NUMBER_OF_ONLINE_SERVICES; index++)
	{
		if (unknown_00467178[index].identifier==ONLINE_UNKNOWN_000B36C0_SERVICE)
		{
			available= unknown_00467178[index].available;
		}
	}

	return available ? TRUE : FALSE;
}

/* 0x000b3b10: the one in use, if there is one, is forgotten: its address
and its key of the network, and it counts no more */
void online_unknown_000b36c0_unregister(
	void)
{
	byte *key;

	if (unknown_005107dc==NONE)
	{
		return;
	}
	key= unknown_00510588[unknown_005107dc];
	if (unknown_005107e0)
	{
		XNetUnregisterInAddr(unknown_005107e0);
		unknown_005107e0= 0;
	}
	XNetUnregisterKey(key+ONLINE_UNKNOWN_000B36C0_KEY_OFFSET);
	unknown_005107e4--;
	unknown_005107dc= NONE;
	unknown_005107e0= 0;
}

/* 0x000b3cf0: how long a text is, up to 0xFFF characters */
long online_unknown_000b36c0_string_length(
	const char *string)
{
	long length;

	for (length= 0; length<ONLINE_UNKNOWN_000B36C0_MAXIMUM_STRING_LENGTH; length++)
	{
		if (!string[length])
		{
			break;
		}
	}

	return length;
}

/* 0x00510580: whether something is going on. 0x00510584: its task, or
NONE. 0x005107d8: so many (three times as many updates end it).
0x0054761c: a byte that goes with it. 0x00547620: how many updates it has
been going on */
byte unknown_00510580;
long unknown_00510584;
long unknown_005107d8;
byte unknown_0054761c;
long unknown_00547620;

/* 0x000b4020: it ends: the one in use is forgotten
(online_unknown_000b36c0_unregister) and its task closed */
void online_unknown_000b36c0_unknown_000b4020(
	void)
{
	unknown_0054761c= FALSE;
	online_unknown_000b36c0_unregister();
	if (unknown_00510584!=NONE)
	{
		online_task_close(unknown_00510584);
		unknown_00510584= NONE;
	}
	unknown_00510580= FALSE;
}

/* 0x000b4050: one update more; after three times unknown_005107d8 of them
(or at once if nothing is going on) it ends as in 0x000b4020 */
void online_unknown_000b36c0_unknown_000b4050(
	void)
{
	long count= 0;

	unknown_00547620++;
	if (unknown_00510580)
	{
		count= unknown_005107d8;
	}
	if (unknown_00547620>=count*3)
	{
		online_unknown_000b36c0_unregister();
		if (unknown_00510584!=NONE)
		{
			online_task_close(unknown_00510584);
			unknown_00510584= NONE;
		}
		unknown_00510580= FALSE;
		unknown_0054761c= FALSE;
	}
}
