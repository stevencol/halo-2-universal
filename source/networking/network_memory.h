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
	NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWNA0AC_OFFSET= 0xA0AC
};

/* ---------- prototypes/NETWORK_MEMORY.C */

void network_memory_free(void);

#endif
