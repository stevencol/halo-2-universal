/*
SIMULATION_VIEW.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A view that the game in the network has of itself: what it talks with
another console by, one for each channel of the game. The name of the file
is a guess, and so are the names of its functions.
*/

#include "simulation_view.h"

#include "networking/network_observer.h"
#include "cache/texture_cache.h"
#include "memory/ring_buffer.h"
#include "saved games/game_state.h"
#include "networking/network_channel.h"
#include "networking/replication_entity_view.h"
#include "networking/replication_event_view.h"
#include "networking/simulation.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- structures */

/* what sees to the things of a kind: something with functions, of which
the seventh says whether a thing has something that is still to be told of */
struct simulation_view_entity_client;

struct simulation_view_entity_client_functions
{
	void *unknown0[6];
	boolean (__attribute__((thiscall)) *pending)(struct simulation_view_entity_client *client, const byte *entity);
};

struct simulation_view_entity_client
{
	const struct simulation_view_entity_client_functions *functions;
};

/* ---------- prototypes */

long GetTickCount(void);


/* 0x001995a0: the state of the game, from where it is, is packed into
memory of a size; how many bytes it is then, and whether it could be.
0x000685f0: one of the things that come is put in bytes, in memory of a
size; how many, and whether it could be. 0x00068670: one of the things that come is made
from the bytes that were kept of it; whether it could be */
boolean __attribute__((stdcall)) unknown_001995a0(const void *base, void *buffer, long *size_out, long capacity,
	long level);

/* 0x0006a7f0: the thing takes what some players of the other console do, as
bits say which. 0x0006a990: it takes a piece of the state of the game, of a
size, that goes at a place; whether it could. 0x0006aa20: all of the state of
the game has come, so many bytes: it is the one of the game now; whether it
could */


/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x00510ca2: a view that is not of the second kind or of the fourth is
ready */
boolean unknown_00510ca2;

/* 0x004d87e4: whether the views have something at 0x8. 0x00450d14: the
functions of one more thing that that has */
boolean unknown_004d87e4;
void *unknown_00450d14[1];

/* ---------- code */

/* 0x00086140: a view is told whether it keeps the game from going on. When
it starts to, the time is noted. And one that has done so for two seconds,
and is well, goes to its first state (simulation_view_set_state) and is then
as 4 says. The game has the time of the network, and network_time_since, written
here */
void simulation_view_set_blocked(
	byte *view,
	boolean blocked)
{
	if (blocked && !view[SIMULATION_VIEW_BLOCKED_OFFSET])
	{
		*(long *)(view+SIMULATION_VIEW_BLOCKED_TIME_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
	}
	view[SIMULATION_VIEW_BLOCKED_OFFSET]= blocked;
	if (blocked && network_time_since(*(const long *)(view+SIMULATION_VIEW_BLOCKED_TIME_OFFSET))>=SIMULATION_VIEW_BLOCKED_TIMEOUT &&
		*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_blocked;
	}
}

/* 0x00085a70: a view goes to a state, with a number. It can if the state is
one of the first two and the number is NONE; if it is the third and the
number is not less than nothing; or if it is the one after the state that
the view is in, with the same number. If it cannot, a view that is well
goes to its first state, and is then as 7 says.

With the state and the number that it has already, nothing is done. If not,
they are the ones of the view; the other console is told, if the view talks
by a channel of the observer (a message of the kind 0x25, with the two); and
what follows from it is seen to (simulation_view_update_ready) */
void __attribute__((stdcall)) simulation_view_set_state(
	byte *view,
	long state,
	long number)
{
	struct simulation_view_state_message message= {0, 0};
	boolean good;

	if (state<_simulation_view_state2)
	{
		good= number==NONE;
	}
	else if (state==_simulation_view_state2)
	{
		good= number>=0;
	}
	else
	{
		good= number==*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET) &&
			state==(long)(*(const unsigned long *)(view+SIMULATION_VIEW_STATE_OFFSET)+1);
	}
	if (!good)
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
		{
			simulation_view_set_state(view, _simulation_view_state0, NONE);
			*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_bad_state;
		}

		return;
	}
	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==state &&
		*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)==number)
	{
		return;
	}
	*(long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)= number;
	message.number= number;
	*(long *)(view+SIMULATION_VIEW_STATE_OFFSET)= state;
	message.state= state;
	if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
	{
		network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
			*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
			_network_message_simulation_view_state, sizeof(message), &message);
	}
	simulation_view_update_ready(view);
}

/* 0x000862c0: a view that is well goes to its first state, and is then as a
number says. The game has this written in most of the places that do it */
void __attribute__((stdcall)) simulation_view_fail(
	byte *view,
	long reason)
{
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= reason;
	}
}

/* 0x00085f50: a view of the second kind is told what the other console
says of its players: that it could not go on, or two numbers, of which the
second is the one of what it is up to, and what some of its players do;
whether that was heeded.

If it could not go on, the view goes to its first state and is as 2 says
(simulation_view_fail, written here). If not: numbers before the last that
came are not heeded, nor one that is not before the one of the thing. The
rest are; and if a number of the thing is 4, the two numbers are kept and
the thing takes what the players do (simulation_world_set_player_actions) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085f50(
	byte *view,
	boolean failed,
	long update,
	long unknown0,
	long players,
	const void *actions)
{
	byte *world;

	if (failed)
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
		{
			simulation_view_set_state(view, _simulation_view_state0, NONE);
			*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_remote_failed;
		}

		return TRUE;
	}
	if (unknown0<*(const long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET) ||
		update<*(const long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET))
	{
		return FALSE;
	}
	world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	if (update>=*(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET))
	{
		return FALSE;
	}
	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET)= update;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET)= unknown0;
		simulation_world_set_player_actions(world, view+SIMULATION_VIEW_UNKNOWN14_OFFSET, players, actions);
	}

	return TRUE;
}

/* 0x00085ed0: a view of the first kind is given a piece of the state of
the game that the other console sends, which goes at a place and is of a
size; whether it was taken. It is not if the thing is not being sent the
state of the game. A piece of some size is one more piece (simulation_world_receive_game_state);
one of none says that it has all come, so many bytes (simulation_world_finish_game_state). If
the thing could not take it, the view goes to its first state and is as 5
says (simulation_view_fail, written here) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085ed0(
	byte *view,
	long offset,
	long size,
	const void *data)
{
	byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	boolean good;

	if (!simulation_world_receiving_game_state(world))
	{
		return FALSE;
	}
	if (size>0)
	{
		good= simulation_world_receive_game_state(world, offset, size, data);
	}
	else
	{
		good= simulation_world_finish_game_state(world, offset);
	}
	if (!good && *(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
	}

	return good;
}

/* 0x00085dc0: a view of the first kind is told the number that the game
goes on from; whether that was heeded. It is only if the number that the
thing has at 0x18 is 3 and the thing is not being sent the state of the
game. Then the thing makes ready to be sent the state of the game
(simulation_world_begin_game_state); if it could, the number
is the one of the thing, it starts to go as simulation_world_unknown_0006b2a0
has it (written here), and the game is brought up to what has come
(simulation_world_unknown_00069350). If it could not, the view goes to its
first state and is as 5 says (simulation_view_fail, written here) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085dc0(
	byte *view,
	long number)
{
	byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);

	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)!=_simulation_world_unknown18_3 ||
		simulation_world_receiving_game_state(world))
	{
		return FALSE;
	}
	if (simulation_world_begin_game_state(world))
	{
		world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
		*(long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET)= number;
		if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
		{
			simulation_world_clear_updates(world);
			*(long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)= (long)((unsigned long)number-1);
			*(long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)= number;
		}
		world[SIMULATION_WORLD_GOING_OFFSET]= TRUE;
		simulation_world_unknown_00069350(TRUE, *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET));

		return TRUE;
	}
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
	}

	return FALSE;
}

/* 0x00085d00: a view of the second kind or of the fourth is told the
players that the other console knows of: two bits for each, and the three
numbers that each is known by; whether that was heeded. It is not by a view
of no channel or one that is not ready. The players that the view knows of
are the ones with both bits that the thing has with those numbers
(simulation_world_player_matches). And if the view is past being made
ready, and that leaves out a player of the thing whose byte is set
(simulation_world_players_unknown25_mask), the view goes to its first state
and is as 8 says (simulation_view_fail, written here) */
boolean __attribute__((stdcall)) simulation_view_set_players(
	byte *view,
	unsigned long players,
	unsigned long valid,
	const long *identifiers)
{
	unsigned long known= 0;
	long player;

	if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)==NONE || !view[SIMULATION_VIEW_READY_OFFSET])
	{
		return FALSE;
	}
	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		unsigned long bit= 1u<<player;

		if ((players&bit) && (valid&bit) && simulation_world_player_matches(player,
			*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET), identifiers+player*(SIMULATION_PLAYER_IDENTIFIER_SIZE/4)))
		{
			known|= bit;
		}
	}
	if (simulation_view_established(view) &&
		(~known&simulation_world_players_unknown25_mask(*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET))) &&
		*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_players_missing;
	}
	*(unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET)= known;

	return TRUE;
}

/* 0x000860b0: a view is given one of the things that come and wait for the
thing. In its sixth state, it is sent on to the other console as it is (a
message of the kind 0x27, of 0x4048 bytes), if the view talks by a channel
of the observer. In any other, and with something at 0x94, the view takes it
(simulation_view_queue_update) and something follows (simulation_view_send_game_state); if it could
not, the view goes to its first state and is as 5 says (simulation_view_fail,
written here). The game sends a copy */
void simulation_view_send_update(
	byte *view,
	const void *update)
{
	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==_simulation_view_state5)
	{
		byte copy[SIMULATION_VIEW_UPDATE_SIZE];

		memcpy(copy, update, sizeof(copy));
		if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
		{
			network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
				*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
				_network_message_simulation_update, sizeof(copy), copy);
		}
	}
	else if (*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET))
	{
		if (simulation_view_queue_update(update, view))
		{
			simulation_view_send_game_state(view);
		}
		else if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
		{
			simulation_view_set_state(view, _simulation_view_state0, NONE);
			*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
		}
	}
}

/* 0x00085b20: the other end of a view says its state, and the number that
goes with it: what the message of the kind 0x25 has
(simulation_view_set_state sends it). FALSE, with nothing done, for a view
of no thing or of no channel. They are kept; then, if the view is well:

A view of the second kind or of the fourth is the one that leads. In its
first two states it says its own state again, unless the other end is in
them too with no number. In its third: nothing if the other end is in the
third with the same number; if it was there with that number before, it has
gone back, and the view is as 7 says; if it is in the second, the view says
its state again; and if it is in the first, the view goes back to its
second. Past the third: the other end has to follow one state at a time,
with the same number, no further than the view; if not the view is as 7
says, or as 6 if the other end went back to its first state.

Any other view follows: it goes to the state that the other end says, if
that is the third and the view has no number or the same one, or a later
one with the same number. If not: a view from its third state on is as 6
says; one before that says its own state again if the other end was in its
first state and is not any more.

At the end, what follows from the two states is seen to
(simulation_view_update_ready) */
boolean simulation_view_handle_remote_state(
	byte *view,
	long state,
	long number)
{
	long old_state;
	long old_number;
	long own_state;
	boolean send= FALSE;

	if (!*(void *const *)(view+SIMULATION_VIEW_WORLD_OFFSET) || *(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)==NONE)
	{
		return FALSE;
	}
	old_state= *(const long *)(view+SIMULATION_VIEW_REMOTE_STATE_OFFSET);
	old_number= *(const long *)(view+SIMULATION_VIEW_REMOTE_STATE_NUMBER_OFFSET);
	*(long *)(view+SIMULATION_VIEW_REMOTE_STATE_OFFSET)= state;
	*(long *)(view+SIMULATION_VIEW_REMOTE_STATE_NUMBER_OFFSET)= number;
	own_state= *(const long *)(view+SIMULATION_VIEW_STATE_OFFSET);
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		long own_number= *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET);

		if ((1u<<(view[SIMULATION_VIEW_KIND_OFFSET]&31))&SIMULATION_VIEW_KINDS_2_AND_4)
		{
			if (own_state<_simulation_view_state2)
			{
				send= state>=_simulation_view_state2 || number!=NONE;
			}
			else if (own_state==_simulation_view_state2)
			{
				if (state==_simulation_view_state2 && number==own_number)
				{
				}
				else if (old_state==_simulation_view_state2 && old_number==own_number)
				{
					simulation_view_fail(view, _simulation_view_unknown28_bad_state);
				}
				else if (state>=_simulation_view_state1)
				{
					send= TRUE;
				}
				else
				{
					simulation_view_set_state(view, _simulation_view_state1, NONE);
				}
			}
			else if (state==_simulation_view_state0)
			{
				simulation_view_fail(view, _simulation_view_unknown28_remote_reset);
			}
			else if (!(number==own_number && state>_simulation_view_state2 && state<=own_state &&
				state==(long)((unsigned long)old_state+1)))
			{
				simulation_view_fail(view, _simulation_view_unknown28_bad_state);
			}
		}
		else if ((state==_simulation_view_state2 && (own_number==NONE || own_number==number)) ||
			(state>_simulation_view_state2 && own_number==number))
		{
			simulation_view_set_state(view, state, number);
		}
		else if (own_state>=_simulation_view_state2)
		{
			simulation_view_fail(view, _simulation_view_unknown28_remote_reset);
		}
		else
		{
			send= old_state==_simulation_view_state0 && state>_simulation_view_state0;
		}
	}
	if (send)
	{
		struct simulation_view_state_message message;

		message.state= own_state;
		message.number= *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET);
		if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
		{
			network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
				*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
				_network_message_simulation_view_state, sizeof(message), &message);
		}
	}
	simulation_view_update_ready(view);

	return TRUE;
}

/* 0x00086590: a view gives back the memory that the texture cache lent it,
if it has some, and has none, with nothing for the numbers that go with it */
void simulation_view_unknown_00086590(
	byte *view)
{
	if (*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET))
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN9C_OFFSET)= 0;
		*(long *)(view+SIMULATION_VIEW_UNKNOWNA0_OFFSET)= 0;
		texture_cache_return_memory(*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET));
		*(void **)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET)= NULL;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN98_OFFSET)= 0;
	}
}

/* 0x000861c0: what follows from the state of a view and the one of its
other end. A view of a thing and of a channel, whose number is the one that
the other end has, is ready if both are in the third state or past it; and
it has its byte at 0x78 set if both are in the sixth or past it.

When whether it is ready is another: that is noted. One that is not ready
any more knows of no players, if it is of the second kind or of the fourth
(and one of the second has NONE for what the players of the other console
last said, and gives back its memory); one of the first kind has nothing at
0xB0. A view of the third kind or of the fourth tells what it has at 0x8
(simulation_view_events_set_ready), and the thing is told (simulation_world_unknown_0006b090).

When the byte at 0x78 is another: that is noted, and the thing is told
(simulation_world_unknown_0006b110) */
void simulation_view_update_ready(
	byte *view)
{
	boolean ready= FALSE;
	boolean established= FALSE;
	byte was_ready;
	word kind;

	if (*(void *const *)(view+SIMULATION_VIEW_WORLD_OFFSET) && *(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE &&
		*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)==*(const long *)(view+SIMULATION_VIEW_REMOTE_STATE_NUMBER_OFFSET))
	{
		long own_state= *(const long *)(view+SIMULATION_VIEW_STATE_OFFSET);
		long remote_state= *(const long *)(view+SIMULATION_VIEW_REMOTE_STATE_OFFSET);

		ready= own_state>=_simulation_view_state2 && remote_state>=_simulation_view_state2;
		established= own_state>=_simulation_view_state5 && remote_state>=_simulation_view_state5;
	}
	was_ready= *(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE ? view[SIMULATION_VIEW_READY_OFFSET] : FALSE;
	if (ready!=was_ready)
	{
		view[SIMULATION_VIEW_READY_OFFSET]= ready;
		if (!ready)
		{
			kind= *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET);
			if ((1u<<(kind&31))&SIMULATION_VIEW_KINDS_2_AND_4)
			{
				*(unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET)= 0;
				if (kind==_simulation_view_kind_2)
				{
					*(long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET)= NONE;
					*(long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET)= NONE;
					if (*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET))
					{
						simulation_view_unknown_00086590(view);
					}
				}
			}
			else if (kind==_simulation_view_kind_1)
			{
				*(long *)(view+SIMULATION_VIEW_UNKNOWNB0_OFFSET)= 0;
			}
		}
		kind= *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET);
		if (kind==_simulation_view_kind_3 || kind==_simulation_view_kind_4)
		{
			simulation_view_events_set_ready(ready, *(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_VIEW_UNKNOWN8_UNKNOWN6060_OFFSET);
		}
		simulation_world_unknown_0006b090(ready, *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET), view);
	}
	if (view[SIMULATION_VIEW_UNKNOWN78_OFFSET]!=established)
	{
		view[SIMULATION_VIEW_UNKNOWN78_OFFSET]= established;
		simulation_world_unknown_0006b110(*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET), established, view);
	}
}

/* 0x00085cb0: whether a view has nothing more to send before the game goes
on: one of the third kind or of the fourth, as simulation_view_entities_pending says; any
other, if it has no memory of the state of the game or nothing of it left */
boolean simulation_view_nothing_to_send(
	byte *view)
{
	word kind= *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET);

	if (kind==_simulation_view_kind_3 || kind==_simulation_view_kind_4)
	{
		return !simulation_view_entities_pending(view);
	}

	return !(*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET) && *(const long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET)>0);
}

/* 0x00085fc0: a view whose byte at 0x78 is set says to the other console
what the players of this one do, as four bits say which: a message of the
kind 0x28, with a count of them that goes on by one, the number of the last
thing that the thing took, and whether something came that it could not take
(in its third state or its fifth). It is what simulation_view_unknown_00085f50
takes at the other end */
void __attribute__((stdcall)) simulation_view_send_player_actions(
	byte *view,
	unsigned long players,
	const void *actions)
{
	struct simulation_view_player_actions_message message;
	const byte *world;
	long state;
	long index;

	if (!view[SIMULATION_VIEW_UNKNOWN78_OFFSET])
	{
		return;
	}
	memset(&message, 0, sizeof(message));
	message.count= *(const long *)(view+SIMULATION_VIEW_UNKNOWNB0_OFFSET);
	*(long *)(view+SIMULATION_VIEW_UNKNOWNB0_OFFSET)= (long)((unsigned long)message.count+1);
	world= *(const byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	message.update= (long)(*(const unsigned long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET)-1);
	state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
	message.failed= state==3 || state==5 ? world[SIMULATION_WORLD_UNKNOWN2C_OFFSET] : FALSE;
	message.players= players;
	for (index= 0; index<4; index++)
	{
		if (players&(1u<<index))
		{
			memcpy(message.actions[index], (const byte *)actions+index*sizeof(message.actions[index]), sizeof(message.actions[index]));
		}
	}
	if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
	{
		network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
			*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
			_network_message_simulation_player_actions, sizeof(message), &message);
	}
}

/* 0x00086ad0: whether a view of the third kind or of the fourth has things
of the game left to tell of: one of the things that its view of them goes
through (replication_entity_view_next_entity) has something still to be told
of, as what sees to its kind says. FALSE if a byte of what the view has at
0x8 is clear */
boolean simulation_view_entities_pending(
	byte *view)
{
	const byte *database;
	long entity_index= NONE;

	if (!(*(const byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET))[SIMULATION_VIEW_UNKNOWN8_UNKNOWN39_OFFSET])
	{
		return FALSE;
	}
	database= *(const byte *const *)(*(const byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET)+SIMULATION_WORLD_UNKNOWN4_OFFSET)+
		SIMULATION_WORLD_UNKNOWN4_DATABASE_OFFSET;
	for (;;)
	{
		struct simulation_view_entity_client *client;
		const byte *entity;

		entity_index= replication_entity_view_next_entity(entity_index,
			*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_VIEW_UNKNOWN8_ENTITY_VIEW_OFFSET);
		if (entity_index==NONE)
		{
			return FALSE;
		}
		entity= database+SIMULATION_DATABASE_ENTITIES_OFFSET+
			((unsigned long)entity_index&SIMULATION_DATABASE_ENTITY_INDEX_MASK)*SIMULATION_DATABASE_ENTITY_SIZE;
		client= (*(struct simulation_view_entity_client *const *const *)(database+SIMULATION_DATABASE_CLIENTS_OFFSET))[
			*(const short *)(entity+SIMULATION_DATABASE_ENTITY_TYPE_OFFSET)+1];
		if (client->functions->pending(client, entity))
		{
			return TRUE;
		}
	}
}

/* 0x00086b40: what a view of the third kind or of the fourth has at 0x6060
of what it has at 0x8 is told whether the view is ready. When it is: what it
keeps is emptied, with nothing for its number; if its view is of the second
kind or of the fourth, a number in it is NONE and its first byte is set; if
not, its second byte is set, and one of the game with it. When it is not:
the bytes that were set are cleared, and the one of the game with the
second */
void simulation_view_events_set_ready(
	boolean ready,
	byte *events)
{
	if (ready)
	{
		const byte *view= *(const byte *const *)(events+SIMULATION_VIEW_EVENTS_VIEW_OFFSET);
		unsigned long kind_bit= 1u<<(view[SIMULATION_VIEW_KIND_OFFSET]&31);

		memset(events+SIMULATION_VIEW_EVENTS_DATA_OFFSET, 0, SIMULATION_VIEW_EVENTS_DATA_SIZE);
		if (kind_bit&SIMULATION_VIEW_KINDS_2_AND_4)
		{
			*(long *)(events+SIMULATION_VIEW_EVENTS_UNKNOWN4CCC_OFFSET)= NONE;
			*(long *)(events+SIMULATION_VIEW_EVENTS_UNKNOWN8_OFFSET)= 0;
			events[SIMULATION_VIEW_EVENTS_UNKNOWN5_OFFSET]= TRUE;
		}
		else
		{
			*(long *)(events+SIMULATION_VIEW_EVENTS_UNKNOWN8_OFFSET)= 0;
			unknown_00510ca2= TRUE;
			events[SIMULATION_VIEW_EVENTS_UNKNOWN6_OFFSET]= TRUE;
		}
	}
	else
	{
		if (events[SIMULATION_VIEW_EVENTS_UNKNOWN5_OFFSET])
		{
			events[SIMULATION_VIEW_EVENTS_UNKNOWN5_OFFSET]= FALSE;
		}
		if (events[SIMULATION_VIEW_EVENTS_UNKNOWN6_OFFSET])
		{
			unknown_00510ca2= FALSE;
			events[SIMULATION_VIEW_EVENTS_UNKNOWN6_OFFSET]= FALSE;
		}
	}
}

/* 0x00086aa0: the texture cache takes back memory that it lent: if it is
the one of a view of the second kind, the view gives it back
(simulation_view_unknown_00086590). It is what the cache is given with the
memory, in unknown_000862e0 */
void __attribute__((stdcall)) simulation_view_reclaim_game_state(
	void *address,
	byte *view)
{
	if (*(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind_2 &&
		*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET) &&
		*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET)==address)
	{
		simulation_view_unknown_00086590(view);
	}
}

/* 0x00086800: the state of the game goes into the ring of a view, to be
sent to a console that comes in; whether it could. It is packed into memory
that is given, of a size (unknown_001995a0); what that makes goes into the
ring in pieces of no more than 0x400 bytes, each after eight bytes that say
that it is a piece, its size and where it goes; and at the end eight more,
of a piece of no size, with the size of it all. Each thing that goes into
the ring is one more that the view has to send. FALSE if the state could not
be packed, or the ring has no room. The game has ring_buffer_append written
here */
boolean __attribute__((stdcall)) simulation_view_queue_game_state(
	long capacity,
	byte *view,
	long size,
	const void *buffer)
{
	struct ring_buffer *ring= (struct ring_buffer *)(view+SIMULATION_VIEW_RING_OFFSET);
	struct simulation_view_game_state_record record;
	long packed_size;
	long position= 0;
	long remaining;
	boolean result;

	result= unknown_001995a0(game_state_globals.base_address, (void *)buffer, &packed_size, capacity,
		SIMULATION_VIEW_GAME_STATE_COMPRESSION_LEVEL);
	if (!result)
	{
		return FALSE;
	}
	for (remaining= packed_size; remaining>0; remaining-= record.size, position+= record.size)
	{
		record.type= _simulation_view_game_state_record_piece;
		record.size= remaining>MAXIMUM_SIMULATION_VIEW_GAME_STATE_PIECE_SIZE ?
			MAXIMUM_SIMULATION_VIEW_GAME_STATE_PIECE_SIZE : (short)remaining;
		record.number= position;
		if (ring_buffer_append(ring, sizeof(record), &record)==NONE ||
			ring_buffer_append(ring, record.size, (const byte *)buffer+position)==NONE)
		{
			return FALSE;
		}
		(*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET))++;
	}
	record.type= _simulation_view_game_state_record_piece;
	record.size= 0;
	record.number= position;
	if (ring_buffer_append(ring, sizeof(record), &record)==NONE)
	{
		return FALSE;
	}
	(*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET))++;

	return result;
}

/* 0x000862e0: a view of the second kind starts to send the state of the
game to its console, one more time; whether all of it is ready to go. The
texture cache lends memory for the ring of the view, to be given back with
simulation_view_reclaim_game_state, and more to pack the state into, each
with a short wait; with either missing, nothing is done.

The ring starts with eight bytes that say the number that the game goes on
from; then, between what is done before a state is saved and after
(simulation_world_before_send_game_state, and the first thing of after), the
packed state goes in, in pieces (simulation_view_queue_game_state). If that
could not be, the view gives its memory back. The memory that the state was
packed into goes back to the cache at the end. The game has
texture_cache_steal_memory written here twice, and ring_buffer_append */
boolean simulation_view_begin_send_game_state(
	byte *view)
{
	boolean result= FALSE;
	void *memory;
	void *buffer;

	(*(long *)(view+SIMULATION_VIEW_GAME_STATE_SEND_COUNT_OFFSET))++;
	memory= texture_cache_steal_memory(_texture_cache_steal_wait_short, SIMULATION_VIEW_RING_SIZE, (long)view, NULL,
		(texture_cache_stolen_memory_proc)simulation_view_reclaim_game_state);
	buffer= texture_cache_steal_memory(_texture_cache_steal_wait_short, SIMULATION_VIEW_PACKED_GAME_STATE_SIZE, 0, NULL, NULL);
	if (memory)
	{
		if (!buffer)
		{
			texture_cache_return_memory(memory);

			return result;
		}
		else
		{
			struct ring_buffer *ring= (struct ring_buffer *)(view+SIMULATION_VIEW_RING_OFFSET);
			byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
			struct simulation_view_game_state_record record;

			*(void **)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET)= memory;
			*(long *)(view+SIMULATION_VIEW_UNKNOWN98_OFFSET)= SIMULATION_VIEW_RING_SIZE;
			ring->size= SIMULATION_VIEW_RING_SIZE;
			ring->data= memory;
			ring->start= 0;
			ring->count= 0;
			*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET)= 0;
			record.type= _simulation_view_game_state_record_start;
			record.size= 0;
			record.number= *(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET);
			if (ring_buffer_append(ring, sizeof(record), &record)==NONE)
			{
				simulation_view_unknown_00086590(view);
			}
			else
			{
				long size;

				result= TRUE;
				(*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET))++;
				simulation_world_before_send_game_state(*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET), &size);
				if (!simulation_view_queue_game_state(SIMULATION_VIEW_PACKED_GAME_STATE_SIZE, view, size, buffer))
				{
					result= FALSE;
				}
				(*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET))[SIMULATION_WORLD_SENDING_GAME_STATE_OFFSET]= FALSE;
				game_state_after_save_procs[0](0);
				if (!result)
				{
					simulation_view_unknown_00086590(view);
				}
			}
		}
	}
	if (buffer)
	{
		texture_cache_return_memory(buffer);
	}

	return result;
}

/* 0x000865d0: what the ring of a view keeps for a console that comes in
goes out, a thing at a time, while the view is of a channel, is ready, has
things left, and the channel of the game is in its sixth state, has its
second thing, and that has room for 0x600 bytes more of messages.

Each thing is taken from the ring: its eight bytes, and what follows them.
The number that the game goes on from goes in a message of the kind 0x29; a
piece of the state of the game, in one of the kind 0x2A, with where it goes
and its size; and anything else is one of the things that came while the
state was sent: it is made again from its bytes (simulation_update_decode) and goes
as they do, in a message of the kind 0x27. If it could not be made, the view
goes to its first state and is as 5 says (simulation_view_fail, written
here), and no more goes out */
void __attribute__((stdcall)) simulation_view_send_game_state(
	byte *view)
{
	struct ring_buffer *ring= (struct ring_buffer *)(view+SIMULATION_VIEW_RING_OFFSET);

	for (;;)
	{
		struct simulation_view_game_state_record record;
		byte data[MAXIMUM_SIMULATION_VIEW_GAME_STATE_PIECE_SIZE];
		const byte *channel;
		const byte *queue;

		if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)==NONE || !view[SIMULATION_VIEW_READY_OFFSET] ||
			*(const long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET)==0)
		{
			return;
		}
		channel= unknown_004d87d4+*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
		if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)!=_network_channel_state5 ||
			!(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10))
		{
			return;
		}
		queue= unknown_004d87dc+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)*NETWORK_CHANNEL_UNKNOWN14_SIZE;
		if ((long)((SIMULATION_VIEW_SEND_QUEUE_MESSAGES-(*(const unsigned long *)(queue+NETWORK_CHANNEL_UNKNOWN14_UNKNOWN18_OFFSET)-
			*(const unsigned long *)(queue+NETWORK_CHANNEL_UNKNOWN14_UNKNOWN1C_OFFSET)))<<5)<SIMULATION_VIEW_SEND_QUEUE_ROOM)
		{
			return;
		}
		ring_buffer_read(ring, ring->start, sizeof(record), &record);
		ring->start= (long)((unsigned long)ring->start+sizeof(record))%ring->size;
		ring->count-= (long)sizeof(record);
		if (record.size>0)
		{
			ring_buffer_read(ring, ring->start, record.size, data);
			ring->start= (long)((unsigned long)ring->start+(unsigned long)(long)record.size)%ring->size;
			ring->count-= record.size;
		}
		switch (record.type)
		{
		case _simulation_view_game_state_record_start:
		{
			long number= record.number;

			if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
			{
				network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
					*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
					_network_message_simulation_game_state_start, sizeof(number), &number);
			}
			break;
		}
		case _simulation_view_game_state_record_piece:
		{
			struct simulation_view_game_state_piece_message message;

			message.offset= record.number;
			message.size= record.size;
			if (record.size>0)
			{
				memcpy(message.data, data, record.size);
			}
			if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
			{
				network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
					*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
					_network_message_simulation_game_state_piece, message.size+8, &message);
			}
			break;
		}
		default:
		{
			byte update[SIMULATION_VIEW_UPDATE_SIZE];

			memset(update, 0, sizeof(update));
			if (!simulation_update_decode(data, record.size, update))
			{
				if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
				{
					simulation_view_set_state(view, _simulation_view_state0, NONE);
					*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
				}

				return;
			}
			if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
			{
				network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
					*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
					_network_message_simulation_update, sizeof(update), update);
			}
			break;
		}
		}
		(*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET))--;
	}
}

/* 0x000869a0: one of the things that come while a view is sent the state of
the game is kept in its ring, to go after it; whether it could be. It is put
in bytes (simulation_update_encode), and those go into the ring after eight bytes
that say that it is one of those things, how many bytes it is, and its
number; and the view has one more thing to send. The game has
ring_buffer_append written here the first time */
boolean __attribute__((stdcall)) simulation_view_queue_update(
	const void *update,
	byte *view)
{
	struct ring_buffer *ring= (struct ring_buffer *)(view+SIMULATION_VIEW_RING_OFFSET);
	byte data[MAXIMUM_SIMULATION_VIEW_PACKED_UPDATE_SIZE];
	struct simulation_view_game_state_record record;
	long size;

	if (!simulation_update_encode(data, sizeof(data), update, &size))
	{
		return FALSE;
	}
	record.type= _simulation_view_game_state_record_update;
	record.size= (short)size;
	record.number= *(const long *)update;
	if (ring_buffer_append(ring, sizeof(record), &record)==NONE || ring_buffer_append(ring, size, data)==NONE)
	{
		return FALSE;
	}
	(*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET))++;

	return TRUE;
}

/* 0x00085880: a view lets go of its channel. One that is not in its first
state goes to it. The channel of the game has nothing of the view. A view of
the third kind or of the fourth takes itself out of what keeps the things
that the consoles share and the things that happen: its view of each starts
over (replication_entity_view_reset, replication_event_view_reset) and is
not one of theirs any more, and what it has at 0x8 has its numbers and bytes
as at the start. The channel of the observer has not the bit of a view; and
the view has no observer, no channel of it, and no channel of the game. The
game does not look whether there is a channel of the observer */
void simulation_view_release_channel(
	byte *view)
{
	byte *channel;
	word kind;

	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)!=_simulation_view_state0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
	}
	*(long *)(unknown_004d87d4+*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE+
		SIMULATION_VIEW_CHANNEL_UNKNOWN3C_OFFSET)= 0;
	kind= *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET);
	view[SIMULATION_VIEW_UNKNOWN44_OFFSET]= FALSE;
	if (kind==_simulation_view_kind_3 || kind==_simulation_view_kind_4)
	{
		byte *unknown8= *(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET);
		byte *unknown4= *(byte *const *)(*(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET)+SIMULATION_WORLD_UNKNOWN4_OFFSET);
		byte *event_view;
		long index;

		*(long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN28_OFFSET)-= *(const long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN20_OFFSET);
		*(long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN14_OFFSET)= 0;
		unknown8= *(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET);
		*(long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN28_OFFSET)-= *(const long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN1C_OFFSET);
		*(long *)(unknown8+SIMULATION_VIEW_UNKNOWN8_UNKNOWN10_OFFSET)= 0;
		*(long *)(*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_VIEW_UNKNOWN8_UNKNOWN2C_OFFSET)= 0;
		index= *(const long *)(view+SIMULATION_VIEW_INDEX_OFFSET);
		replication_entity_view_reset(*(byte *const *)(unknown4+SIMULATION_WORLD_UNKNOWN4_VIEWS_OFFSET+index*4));
		*(byte **)(unknown4+SIMULATION_WORLD_UNKNOWN4_VIEWS_OFFSET+index*4)= NULL;
		*(unsigned long *)(unknown4+SIMULATION_WORLD_UNKNOWN4_VIEW_MASK_OFFSET)&= ~(1u<<(index&31));
		index= *(const long *)(view+SIMULATION_VIEW_INDEX_OFFSET);
		replication_event_view_reset(*(byte *const *)(unknown4+SIMULATION_WORLD_UNKNOWN4_EVENT_VIEWS_OFFSET+index*4));
		*(unsigned long *)(unknown4+SIMULATION_WORLD_UNKNOWN4_EVENT_VIEW_MASK_OFFSET)&= ~(1u<<(index&31));
		*(byte **)(unknown4+SIMULATION_WORLD_UNKNOWN4_EVENT_VIEWS_OFFSET+index*4)= NULL;
		unknown8= *(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_VIEW_UNKNOWN8_UNKNOWN5098_OFFSET;
		unknown8[8]= FALSE;
		*(long *)(unknown8+0xC)= 0;
		event_view= *(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_VIEW_UNKNOWN8_EVENT_VIEW_OFFSET;
		replication_event_view_reset(event_view);
		event_view[8]= FALSE;
		(*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET))[SIMULATION_VIEW_UNKNOWN8_UNKNOWN38_OFFSET]= FALSE;
		(*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET))[SIMULATION_VIEW_UNKNOWN8_UNKNOWN8_OFFSET]= FALSE;
	}
	channel= *(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET)+
		*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)*SIMULATION_VIEW_OBSERVER_CHANNEL_SIZE+
		SIMULATION_VIEW_OBSERVER_CHANNELS_OFFSET;
	channel[SIMULATION_VIEW_OBSERVER_CHANNEL_FLAGS_OFFSET]&= (byte)~_simulation_view_observer_channel_view_bit;
	*(byte **)(view+SIMULATION_VIEW_OBSERVER_OFFSET)= NULL;
	*(long *)(view+SIMULATION_VIEW_UNKNOWN40_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)= NONE;
}

/* 0x00085600: a view is not of its thing any more: one of the second kind
gives back its memory (simulation_view_unknown_00086590, written here); the
thing has no view at its place, and one fewer; and the view has no thing,
and no place */
void simulation_view_detach(
	byte *view)
{
	byte *world;

	if (*(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind_2 &&
		*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET))
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN9C_OFFSET)= 0;
		*(long *)(view+SIMULATION_VIEW_UNKNOWNA0_OFFSET)= 0;
		texture_cache_return_memory(*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET));
		*(void **)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET)= NULL;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN98_OFFSET)= 0;
	}
	world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	*(byte **)(world+SIMULATION_WORLD_VIEWS_OFFSET+*(const long *)(view+SIMULATION_VIEW_INDEX_OFFSET)*4)= NULL;
	(*(long *)(world+SIMULATION_WORLD_VIEW_COUNT_OFFSET))--;
	*(byte **)(view+SIMULATION_VIEW_WORLD_OFFSET)= NULL;
	*(long *)(view+SIMULATION_VIEW_INDEX_OFFSET)= NONE;
}

/* 0x00085540: a view is new: its place in the table of the views, its kind,
what it has at 0x8, the six bytes that it is known by and a number; it is of
no thing, of no channel, in its first state with no number, as its other end
is, and well. One of the second kind or the fourth knows of no players; one
of the second has NONE for what the players of the other console last said,
no memory, nothing to send, and has not been sent the state of the game; one
of the first has nothing at 0xB0. The view is what is said */
byte *__attribute__((stdcall)) simulation_view_new(
	byte *view,
	const word *unknown14,
	long datum_index,
	long kind,
	byte *data,
	long unknown1c)
{
	*(long *)(view+SIMULATION_VIEW_DATUM_INDEX_OFFSET)= datum_index;
	*(byte **)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)= data;
	*(word *)(view+SIMULATION_VIEW_KIND_OFFSET)= (word)kind;
	memcpy(view+SIMULATION_VIEW_UNKNOWN14_OFFSET, unknown14, SIMULATION_VIEW_UNKNOWN14_SIZE);
	*(long *)(view+SIMULATION_VIEW_UNKNOWN1C_OFFSET)= unknown1c;
	*(byte **)(view+SIMULATION_VIEW_WORLD_OFFSET)= NULL;
	*(long *)(view+SIMULATION_VIEW_INDEX_OFFSET)= NONE;
	*(byte **)(view+SIMULATION_VIEW_OBSERVER_OFFSET)= NULL;
	*(long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= 0;
	*(long *)(view+SIMULATION_VIEW_STATE_OFFSET)= _simulation_view_state0;
	*(long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_REMOTE_STATE_OFFSET)= _simulation_view_state0;
	*(long *)(view+SIMULATION_VIEW_REMOTE_STATE_NUMBER_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)= NONE;
	*(long *)(view+SIMULATION_VIEW_UNKNOWN40_OFFSET)= NONE;
	view[SIMULATION_VIEW_UNKNOWN78_OFFSET]= FALSE;
	if ((1u<<(kind&31))&SIMULATION_VIEW_KINDS_2_AND_4)
	{
		*(unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET)= 0;
	}
	if ((word)kind==_simulation_view_kind_2)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET)= NONE;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET)= NONE;
		*(long *)(view+SIMULATION_VIEW_GAME_STATE_SEND_COUNT_OFFSET)= 0;
		*(void **)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET)= NULL;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN98_OFFSET)= 0;
		*(long *)(view+SIMULATION_VIEW_UNKNOWNAC_OFFSET)= 0;
		view[SIMULATION_VIEW_BLOCKED_OFFSET]= FALSE;
	}
	else if ((word)kind==_simulation_view_kind_1)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWNB0_OFFSET)= 0;
	}

	return view;
}

/* 0x00081550: a view is made: what it has of its channel and the numbers
after it are nothing, it is not ready, and its ring has no start and no
bytes */
void simulation_view_construct(
	byte *view)
{
	long index;

	view[SIMULATION_VIEW_UNKNOWN44_OFFSET]= FALSE;
	for (index= 0; index<3; index++)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN48_OFFSET+index*4)= 0;
	}
	for (index= 0; index<SIMULATION_VIEW_UNKNOWN54_COUNT; index++)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN54_OFFSET+index*4)= 0;
	}
	view[SIMULATION_VIEW_READY_OFFSET]= FALSE;
	*(long *)(view+SIMULATION_VIEW_UNKNOWN9C_OFFSET)= 0;
	*(long *)(view+SIMULATION_VIEW_UNKNOWNA0_OFFSET)= 0;
}

/* 0x00081590: what a view has at 0x8 is made: each of the five things in it
gets its functions, and the first four a byte that is clear */
void simulation_view_data_construct(
	byte *data)
{
	*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN4_OFFSET)= unknown_00450cf4;
	data[SIMULATION_VIEW_UNKNOWN8_UNKNOWN8_OFFSET]= FALSE;
	*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN30_OFFSET)= unknown_00450cd0;
	data[SIMULATION_VIEW_UNKNOWN8_UNKNOWN38_OFFSET]= FALSE;
	*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN5070_OFFSET)= unknown_00450d1c;
	data[SIMULATION_VIEW_DATA_UNKNOWN5078_OFFSET]= FALSE;
	*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN5098_OFFSET)= unknown_00450c94;
	data[SIMULATION_VIEW_DATA_UNKNOWN50A0_OFFSET]= FALSE;
	*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN5FB8_OFFSET)= unknown_00450d14;
}

/* 0x000821c0: a place is taken for a view in the table of the views
(datum_new), and the view is made; and if the views have something at 0x8,
the same place is taken in the other table (datum_new_at_index), and that is
made too. The number of the place, or NONE; where the view is, and where the
other thing is, or NULL */
long __attribute__((stdcall)) simulation_view_allocate(
	byte **view_out,
	byte **data_out)
{
	byte *view= NULL;
	byte *data= NULL;
	long datum_index= datum_new(unknown_004d87ec);

	if (datum_index!=NONE)
	{
		view= (byte *)unknown_004d87ec->data+((unsigned long)datum_index&0xFFFF)*SIMULATION_VIEW_SIZE;
		if (view)
		{
			simulation_view_construct(view);
		}
		if (unknown_004d87e4)
		{
			data= (byte *)unknown_004d87f0->data+
				((unsigned long)datum_new_at_index(unknown_004d87f0, datum_index)&0xFFFF)*SIMULATION_VIEW_DATA_SIZE;
			if (data)
			{
				simulation_view_data_construct(data);
			}
		}
	}
	*view_out= view;
	*data_out= data;

	return datum_index;
}
