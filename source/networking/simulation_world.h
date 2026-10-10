/*
SIMULATION_WORLD.H

What unknown_004cf770 has going of a game in the network, and the views
that it has of it: one for each channel of the game that it talks by. The
name of the file is a guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_WORLD_H
#define __SIMULATION_WORLD_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of the thing: something that it has when it is going, and its views,
	of which each is there or not */
	SIMULATION_WORLD_UNKNOWN8_OFFSET= 0x8,
	SIMULATION_WORLD_VIEWS_OFFSET= 0x40,
	NUMBER_OF_SIMULATION_WORLD_VIEWS= 15,
	/* of a view: its kind, something that it has, and the channel of the
	game that it is of, by its number in the list */
	SIMULATION_VIEW_KIND_OFFSET= 0x2,
	SIMULATION_VIEW_UNKNOWNC_OFFSET= 0xC,
	SIMULATION_VIEW_CHANNEL_OFFSET= 0x3C
};

/* ---------- structures */

/* what goes through the views of the thing: a bit for each kind of view
that is wanted, and the place of the next one to look at */
struct simulation_view_iterator
{
	unsigned long kind_mask;
	long index;
};

/* ---------- prototypes/SIMULATION_WORLD.C */

boolean __attribute__((stdcall)) simulation_world_next_view(struct simulation_view_iterator *iterator,
	const byte *world, byte **view_out);
byte *simulation_world_get_channel_view(const byte *world, long channel_index);

#endif
