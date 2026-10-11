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
	ONLINE_UNKNOWN_0008CC00_TEXT_LENGTH= 255
};

/* ---------- prototypes/ONLINE_FRIENDS.C */

boolean online_title_is_same_title(unsigned long title);
boolean online_friend_is_same_title(const byte *online_friend);
void online_unknown_0008cc00(byte *unknown);
void online_unknown_0008cc20(wchar *destination, const wchar *source);
void online_friend_get_identity(const byte *online_friend, byte *identity);

#endif
