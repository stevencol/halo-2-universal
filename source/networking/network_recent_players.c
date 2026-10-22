/*
NETWORK_RECENT_PLAYERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A table of 350 players (0x58 bytes of each, that say who it is) kept in two
orders, each a list from the first to the last, as a list of the players
that were played with would be. The name of the file is a guess, and so are
the names of its functions.
*/

#include "cseries/cseries.h"
#include "networking/network_observer.h"

#include <string.h>

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_RECENT_PLAYERS= 350,
	NETWORK_RECENT_PLAYER_SIZE= 0x68,
	NETWORK_RECENT_PLAYER_DATA_SIZE= 0x58,
	NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET= 0x56,
	/* the two lists: the one before and the one after, in each */
	NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET= 0x58,
	NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET= 0x5A,
	NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET= 0x5C,
	NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET= 0x5E
};

/* ---------- prototypes */

/* the time of the console, in milliseconds */
long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c (main.c): whether the network keeps a time of its
own, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004cf8e4, 0x004cf95c (online_tasks.c): the observer of the network,
for something that measures it */
extern byte *unknown_004cf8e4;

/* 0x004cf8e0: whether there is that observer. 0x004cf95d, 0x004cf960: that
it was started over, and when */
boolean unknown_004cf8e0;
boolean unknown_004cf95d;
long unknown_004cf960;

/* 0x004cf97c, 0x004cf980: the first and the last of the first list; NONE if
it has none. 0x004cf984, 0x004cf988: those of the second */
long unknown_004cf97c;
long unknown_004cf980;
long unknown_004cf984;
long unknown_004cf988;

/* 0x004cf98c: the players */
byte unknown_004cf98c[MAXIMUM_NETWORK_RECENT_PLAYERS*NETWORK_RECENT_PLAYER_SIZE];

/* ---------- private code */

/* the time in milliseconds: the one that main.c kept, if it says so. The
game has this in each function that uses it */
static inline __attribute__((always_inline)) long network_recent_players_time(
	void)
{
	return unknown_00510548 ? unknown_0051054c : GetTickCount();
}

static inline __attribute__((always_inline)) byte *network_recent_player_get(
	long index)
{
	return unknown_004cf98c+index*NETWORK_RECENT_PLAYER_SIZE;
}

/* ---------- code */

/* 0x0007f260: if there is the observer that measures the network, it starts
over (network_observer_reset_unknown4e08), and that is kept with the time */
void network_recent_players_unknown_0007f260(
	void)
{
	if (!unknown_004cf8e0)
	{
		return;
	}
	network_observer_reset_unknown4e08(unknown_004cf8e4);
	unknown_004cf95d= TRUE;
	unknown_004cf960= network_recent_players_time();
}

/* 0x0007f8b0: the 0x58 bytes of a player are made: the twelve bytes that it
is given first, 0x48 of what it is given second (from 0) and the byte of
0x7F of that, or, if that is 0xFF, the one of 0x56 of what it is given last,
if anything */
void __attribute__((stdcall)) network_recent_player_make(
	const byte *unknown,
	byte *data,
	const byte *identifier,
	const byte *other)
{
	byte unknown7F;

	memset(data, 0, NETWORK_RECENT_PLAYER_DATA_SIZE);
	memcpy(data, identifier, 0xC);
	memcpy(data+0xC, unknown, 0x40);
	*(long *)(data+0x4C)= *(const long *)(unknown+0x40);
	data[0x50]= unknown[0x44];
	data[0x51]= unknown[0x45];
	data[0x52]= unknown[0x46];
	data[0x53]= unknown[0x47];
	unknown7F= unknown[0x7F];
	if (unknown7F==0xFF && other)
	{
		data[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= other[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET];
	}
	else
	{
		data[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= unknown7F;
	}
}

/* 0x0007fd10: a player goes out of the first list */
void network_recent_player_remove_first(
	long index)
{
	byte *player= network_recent_player_get(index);
	short previous= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET);
	short next;

	if (previous!=NONE)
	{
		*(short *)(network_recent_player_get(previous)+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)=
			*(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET);
	}
	next= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET);
	if (next!=NONE)
	{
		*(short *)(network_recent_player_get(next)+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET)=
			*(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET);
	}
	if (unknown_004cf97c==index)
	{
		unknown_004cf97c= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET);
	}
	if (unknown_004cf980==index)
	{
		unknown_004cf980= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET);
	}
}

/* 0x0007fd80: a player goes out of the second list */
void network_recent_player_remove_second(
	long index)
{
	byte *player= network_recent_player_get(index);
	short previous= *(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET);
	short next;

	if (previous!=NONE)
	{
		*(short *)(network_recent_player_get(previous)+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET)=
			*(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET);
	}
	next= *(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET);
	if (next!=NONE)
	{
		*(short *)(network_recent_player_get(next)+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET)=
			*(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET);
	}
	if (unknown_004cf984==index)
	{
		unknown_004cf984= *(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET);
	}
	if (unknown_004cf988==index)
	{
		unknown_004cf988= *(const short *)(player+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET);
	}
}

/* 0x0007fdf0: a player goes to the start of the first list, if it is not
there. If the list had none, it is not made its last */
void network_recent_player_move_to_first(
	long index)
{
	byte *player;

	if (unknown_004cf97c==index)
	{
		return;
	}
	network_recent_player_remove_first(index);
	if (unknown_004cf97c!=NONE)
	{
		*(short *)(network_recent_player_get(unknown_004cf97c)+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET)= (short)index;
	}
	player= network_recent_player_get(index);
	*(short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET)= NONE;
	*(short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)= (short)unknown_004cf97c;
	unknown_004cf97c= index;
}

/* 0x0007fe40: the same with the second list */
void network_recent_player_move_to_second(
	long index)
{
	byte *player;

	if (unknown_004cf984==index)
	{
		return;
	}
	network_recent_player_remove_second(index);
	if (unknown_004cf984!=NONE)
	{
		*(short *)(network_recent_player_get(unknown_004cf984)+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET)= (short)index;
	}
	player= network_recent_player_get(index);
	*(short *)(player+NETWORK_RECENT_PLAYER_SECOND_PREVIOUS_OFFSET)= NONE;
	*(short *)(player+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET)= (short)unknown_004cf984;
	unknown_004cf984= index;
}
