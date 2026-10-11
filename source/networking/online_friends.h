/*
ONLINE_FRIENDS.H

The friends of a player in the service of games by the network. Worked out
from halo2ship.exe; the name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __ONLINE_FRIENDS_H
#define __ONLINE_FRIENDS_H

#include "cseries/cseries.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* of a friend, as the libraries give it: who it is, its name, and the
	game that it plays */
	ONLINE_FRIEND_IDENTIFIER_SIZE= 12,
	ONLINE_FRIEND_NAME_OFFSET= 0xC,
	ONLINE_FRIEND_NAME_LENGTH= 15,
	ONLINE_FRIEND_TITLE_OFFSET= 0x30,
	/* who a friend is and its name, as the game keeps them */
	ONLINE_FRIEND_IDENTITY_SIZE= 0x56,
	/* something that has 32 texts of 255 characters, after four bytes */
	NUMBER_OF_ONLINE_UNKNOWN_0008CC00_TEXTS= 32,
	ONLINE_UNKNOWN_0008CC00_TEXTS_OFFSET= 4,
	ONLINE_UNKNOWN_0008CC00_TEXT_LENGTH= 255,
	/* more of a friend, as the libraries give it: what it is at, as bits,
	and three numbers; and how big all of it is */
	ONLINE_FRIEND_STATE_OFFSET= 0x1C,
	ONLINE_FRIEND_UNKNOWN28_OFFSET= 0x28,
	ONLINE_FRIEND_SIZE= 0x56,
	/* the bits of what a friend is at, as the libraries have them */
	ONLINE_FRIEND_STATE_UNKNOWN1= 0x1,
	ONLINE_FRIEND_STATE_UNKNOWN2= 0x2,
	ONLINE_FRIEND_STATE_UNKNOWN8= 0x8,
	ONLINE_FRIEND_STATE_UNKNOWN10= 0x10,
	ONLINE_FRIEND_STATE_UNKNOWN4000000= 0x4000000,
	ONLINE_FRIEND_STATE_UNKNOWN8000000= 0x8000000,
	ONLINE_FRIEND_STATE_UNKNOWN20000000= 0x20000000,
	ONLINE_FRIEND_STATE_UNKNOWN40000000= 0x40000000,
	ONLINE_FRIEND_STATE_UNKNOWN80000000= 0x80000000,
	/* and as the game has them */
	_online_friend_unknown1= 0x1,
	_online_friend_unknown2= 0x2,
	_online_friend_unknown4= 0x4,
	_online_friend_unknown8= 0x8,
	_online_friend_unknown10= 0x10,
	_online_friend_unknown20= 0x20,
	_online_friend_same_title= 0x40,
	_online_friend_unknown80= 0x80,
	_online_friend_unknown100= 0x100,
	_online_friend_unknown400= 0x400,
	/* the three answers that a friend can be given */
	NUMBER_OF_ONLINE_FRIEND_ANSWERS= 3
};

/* ---------- structures */

/* what the game says of a friend */
struct online_friend_summary
{
	unsigned long identifier[3];
	long state;
	long unknown10;
	long unknown14;
	unsigned long title;
	boolean unknown1C;
};

/* ---------- prototypes/ONLINE_FRIENDS.C */

boolean online_title_is_same_title(unsigned long title);
boolean online_friend_is_same_title(const byte *online_friend);
void online_unknown_0008cc00(byte *unknown);
void online_unknown_0008cc20(wchar *destination, const wchar *source);
void online_friend_get_identity(const byte *online_friend, byte *identity);
long __attribute__((stdcall)) online_friend_state_from_library(unsigned long library_state, unsigned long title);
unsigned long online_friend_state_to_library(unsigned long state);
long online_friend_get_state(const byte *online_friend);
void online_friend_get_summary(const byte *online_friend, struct online_friend_summary *summary);
void __attribute__((stdcall)) online_friend_remove(const byte *online_friend, long owner);
void __attribute__((stdcall)) online_friend_answer_request(long answer, const byte *online_friend, long owner);
void __attribute__((stdcall)) online_friend_answer_game_invite(const byte *online_friend, long owner, long answer);
void __attribute__((stdcall)) online_friend_get_title_name(wchar *name, const byte *online_friend, short size);

#endif
