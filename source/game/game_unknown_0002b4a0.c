/*
GAME_UNKNOWN_0002B4A0.C

a part of the game: it has a table of data (unknown_00509434), and its first
function sets the time of the game in seconds.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "memory/data.h"

#include <stdint.h>

/* ---------- globals */

/* 0x00509434: its table of data */
struct data_array *unknown_00509434;

/* 0x004c1a5c, 0x004c1a74, 0x004c1a8c: three memories of the console, 0x18
bytes from one to the next */
void *unknown_004c1a5c;
void *unknown_004c1a74;
void *unknown_004c1a8c;

int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void game_unknown_0002b4a0_unknown_0003b950(void);

/* ---------- code */

/* 0x0002b5c0: the part is closed: game_unknown_0002b4a0_unknown_0003b950, and it has no table */
void game_unknown_0002b4a0_dispose(
	void)
{
	game_unknown_0002b4a0_unknown_0003b950();
	unknown_00509434= NULL;
}

/* 0x0002b5a0: when a map goes, the table cannot be used, if it is there and
could */
void game_unknown_0002b4a0_dispose_from_old_map(
	void)
{
	struct data_array *data= unknown_00509434;

	if (data && data->valid)
	{
		data->valid= FALSE;
	}
}

/* 0x0003b950: each of the three memories of the console that is there is
given back (VirtualFree; if that fails, the error is asked for), and there
is none */
void game_unknown_0002b4a0_unknown_0003b950(
	void)
{
	if (unknown_004c1a5c)
	{
		if (!VirtualFree(unknown_004c1a5c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a5c= NULL;
	}
	if (unknown_004c1a74)
	{
		if (!VirtualFree(unknown_004c1a74, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a74= NULL;
	}
	if (unknown_004c1a8c)
	{
		if (!VirtualFree(unknown_004c1a8c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a8c= NULL;
	}
}
