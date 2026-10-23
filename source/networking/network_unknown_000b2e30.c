/*
NETWORK_UNKNOWN_000B2E30.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Two things of 0x50 bytes that look for something in the network (they
have a task of the service of games and a measure of the network, and
entries of 0x784 bytes each that the caller gives). It looks like the
search for games, but what it is for is not known, and the name of the
file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include "networking/online_tasks.h"
#include "networking/transport_qos.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- constants */

enum
{
	NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE= 0x784,
	NETWORK_UNKNOWN_000B2E30_IDENTIFIER_SIZE= 8,

	/* in an entry: that it is in use, when it was filled, and a number of
	16 bits that has to be 0 */
	NETWORK_UNKNOWN_000B2E30_ENTRY_TIME_OFFSET= 0x4,
	NETWORK_UNKNOWN_000B2E30_ENTRY_UNKNOWN72_OFFSET= 0x72
};

/* ---------- structures */

/* 0x50 bytes */
struct network_unknown_000b2e30
{
	boolean active;
	boolean unknown1;
	long unknown4;
	byte identifier[NETWORK_UNKNOWN_000B2E30_IDENTIFIER_SIZE];
	long entry_count;
	byte *entries;
	boolean unknown18;
	byte unknown1C[0x20];
	long task_index;
	long qos_index;
	long unknown44;
	long unknown48;
	long unknown4C;
};

/* ---------- prototypes */

/* the time of the console, in milliseconds */
long GetTickCount(void);

/* ---------- globals */

/* 0x004d8b18, 0x004d8b19 (network_session.c) */
extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

/* 0x00510548, 0x0051054c (main.c): whether the network keeps a time of its
own, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004d8eb4: the two (0x004d8eb4 and 0x004d8f04) */
struct network_unknown_000b2e30 unknown_004d8eb4[2];

/* ---------- code */

/* 0x000b2e30: the first starts, if it has not and 0x004d8b18 and
0x004d8b19 say so: with an identifier of its own and the entries that are
given, cleared. Whether it is going on */
boolean network_unknown_000b2e30_start(
	byte *entries,
	long entry_count)
{
	struct network_unknown_000b2e30 *search= &unknown_004d8eb4[0];

	if (!search->active && unknown_004d8b18 && unknown_004d8b19)
	{
		search->active= TRUE;
		search->unknown1= FALSE;
		search->unknown4= 0;
		transport_security_random_identifier(search->identifier);
		search->entry_count= entry_count;
		search->entries= entries;
		memset(entries, 0, entry_count*NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE);
	}

	return search->active;
}

/* 0x000b31b0: the task and the measure of the first are closed, if it has
them, and three of its things cleared */
void network_unknown_000b2e30_stop(
	void)
{
	struct network_unknown_000b2e30 *search= &unknown_004d8eb4[0];

	if (search->task_index!=NONE)
	{
		online_task_close(search->task_index);
		search->task_index= NONE;
	}
	if (search->qos_index!=NONE)
	{
		transport_qos_delete(search->qos_index);
		search->qos_index= NONE;
	}
	search->unknown18= FALSE;
	search->unknown44= 0;
	search->unknown48= 0;
}

/* 0x000b3500: an entry is filled from what is found: cleared, in use, the
time, and 0x10 and then 0x24 bytes of it */
void network_unknown_000b2e30_fill_entry(
	const byte *found,
	byte *entry)
{
	memset(entry, 0, NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE);
	entry[0]= TRUE;
	*(long *)(entry+NETWORK_UNKNOWN_000B2E30_ENTRY_TIME_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
	*(long *)(entry+0x8)= *(const long *)(found+0x10);
	*(long *)(entry+0xC)= *(const long *)(found+0x14);
	memcpy(entry+0x10, found, 0x10);
	memcpy(entry+0x20, found+0x18, 0x24);
}

/* 0x000b35e0: an entry of the second, if it is in use and its number of
0x72 is 0; NULL if not, or if there is no such entry */
byte *network_unknown_000b2e30_get_entry(
	long index)
{
	const struct network_unknown_000b2e30 *search= &unknown_004d8eb4[1];
	byte *entry;

	if (index<0 || index>=search->entry_count)
	{
		return NULL;
	}
	entry= search->entries+index*NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE;
	if (!entry[0] || *(const short *)(entry+NETWORK_UNKNOWN_000B2E30_ENTRY_UNKNOWN72_OFFSET))
	{
		return NULL;
	}

	return entry;
}
