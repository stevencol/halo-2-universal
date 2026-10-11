/*
ONLINE_FRIENDS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The friends of a player in the service of games. The names are guesses.
*/

#include "online_friends.h"

#include <string.h>

/* ---------- prototypes */

/* 0x003a075e, of the console's libraries: whether a game is this one, or
one that counts as it */
long __attribute__((stdcall)) XOnlineTitleIdIsSameTitle(unsigned long title);
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* ---------- code */

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
void online_unknown_0008cc00(
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
void online_unknown_0008cc20(
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
