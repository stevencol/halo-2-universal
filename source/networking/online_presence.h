/*
ONLINE_PRESENCE.H

What other players of the service of games by the network are at, for
players that need not be friends: the online tasks of the kind 0x21. The
libraries do not say the names of what they use, so the name of the file is
a guess, and so are the names of its functions. Worked out from
halo2ship.exe.
*/

#ifndef __ONLINE_PRESENCE_H
#define __ONLINE_PRESENCE_H

#include "cseries/cseries.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	_online_task_type_presence= 0x21,
	/* what the libraries give of a player: who it is, what it is at, as
	bits, and the game that it plays */
	ONLINE_PRESENCE_SIZE= 0x3D,
	ONLINE_PRESENCE_STATE_OFFSET= 0xC,
	ONLINE_PRESENCE_TITLE_OFFSET= 0x18,
	/* what the game remembers of other players: so many things, each with
	who a player is, who it is as the game has it, its name, when that was
	asked for, and what the thing is at */
	MAXIMUM_ONLINE_PRESENCE_NAMES= 150,
	ONLINE_PRESENCE_NAME_SIZE= 0x40,
	ONLINE_PRESENCE_NAME_IDENTIFIER_OFFSET= 0xC,
	ONLINE_PRESENCE_NAME_IDENTIFIER_SIZE= 12,
	ONLINE_PRESENCE_NAME_TEXT_OFFSET= 0x18,
	ONLINE_PRESENCE_NAME_TEXT_LENGTH= 15,
	ONLINE_PRESENCE_NAME_TIME_OFFSET= 0x38,
	ONLINE_PRESENCE_NAME_STATE_OFFSET= 0x3C,
	_online_presence_name_free= 0,
	_online_presence_name_asked,
	_online_presence_name_known= 3
};

/* ---------- prototypes/ONLINE_PRESENCE.C */

long __attribute__((stdcall)) online_presence_state_from_library(unsigned long library_state, unsigned long title);
long online_presence_start(long owner);
void __attribute__((stdcall)) online_presence_unknown_0008c3a0(long task_index, long unknown, long unknown2,
	long unknown3);
void online_presence_unknown_0008c410(long task_index);
void __attribute__((stdcall)) online_presence_get_latest(long task_index, long unknown, unsigned long count,
	byte *entries);
void online_presence_unknown_0008c550(long task_index);
boolean __attribute__((stdcall)) online_presence_names_find(byte *names, long unknown, const unsigned long *user,
	wchar *name, byte *identifier);

#endif
