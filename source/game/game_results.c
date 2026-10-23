/*
GAME_RESULTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game keeps of each player and each team of a game: a thing of 0xA4
bytes for each of 16 players, one of 0x18 for each of 16 teams, and three
tables of numbers of 15 bits: 0x1B5 for each player, two for each pair of
players, and 0x2D for each team. It looks like what the report after a game
shows. What most of it means is not known, and the name of the file is made
up.
*/

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	GAME_RESULTS_PLAYERS= 16,
	GAME_RESULTS_TEAMS= 16,
	/* the teams that most functions take */
	GAME_RESULTS_TEAMS_IN_USE= 8,
	GAME_RESULTS_PLAYER_SIZE= 0xA4,
	GAME_RESULTS_TEAM_SIZE= 0x18,
	GAME_RESULTS_PLAYER_STATISTICS= 0x1B5,
	GAME_RESULTS_TEAM_STATISTICS= 0x2D,

	/* in what is kept of a player: whether it is in use; a text of two-byte
	letters at 2 and another at 0x10 (its names, most likely); 16 bytes;
	its team; two more numbers */
	GAME_RESULTS_PLAYER_UNKNOWN2_OFFSET= 2,
	GAME_RESULTS_PLAYER_UNKNOWN10_OFFSET= 0x10,
	GAME_RESULTS_PLAYER_UNKNOWN50_OFFSET= 0x50,
	GAME_RESULTS_PLAYER_TEAM_OFFSET= 0x8C,
	GAME_RESULTS_PLAYER_UNKNOWN98_OFFSET= 0x98,
	GAME_RESULTS_PLAYER_UNKNOWNA0_OFFSET= 0xA0,
	GAME_RESULTS_PLAYER_UNKNOWNA2_OFFSET= 0xA2,

	/* in what is kept of a team: whether it is in use, a byte and a short */
	GAME_RESULTS_TEAM_UNKNOWN1_OFFSET= 1,
	GAME_RESULTS_TEAM_UNKNOWN2_OFFSET= 2
};

/* ---------- globals */

/* 0x00510ca0, 0x00510cb1, of simulation_view.c */
extern boolean unknown_00510ca0;
extern boolean unknown_00510cb1;

/* 0x00510cb0: with unknown_00510cb1, whether there is something to read */
boolean unknown_00510cb0;

/* 0x00510e2c: a number that game_results_get_unknown_00510e2c gives */
long unknown_00510e2c;

/* 0x00510e30: its bit 0 says that the players are in teams, most likely */
byte unknown_00510e30;

/* 0x00511034: what is kept of each player */
byte unknown_00511034[GAME_RESULTS_PLAYERS][GAME_RESULTS_PLAYER_SIZE];

/* 0x00511a74: what is kept of each team */
byte unknown_00511a74[GAME_RESULTS_TEAMS][GAME_RESULTS_TEAM_SIZE];

/* 0x00511bf4: the numbers of each player */
word unknown_00511bf4[GAME_RESULTS_PLAYERS][GAME_RESULTS_PLAYER_STATISTICS];

/* 0x00515294: two numbers for each pair of players */
word unknown_00515294[GAME_RESULTS_PLAYERS][GAME_RESULTS_PLAYERS][2];

/* 0x00515694: the numbers of each team */
word unknown_00515694[GAME_RESULTS_TEAMS][GAME_RESULTS_TEAM_STATISTICS];

/* 0x00450aac: a text with nothing, of two-byte letters */
const word unknown_00450aac[2]= {0, 0};

/* ---------- code */

/* whether there is something to read */
static inline __attribute__((always_inline)) boolean game_results_readable(
	void)
{
	return unknown_00510cb0 && unknown_00510cb1;
}

/* 0x00196960: one of the numbers of a team, if a team is given; if not, of
a player; NONE if neither is */
long game_results_get_statistic(
	long player_index,
	long team_index,
	long statistic)
{
	long result= NONE;

	if (player_index!=NONE)
	{
		result= (&unknown_00511bf4[0][0])[player_index*GAME_RESULTS_PLAYER_STATISTICS+statistic]&0x7FFF;
	}
	if (team_index!=NONE)
	{
		result= (&unknown_00515694[0][0])[team_index*GAME_RESULTS_TEAM_STATISTICS+statistic]&0x7FFF;
	}

	return result;
}

/* 0x001969d0: one of the numbers of a player that are in rows of eight from
the 0x4E: the one at a column of a row; NONE if there is nothing to read */
long __attribute__((stdcall)) game_results_get_player_unknown4E(
	long player_index,
	long column,
	long row)
{
	if (game_results_readable())
	{
		return (&unknown_00511bf4[0][0])[player_index*GAME_RESULTS_PLAYER_STATISTICS+column+row*8+0x4E]&0x7FFF;
	}

	return NONE;
}

/* 0x00196a10: one of the numbers of a player from the 0x2D on */
long __attribute__((stdcall)) game_results_get_player_unknown2D(
	long player_index,
	long index)
{
	if (game_results_readable())
	{
		return (&unknown_00511bf4[0][0])[player_index*GAME_RESULTS_PLAYER_STATISTICS+index+0x2D]&0x7FFF;
	}

	return NONE;
}

/* 0x00196a50: one of the two numbers of a pair of players */
long __attribute__((stdcall)) game_results_get_player_versus_player(
	long player_index,
	long other_player_index,
	long index)
{
	if (game_results_readable())
	{
		return (&unknown_00515294[0][0][0])[(player_index*GAME_RESULTS_PLAYERS+other_player_index)*2+index]&0x7FFF;
	}

	return NONE;
}

/* 0x00196a90: unknown_00510e2c, or 0 if there is nothing to read */
long game_results_get_unknown_00510e2c(
	void)
{
	if (game_results_readable())
	{
		return unknown_00510e2c;
	}

	return 0;
}

/* 0x00196ab0: a player that is in use takes a short at 0xA2 and a byte at
0xA0 (its score and its place, most likely). Only when unknown_00510ca0 is
set and unknown_00510cb1 is not */
void __attribute__((stdcall)) game_results_set_player_unknownA2(
	long player_index,
	short unknownA2,
	byte unknownA0)
{
	if (unknown_00510ca0 && !unknown_00510cb1 && player_index!=NONE)
	{
		byte *player= unknown_00511034[0]+player_index*GAME_RESULTS_PLAYER_SIZE;

		if (player[0])
		{
			*(short *)(player+GAME_RESULTS_PLAYER_UNKNOWNA2_OFFSET)= unknownA2;
			player[GAME_RESULTS_PLAYER_UNKNOWNA0_OFFSET]= unknownA0;
		}
	}
}

/* 0x00196b00: the same for one of the first eight teams: a number, kept as
a short (the nearest that a short can be), at 2, and a byte at 1 */
void __attribute__((stdcall)) game_results_set_team_unknown2(
	long team_index,
	long unknown2,
	byte unknown1)
{
	if (unknown_00510ca0 && !unknown_00510cb1 && team_index>=0 && team_index<GAME_RESULTS_TEAMS_IN_USE)
	{
		byte *team= unknown_00511a74[0]+team_index*GAME_RESULTS_TEAM_SIZE;

		if (team[0])
		{
			if (unknown2<-0x8000)
			{
				unknown2= -0x8000;
			}
			else if (unknown2>0x7FFF)
			{
				unknown2= 0x7FFF;
			}
			*(short *)(team+GAME_RESULTS_TEAM_UNKNOWN2_OFFSET)= (short)unknown2;
			team[GAME_RESULTS_TEAM_UNKNOWN1_OFFSET]= unknown1;
		}
	}
}

/* a player that can be read: one that is in use, when there is something to
read; NULL if not */
static inline __attribute__((always_inline)) byte *game_results_player_try_and_get(
	long player_index)
{
	if (game_results_readable() && player_index!=NONE)
	{
		byte *player= unknown_00511034[0]+player_index*GAME_RESULTS_PLAYER_SIZE;

		if (player[0])
		{
			return player;
		}
	}

	return NULL;
}

/* one of the first eight teams, the same way */
static inline __attribute__((always_inline)) const byte *game_results_team_try_and_get(
	long team_index)
{
	if (game_results_readable() && team_index>=0 && team_index<GAME_RESULTS_TEAMS_IN_USE)
	{
		const byte *team= unknown_00511a74[0]+team_index*GAME_RESULTS_TEAM_SIZE;

		if (team[0])
		{
			return team;
		}
	}

	return NULL;
}

/* 0x00196b70: the short at 0xA2 of a player; 0 if it cannot be read */
long game_results_get_player_unknownA2(
	long player_index)
{
	const byte *player= game_results_player_try_and_get(player_index);

	if (player)
	{
		return *(const short *)(player+GAME_RESULTS_PLAYER_UNKNOWNA2_OFFSET);
	}

	return 0;
}

/* 0x00196bb0: the short at 2 of a team; 0 if it cannot be read */
long game_results_get_team_unknown2(
	long team_index)
{
	const byte *team= game_results_team_try_and_get(team_index);

	if (team)
	{
		return *(const short *)(team+GAME_RESULTS_TEAM_UNKNOWN2_OFFSET);
	}

	return 0;
}

/* 0x00196c60: the byte at 1 of a team; 0 if it cannot be read */
long game_results_get_team_unknown1(
	long team_index)
{
	const byte *team= game_results_team_try_and_get(team_index);

	if (team)
	{
		return *(const char *)(team+GAME_RESULTS_TEAM_UNKNOWN1_OFFSET);
	}

	return 0;
}

/* 0x00196bf0: the byte at 0xA0 of a player; or, when unknown_00510e30 has
its bit 0, the byte at 1 of its team (any of the sixteen, if it is in use).
NONE if the player cannot be read, it has no team or the team is not in
use */
long game_results_get_player_unknownA0(
	long player_index)
{
	const byte *player= game_results_player_try_and_get(player_index);

	if (player)
	{
		long team_index;
		const byte *team;

		if (!(unknown_00510e30&1))
		{
			return *(const char *)(player+GAME_RESULTS_PLAYER_UNKNOWNA0_OFFSET);
		}
		team_index= *(const char *)(player+GAME_RESULTS_PLAYER_TEAM_OFFSET);
		if (team_index>=0 && team_index<GAME_RESULTS_TEAMS)
		{
			team= unknown_00511a74[0]+team_index*GAME_RESULTS_TEAM_SIZE;
			if (team[0])
			{
				return *(const char *)(team+GAME_RESULTS_TEAM_UNKNOWN1_OFFSET);
			}
		}
	}

	return NONE;
}

/* 0x00196ca0: the text at 0x10 of a player, or one with nothing if the
player cannot be read */
const word *game_results_get_player_unknown10(
	long player_index)
{
	const byte *player= game_results_player_try_and_get(player_index);

	if (player)
	{
		return (const word *)(player+GAME_RESULTS_PLAYER_UNKNOWN10_OFFSET);
	}

	return unknown_00450aac;
}

/* 0x00196ce0: the text at 2 of a player, or NULL */
const word *game_results_get_player_unknown2(
	long player_index)
{
	const byte *player= game_results_player_try_and_get(player_index);

	if (player)
	{
		return (const word *)(player+GAME_RESULTS_PLAYER_UNKNOWN2_OFFSET);
	}

	return NULL;
}

/* a player that is in use, when unknown_00510cb1 is set; these three do not
look at unknown_00510cb0 */
static inline __attribute__((always_inline)) const byte *game_results_player_try_and_get_unknown_00510cb1(
	long player_index)
{
	if (unknown_00510cb1 && player_index!=NONE)
	{
		const byte *player= unknown_00511034[0]+player_index*GAME_RESULTS_PLAYER_SIZE;

		if (player[0])
		{
			return player;
		}
	}

	return NULL;
}

/* 0x00196d20: the team of a player, or NONE */
long game_results_get_player_team(
	long player_index)
{
	const byte *player= game_results_player_try_and_get_unknown_00510cb1(player_index);

	if (player)
	{
		return *(const char *)(player+GAME_RESULTS_PLAYER_TEAM_OFFSET);
	}

	return NONE;
}

/* 0x00196da0: the short at 0x98 of a player, or NONE */
long game_results_get_player_unknown98(
	long player_index)
{
	const byte *player= game_results_player_try_and_get_unknown_00510cb1(player_index);

	if (player)
	{
		return *(const short *)(player+GAME_RESULTS_PLAYER_UNKNOWN98_OFFSET);
	}

	return NONE;
}

/* 0x00196d50: the 16 bytes at 0x50 of a player, or zeros */
void game_results_get_player_unknown50(
	long player_index,
	long *unknown50)
{
	const byte *player= game_results_player_try_and_get_unknown_00510cb1(player_index);

	if (player)
	{
		const long *from= (const long *)(player+GAME_RESULTS_PLAYER_UNKNOWN50_OFFSET);

		unknown50[0]= from[0];
		unknown50[1]= from[1];
		unknown50[2]= from[2];
		unknown50[3]= from[3];
	}
	else
	{
		unknown50[0]= 0;
		unknown50[1]= 0;
		unknown50[2]= 0;
		unknown50[3]= 0;
	}
}

/* 0x001969a0: one of the numbers of a team or of a player
(game_results_get_statistic), if there is something to read; NONE if not */
long __attribute__((stdcall)) game_results_get_statistic_if_readable(
	long player_index,
	long team_index,
	long statistic)
{
	if (!game_results_readable())
	{
		return NONE;
	}

	return game_results_get_statistic(player_index, team_index, statistic);
}
