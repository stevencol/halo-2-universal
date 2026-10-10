/*
NETWORK_MEMORY.H

The memory that the network of the game takes when a map is loaded and lets
go of when it is closed: its channels, its views and the tables of them, and
its heap. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __NETWORK_MEMORY_H
#define __NETWORK_MEMORY_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of what unknown_004d87e8 is: where the two things in it that have
	functions have them */
	NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWN2098_OFFSET= 0x2098,
	NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWNA0AC_OFFSET= 0xA0AC,
	/* of what network_memory_unknown_00081310 makes: four groups of two
	numbers and a real, 0xD8 bytes from one to the next */
	NETWORK_MEMORY_UNKNOWN_00081310_GROUPS_OFFSET= 0x244,
	NETWORK_MEMORY_UNKNOWN_00081310_GROUP_SIZE= 0xD8,
	NETWORK_MEMORY_UNKNOWN_00081310_GROUP_COUNT= 4
};

/* ---------- prototypes/NETWORK_MEMORY.C */

void network_memory_free(void);
void network_memory_unknown_00081310(byte *unknown);

#endif
