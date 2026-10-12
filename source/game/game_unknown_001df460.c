/*
GAME_UNKNOWN_001DF460.C

a part of the game: it keeps 0xE4 bytes in the state of the game, of which the last
0x20 are sixteen rows of sixteen bits.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "game/game.h"
#include "saved games/game_state.h"

#include <math.h>
#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001DF460_SIZE= 0xE4,
	/* sixteen rows of sixteen bits, and where they are */
	GAME_UNKNOWN_001DF460_ROWS= 16,
	GAME_UNKNOWN_001DF460_BITS_OFFSET= 0xC4,
	/* sixteen more rows, before them: of two sides, whether they are friends;
	the first are whether they are not enemies */
	GAME_UNKNOWN_001DF460_FRIEND_BITS_OFFSET= 0xA4
};

/* ---------- globals */

/* 0x004f55ec: what it keeps */
byte *unknown_004f55ec;

/* ---------- code */

/* 0x001df460: what it keeps takes its memory of the state of the game, and
is nothing. The game has game_state_malloc written here */
void game_unknown_001df460_initialize(
	void)
{
	byte *unknown= game_state_malloc(GAME_UNKNOWN_001DF460_SIZE);

	memset(unknown, 0, GAME_UNKNOWN_001DF460_SIZE);
	unknown_004f55ec= unknown;
}

/* 0x001df4b0: for a new map, it is nothing, but for the bit of each row
that has the number of the row: each of sixteen things with itself */
void game_unknown_001df460_initialize_for_new_map(
	void)
{
	byte *unknown= unknown_004f55ec;
	unsigned long *bits= (unsigned long *)(unknown+GAME_UNKNOWN_001DF460_BITS_OFFSET);
	long index;

	memset(unknown, 0, GAME_UNKNOWN_001DF460_SIZE);
	for (index= 0; index<GAME_UNKNOWN_001DF460_ROWS; index++)
	{
		long bit= index*(GAME_UNKNOWN_001DF460_ROWS+1);

		bits[bit>>5]|= 1u<<(bit&31);
	}
}

/* 0x001df9c0: what is done with one of the things that are counted down,
when its time is over for the last time */
void __attribute__((stdcall)) game_team_change_apply(byte *element, boolean enemies, boolean not_friends);

/* 0x001df500: each tick of the game (game.c). What the part keeps starts
with how many things it counts down (a short) and has them from 4, of 0x14
bytes each: a short at 0xE that goes down each tick while it is above zero;
when it gets to zero, the short at 0xC goes down, and if that is zero too,
game_team_change_apply is done with the thing; if not, the first short starts
again from the one at 6. By the sixteen rows of sixteen bits it may be who is
whose friend, and these the changes of it that last for a time; that is a
guess */
void game_unknown_001df460_update(
	void)
{
	byte *unknown= unknown_004f55ec;
	byte *element= unknown+4;
	short index;

	for (index= 0; index<*(short *)unknown; index++)
	{
		short ticks= *(short *)(element+0xE);

		if (ticks>0)
		{
			ticks--;
			*(short *)(element+0xE)= ticks;
			if (ticks==0)
			{
				if (--*(short *)(element+0xC)==0)
				{
					game_team_change_apply(element, 0, 0);
					unknown= unknown_004f55ec;
				}
				else
				{
					*(short *)(element+0xE)= *(short *)(element+6);
				}
			}
		}
		element+= 0x14;
	}
}

/* 0x0015e020: whether two teams are enemies, in multiplayer */
boolean __attribute__((stdcall)) game_engine_teams_are_enemies(short team, short other);

/* 0x001df560: whether two sides are enemies; no side is one of everyone.
In the campaign they are unless their bit of the sixteen rows is set (sides
that are not from 0 to 15 are); in multiplayer game_engine_teams_are_enemies says; in any
other kind of game, two sides that are not the same are. So the rows of this
part are who is whose ally; the names are guesses */
boolean game_team_is_enemy(
	short team,
	short other)
{
	if (team==NONE || other==NONE)
	{
		return TRUE;
	}
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		if (team<0 || team>=GAME_UNKNOWN_001DF460_ROWS || other<0 || other>=GAME_UNKNOWN_001DF460_ROWS)
		{
			return TRUE;
		}

		return !bit_vector_test_flag((const unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_BITS_OFFSET),
			team*GAME_UNKNOWN_001DF460_ROWS+other);
	}
	if (main_game_globals->options.game_mode==_game_mode_multiplayer)
	{
		return game_engine_teams_are_enemies(team, other);
	}

	return team!=other;
}

/* 0x001df5d0: whether two sides are friends; no side is one of anyone. In
the campaign their bit of the other sixteen rows says; in multiplayer, that
they are not enemies; in any other kind of game, that they are the same */
boolean game_team_is_friend(
	short team,
	short other)
{
	if (team==NONE || other==NONE)
	{
		return FALSE;
	}
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		if (team<0 || team>=GAME_UNKNOWN_001DF460_ROWS || other<0 || other>=GAME_UNKNOWN_001DF460_ROWS)
		{
			return FALSE;
		}

		return bit_vector_test_flag((const unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_FRIEND_BITS_OFFSET),
			team*GAME_UNKNOWN_001DF460_ROWS+other);
	}
	if (main_game_globals->options.game_mode==_game_mode_multiplayer)
	{
		return !game_engine_teams_are_enemies(team, other);
	}

	return team==other;
}

/* 0x001df640: whether two sides, which are not the same, are friends and
not allies: only in the campaign, and for sides from 0 to 15 */
boolean game_team_is_friend_and_not_ally(
	short team,
	short other)
{
	if (team!=NONE && other!=NONE && team!=other &&
		main_game_globals->options.game_mode==_game_mode_campaign &&
		team>=0 && team<GAME_UNKNOWN_001DF460_ROWS && other>=0 && other<GAME_UNKNOWN_001DF460_ROWS)
	{
		long bit= team*GAME_UNKNOWN_001DF460_ROWS+other;

		if (bit_vector_test_flag((const unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_FRIEND_BITS_OFFSET), bit) &&
			!bit_vector_test_flag((const unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_BITS_OFFSET), bit))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001df6c0: in the campaign, one of the things that are counted down (at
most eight) is of two sides, in either order: the one that there is of them,
or a new one. It gets two bytes (at 8 and 9) and two shorts (at 4 and 6: the
second is where its time starts again from), its counts are zero, the byte at
0xA is set, game_team_change_apply is done with it and the byte at 0xB is cleared */
void game_team_unknown_001df6c0(
	short team,
	short other,
	byte unknown8,
	byte unknown9,
	short unknown4,
	short unknown6)
{
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		byte *unknown= unknown_004f55ec;
		short count= *(short *)unknown;
		byte *element= unknown+4;
		short index;

		for (index= 0; index<*(short *)unknown; index++)
		{
			if ((*(short *)element==team && *(short *)(element+2)==other) ||
				(*(short *)(element+2)==team && *(short *)element==other))
			{
				break;
			}
			element+= 0x14;
		}
		if (index>=count && count<8)
		{
			index= count;
			*(short *)unknown= (short)(count+1);
		}
		if (index<*(short *)unknown)
		{
			element= unknown+4+index*0x14;
			element[9]= unknown9;
			element[8]= unknown8;
			*(short *)element= team;
			*(short *)(element+2)= other;
			*(short *)(element+4)= unknown4;
			*(short *)(element+6)= unknown6;
			*(short *)(element+0xC)= 0;
			*(short *)(element+0xE)= 0;
			element[0xA]= TRUE;
			game_team_change_apply(element, 0, 0);
			element[0xB]= FALSE;
		}
	}
}

/* 0x001c9830: the minds of the characters are told of a change between two
sides */
void __attribute__((stdcall)) unknown_001c9830(long team, long other, boolean enemies, boolean not_friends);

/* 0x001dfaa0: a bit of some is set, or cleared */
void game_team_bit_set(
	unsigned long *bits,
	long bit,
	boolean value)
{
	if (value)
	{
		BIT_VECTOR_OR_FLAG(bits, bit);
	}
	else
	{
		BIT_VECTOR_AND_FLAG(bits, bit);
	}
}

/* 0x001df9c0: a change between two sides is put in the rows of bits: the two
are friends, each of the other, unless the second argument says not, and
allies unless the first says that they are enemies (only for sides below 16);
the change keeps the first (at 0xA) and is marked (at 0xB), and the minds of
the characters are told. If they are friends and the change already said the
same of enemies, nothing is done */
void __attribute__((stdcall)) game_team_change_apply(
	byte *element,
	boolean enemies,
	boolean not_friends)
{
	if (not_friends || element[0xA]!=enemies)
	{
		short team= *(short *)element;

		element[0xA]= enemies;
		if (team<GAME_UNKNOWN_001DF460_ROWS && *(short *)(element+2)<GAME_UNKNOWN_001DF460_ROWS)
		{
			unsigned long *friends= (unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_FRIEND_BITS_OFFSET);
			unsigned long *allies= (unsigned long *)(unknown_004f55ec+GAME_UNKNOWN_001DF460_BITS_OFFSET);

			game_team_bit_set(friends, team*GAME_UNKNOWN_001DF460_ROWS+*(short *)(element+2), !not_friends);
			game_team_bit_set(friends, *(short *)(element+2)*GAME_UNKNOWN_001DF460_ROWS+*(short *)element, !not_friends);
			game_team_bit_set(allies, *(short *)element*GAME_UNKNOWN_001DF460_ROWS+*(short *)(element+2), !enemies);
			game_team_bit_set(allies, *(short *)(element+2)*GAME_UNKNOWN_001DF460_ROWS+*(short *)element, !enemies);
		}
		element[0xB]= TRUE;
		unknown_001c9830(*(unsigned short *)element, *(unsigned short *)(element+2), enemies, not_friends);
	}
}

/* 0x001df950: in the campaign, the time of the change of two sides starts
again (from its short at 6, unless that is NONE), if the change has been
counted at least once (its short at 0xC). The change is the first that is of
the two sides, with its byte at 9 set when they are in its order, or its byte
at 8 when they are the other way round */
void game_team_unknown_001df950(
	short team,
	short other)
{
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		byte *element= unknown_004f55ec+4;
		short count= *(short *)unknown_004f55ec;
		short index;

		for (index= 0; index<count; index++)
		{
			if ((*(short *)element==team && *(short *)(element+2)==other && element[9]) ||
				(*(short *)(element+2)==team && *(short *)element==other && element[8]))
			{
				if (*(short *)(element+0xC)>0 && *(short *)(element+6)!=NONE)
				{
					*(short *)(element+0xE)= *(short *)(element+6);
				}
				break;
			}
			element+= 0x14;
		}
	}
}

/* 0x001df820: in the campaign, something that one side did to another is
counted in the change of the two (found as game_team_unknown_001df950 finds
it), if a fifth of a second has gone by since the last that was counted: 1, 3
or one less, by its kind (0, 1 or 2). The time of the change starts again;
and when the count gets to what the change has at 4 (unless that is NONE),
the two sides are enemies, and it is true */
boolean __attribute__((stdcall)) game_team_unknown_001df820(
	short team,
	short other,
	short kind)
{
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		byte *unknown= unknown_004f55ec;
		byte *element= unknown+4;
		short index;

		for (index= 0; index<*(short *)unknown; index++)
		{
			if ((*(short *)element==team && *(short *)(element+2)==other && element[9]) ||
				(*(short *)(element+2)==team && *(short *)element==other && element[8]))
			{
				long time= (long)game_time_globals->time;

				if (time-*(long *)(element+0x10)>lrintf((float)game_time_globals->ticks_per_second*0.2f))
				{
					short amount= 0;

					*(long *)(element+0x10)= time;
					switch (kind)
					{
					case 0:
						amount= 1;
						break;
					case 1:
						amount= 3;
						break;
					case 2:
						amount= NONE;
						break;
					}
					*(short *)(element+0xC)+= amount;
					if (*(short *)(element+6)!=NONE)
					{
						*(short *)(element+0xE)= *(short *)(element+6);
					}
					if (*(short *)(element+4)!=NONE && *(short *)(element+0xC)>=*(short *)(element+4))
					{
						game_team_change_apply(element, TRUE, FALSE);
						unknown_001c9830(*(unsigned short *)element, *(unsigned short *)(element+2), TRUE, FALSE);

						return TRUE;
					}

					return FALSE;
				}
			}
			element+= 0x14;
		}
	}

	return FALSE;
}
