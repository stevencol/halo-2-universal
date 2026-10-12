/*
GAME_UNKNOWN_0002B4A0.C

a part of the game: it has a table of data (unknown_00509434), and its first
function sets the time of the game in seconds.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- globals */

/* 0x00509434: its table of data */
struct data_array *unknown_00509434;

/* 0x0003b950: what else is closed with it */
void unknown_0003b950(void);

/* ---------- code */

/* 0x0002b5c0: the part is closed: unknown_0003b950, and it has no table */
void game_unknown_0002b4a0_dispose(
	void)
{
	unknown_0003b950();
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
