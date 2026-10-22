/*
NETWORK_HOPPERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A list of sixteen things that one of the blocks of the service of games
fills. The names are guesses.
*/

#include "network_hoppers.h"

#include "cache/cache_files.h"
#include "networking/network_blob.h"
#include "tag_files/files.h"
#include "text/unicode.h"

#include <string.h>

/* ---------- prototypes */


/* 0x00194660: something is done with one of the list whose first number
is nothing */
void __attribute__((stdcall)) unknown_00194660(long index);

/* 0x001943d0: the games that one of the list can play; whether there are
any. 0x0019c5f0: something of a map, by its number, if the console has the
map; NULL if not */
boolean __attribute__((stdcall)) unknown_001943d0(long hopper_index, struct network_hopper_game_set *set);
const byte *__attribute__((stdcall)) levels_get_multiplayer_level(long map);

/* ---------- globals */

/* 0x00551ae8 */
struct network_hopper network_hoppers[MAXIMUM_NETWORK_HOPPERS];

/* 0x0047d8f4, of online_tasks.c: what has the block that fills the list */
extern struct network_blob unknown_0047d8f4;

/* ---------- code */

/* whether there is a list: its block is of a kind and is good */
static inline __attribute__((always_inline)) boolean network_hoppers_available(
	void)
{
	return unknown_0047d8f4.kind!=0 && (unknown_0047d8f4.flags&_network_blob_good_bit);
}

/* 0x00192db0: whether one of the list is there and is good */
boolean network_hoppers_index_valid(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		return network_hopper_valid(&network_hoppers[index]);
	}

	return FALSE;
}

/* 0x00192e60: one of the list, if it is there and is good; NULL if not */
struct network_hopper *network_hoppers_get(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		struct network_hopper *hopper= &network_hoppers[index];

		if (network_hopper_valid(hopper))
		{
			return hopper;
		}
	}

	return NULL;
}

/* 0x001933a0: a number of one of the fifth kind, or of one of the fourth
that has it; nothing for the others */
long network_hopper_get_unknown5F4(
	const struct network_hopper *hopper)
{
	if (hopper->type==_network_hopper_type5)
	{
		return hopper->unknown5F4;
	}
	if (hopper->type==_network_hopper_type4 && hopper->unknown604)
	{
		return hopper->unknown60C;
	}

	return 0;
}

/* 0x001933d0: the most players that one of the fifth kind takes, or the
number of one of the fourth that has it; nothing for the others */
long network_hopper_get_maximum(
	const struct network_hopper *hopper)
{
	if (hopper->type==_network_hopper_type5)
	{
		return hopper->maximum_players;
	}
	if (hopper->type==_network_hopper_type4 && hopper->unknown604)
	{
		return hopper->unknown608;
	}

	return 0;
}

/* 0x00193470: whether one is of the second, fourth or fifth kind */
boolean network_hopper_unknown_00193470(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type5 || hopper->type==_network_hopper_type2 ||
		hopper->type==_network_hopper_type4;
}

/* 0x00193490: whether one is of the first or third kind */
boolean network_hopper_unknown_00193490(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type1 || hopper->type==_network_hopper_type3;
}

/* 0x001934b0: whether one is of the third, fourth or fifth kind */
boolean network_hopper_unknown_001934b0(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type5 || hopper->type==_network_hopper_type3 ||
		hopper->type==_network_hopper_type4;
}

/* 0x001934d0: whether one is of the fifth kind, or of the fourth and has
its two numbers */
boolean network_hopper_unknown_001934d0(
	const struct network_hopper *hopper)
{
	return (hopper->type==_network_hopper_type4 && hopper->unknown604) || hopper->type==_network_hopper_type5;
}

/* 0x00193610: whether the players that one takes are right: one at least,
sixteen at most, and no more at least than at most */
boolean network_hopper_players_valid(
	const struct network_hopper *hopper)
{
	return hopper->minimum_players>=1 && hopper->maximum_players<=NETWORK_HOPPER_MAXIMUM_PLAYERS &&
		hopper->minimum_players<=hopper->maximum_players;
}

/* 0x001932c0: the fewest players that one takes: of the first two kinds,
what it says; of the others, its fewest teams by the players of a team. The
game does not look at whether the kind is one of the five, here or in the
functions after this one */
long network_hopper_get_minimum_players(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return hopper->minimum_players;
	default:
		return hopper->maximum_players*hopper->minimum_players;
	}
}

/* 0x00193300: the most players that one takes: of the first two kinds,
what it says; of the others, its most teams by the players of a team */
long network_hopper_get_maximum_players(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return hopper->maximum_players;
	default:
		return hopper->unknown5F4*hopper->minimum_players;
	}
}

/* 0x001931d0: the number that one has for a number of players: nothing for
the most that it takes, and a very big number for what it does not take */
long network_hopper_get_players_value(
	const struct network_hopper *hopper,
	long player_count)
{
	long maximum= network_hopper_get_maximum_players(hopper);
	long minimum= network_hopper_get_minimum_players(hopper);

	if (player_count<minimum || player_count>maximum)
	{
		return NETWORK_HOPPER_PLAYERS_VALUE_NONE;
	}
	if (player_count==maximum)
	{
		return 0;
	}

	return hopper->unknown5A0[player_count];
}

/* 0x00193250: a number of one, by its kind: its most players for the first
and the third; 1 for the second; and two numbers of its own for the fourth
and the fifth */
long network_hopper_unknown_00193250(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
		return hopper->maximum_players;
	case _network_hopper_type2:
		return 1;
	case _network_hopper_type3:
		return hopper->unknown5F4*hopper->minimum_players;
	case _network_hopper_type4:
		return hopper->unknown600;
	default:
		return hopper->unknown5F4;
	}
}

/* 0x00193340: the fewest teams that one takes; 1 for the first two kinds */
long network_hopper_get_minimum_teams(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 1;
	default:
		return hopper->maximum_players;
	}
}

/* 0x00193370: the most teams that one takes; 1 for the first two kinds */
long network_hopper_get_maximum_teams(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 1;
	default:
		return hopper->unknown5F4;
	}
}

/* 0x00193400: something of one: nothing for the first two kinds; whether
its byte is set, for the third; and its number, for the others */
long network_hopper_unknown_00193400(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 0;
	case _network_hopper_type3:
		return (byte)hopper->unknown5F8 ? 1 : 0;
	default:
		return hopper->unknown5F8;
	}
}

/* 0x00193440: how many players a team of one has; NONE for the first two
kinds, which have no teams */
long network_hopper_get_team_size(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return NONE;
	default:
		return hopper->minimum_players;
	}
}

/* 0x001934f0: whether one of the list is good: as all are, and as the ones
of its kind are */
boolean network_hopper_valid(
	const struct network_hopper *hopper)
{
	if (!network_hopper_header_valid(hopper))
	{
		return FALSE;
	}
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return network_hopper_players_valid(hopper)!=0;
	case _network_hopper_type4:
		return network_hopper_type4_valid(hopper)!=0;
	default:
		return network_hopper_teams_valid(hopper)!=0;
	}
}

/* 0x00193560: whether one of the list is good, whatever its kind: the kind
is one of the five, its two numbers are what they can be, and each of its
128 bytes is at least its place among them */
boolean network_hopper_header_valid(
	const struct network_hopper *hopper)
{
	boolean valid= FALSE;
	long index;

	if ((hopper->type==_network_hopper_type2 || hopper->type==_network_hopper_type1 ||
		hopper->type==_network_hopper_type4 || hopper->type==_network_hopper_type3 ||
		hopper->type==_network_hopper_type5) &&
		hopper->unknown4>=0 && hopper->unknown4<NUMBER_OF_NETWORK_HOPPER_UNKNOWN4 &&
		hopper->unknown8>0 && hopper->unknown8<=NETWORK_HOPPER_UNKNOWN8_MAXIMUM)
	{
		valid= TRUE;
	}
	for (index= 0; index<NUMBER_OF_NETWORK_HOPPER_UNKNOWN51C; index++)
	{
		if (hopper->unknown51C[index]<index)
		{
			valid= FALSE;
		}
	}

	return valid;
}

/* 0x00193630: whether one of the fourth kind is good: as the ones with
teams are, but with two teams at least; and its three other numbers of teams
are right */
boolean network_hopper_type4_valid(
	const struct network_hopper *hopper)
{
	long team_size= hopper->minimum_players;
	long minimum_teams= hopper->maximum_players;
	long maximum_teams= hopper->unknown5F4;

	return team_size>1 && team_size<=NETWORK_HOPPER_MAXIMUM_PLAYERS && minimum_teams>1 &&
		maximum_teams<=NETWORK_HOPPER_MAXIMUM_PLAYERS/team_size && minimum_teams<=maximum_teams &&
		hopper->unknown5FC>0 && hopper->unknown600<=maximum_teams && hopper->unknown5FC<=hopper->unknown600 &&
		hopper->unknown5F8>=0 && hopper->unknown5F8<=maximum_teams-minimum_teams;
}

/* 0x001936a0: whether one with teams is good: from two to sixteen players
in a team, one team at least, no more teams than make sixteen players, and
no more at least than at most */
boolean network_hopper_teams_valid(
	const struct network_hopper *hopper)
{
	long team_size= hopper->minimum_players;
	long minimum_teams= hopper->maximum_players;
	long maximum_teams= hopper->unknown5F4;

	return team_size>1 && team_size<=NETWORK_HOPPER_MAXIMUM_PLAYERS && minimum_teams>=1 &&
		maximum_teams<=NETWORK_HOPPER_MAXIMUM_PLAYERS/team_size && minimum_teams<=maximum_teams;
}

/* 0x001931a0: from a number down, the first place of the 128 bytes of one
whose byte before it is less than the number; nothing if there is none, and
the number itself if it is not more than nothing */
long network_hopper_unknown_001931a0(
	long number,
	const struct network_hopper *hopper)
{
	long index= number;

	while (index>0 && hopper->unknown51C[index-1]>=number)
	{
		index--;
	}

	return index;
}

/* 0x00192df0: the first number of one of the list; nothing for NONE, and 3
for one that is not there or is not good. Something is done with one whose
number is nothing (unknown_00194660). The game asks three times whether it
is good */
long network_hoppers_get_unknown4(
	long index)
{
	if (index==NONE)
	{
		return 0;
	}
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		const struct network_hopper *hopper= &network_hoppers[index];

		if (network_hopper_valid(hopper))
		{
			if (network_hopper_valid(hopper) && hopper->unknown4==0)
			{
				unknown_00194660(index);
			}
			if (network_hopper_valid(hopper))
			{
				return hopper->unknown4;
			}
		}
	}

	return NETWORK_HOPPER_UNKNOWN4_NONE;
}

/* 0x00192eb0: the game that one of the list plays next: one of its games,
taken at random by how much each weighs (from 1 to 1000), of the ones whose
map the console has. Its map and its rules; whether there is one */
boolean __attribute__((stdcall)) network_hopper_choose_game(
	long hopper_index,
	long *map,
	struct game_variant *variant)
{
	struct network_hopper_game_set set;
	long weights[MAXIMUM_NETWORK_HOPPER_GAMES];
	unsigned long seed;
	long chosen= NONE;
	long total= 0;
	long running= 0;
	long index;
	long pick;

	if (!unknown_001943d0(hopper_index, &set))
	{
		return FALSE;
	}
	if (set.count<=0)
	{
		return FALSE;
	}
	index= 0;
	do
	{
		long weight= 0;

		if (levels_get_multiplayer_level(set.games[index].map))
		{
			weight= set.games[index].weight;
			if (weight<NETWORK_HOPPER_GAME_WEIGHT_MINIMUM)
			{
				weight= NETWORK_HOPPER_GAME_WEIGHT_MINIMUM;
			}
			else if (weight>NETWORK_HOPPER_GAME_WEIGHT_MAXIMUM)
			{
				weight= NETWORK_HOPPER_GAME_WEIGHT_MAXIMUM;
			}
		}
		total+= weight;
		weights[index]= weight;
		index++;
	} while (index<set.count);
	if (total<=0)
	{
		return FALSE;
	}
	seed= global_random_seed_reference[1]*0x19660D+0x3C6EF35F;
	global_random_seed_reference[1]= seed;
	pick= (short)(((unsigned long)((long)(short)(total+1)-1)*(seed>>16)>>16)+1);
	for (index= 0; index<set.count && chosen==NONE; index++)
	{
		running+= weights[index];
		if (running>=pick)
		{
			chosen= index;
		}
	}
	if (chosen<0 || chosen>=set.count)
	{
		return FALSE;
	}
	*map= set.games[chosen].map;
	*variant= set.games[chosen].variant;

	return TRUE;
}

/* ---------- the list as a whole, and two texts of each one */

enum
{
	/* in what one of the list has from 0xC: a name of 16 bytes in each
	language, and then a text of 128 bytes in each */
	NETWORK_HOPPER_NAME_SIZE= 0x10,
	NETWORK_HOPPER_DESCRIPTIONS_OFFSET= 0x90,
	NETWORK_HOPPER_DESCRIPTION_SIZE= 0x80,
	/* the number at 0x5E4 and the byte at 0x5E8, which are the last two of
	unknown5A0 */
	NETWORK_HOPPER_UNKNOWN5E4_INDEX= 17,
	NETWORK_HOPPER_UNKNOWN5E8_INDEX= 18
};

/* 0x0033039a: the language of the console */
long XGetLanguage(void);

/* the language of the game, worked out once from that of the console. The
game has this in each function that uses it */
static inline __attribute__((always_inline)) long network_hoppers_language(
	void)
{
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}

	return global_language;
}

/* 0x00193f50: whether there is a list (its block is of a kind and is good) */
boolean network_hoppers_list_available(
	void)
{
	return network_hoppers_available();
}

/* 0x00193f70: whether the list is being read (its block is of a kind and at
its first step); how far it went, if it is and that is asked for */
boolean network_hoppers_list_reading(
	real *progress)
{
	if (unknown_0047d8f4.kind==0 || unknown_0047d8f4.state!=1)
	{
		return FALSE;
	}
	if (progress)
	{
		network_blob_get_progress(&unknown_0047d8f4, progress);
	}

	return TRUE;
}

/* 0x001944c0: the name of one of the list, in the language of the game (16
characters of each language, in UTF-8); FALSE if it is not there or is not
good */
boolean __attribute__((stdcall)) network_hoppers_get_name(
	long index,
	wchar *name)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available() &&
		network_hopper_valid(&network_hoppers[index]))
	{
		long language= network_hoppers_language();

		utf8_string_to_wchar_string((const char *)network_hoppers[index].unknownC+language*NETWORK_HOPPER_NAME_SIZE, name,
			NETWORK_HOPPER_NAME_SIZE);

		return TRUE;
	}

	return FALSE;
}

/* 0x00194540: the same with the longer text of one of the list (128
characters of each language) */
boolean __attribute__((stdcall)) network_hoppers_get_description(
	long index,
	wchar *description)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available() &&
		network_hopper_valid(&network_hoppers[index]))
	{
		long language= network_hoppers_language();

		utf8_string_to_wchar_string((const char *)network_hoppers[index].unknownC+NETWORK_HOPPER_DESCRIPTIONS_OFFSET+
			language*NETWORK_HOPPER_DESCRIPTION_SIZE, description, NETWORK_HOPPER_DESCRIPTION_SIZE);

		return TRUE;
	}

	return FALSE;
}

/* 0x001945c0: the number at 0x5E4 of one of the list; nothing if it is not
there or is not good */
long network_hoppers_get_unknown5E4(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available() &&
		network_hopper_valid(&network_hoppers[index]))
	{
		return network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E4_INDEX];
	}

	return 0;
}

/* 0x00194610: the byte at 0x5E8 of one of the list; FALSE if it is not
there or is not good */
boolean network_hoppers_get_unknown5E8(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available() &&
		network_hopper_valid(&network_hoppers[index]))
	{
		return *(const byte *)&network_hoppers[index].unknown5A0[NETWORK_HOPPER_UNKNOWN5E8_INDEX];
	}

	return FALSE;
}

/* ---------- the numbers for each number of players, the file and one by
its number */

/* 0x0046dd5c: the name of the file of the list, on the hard disk */
const char *network_hoppers_file_name= "multiplayer_hopper.dat";

/* 0x001939f0: the numbers of a list for each number of players from 1 to
16 that do not go by teams: a very big number for what it does not take,
nothing for the most that it takes, and in between a part of a number,
less for more players */
void __attribute__((stdcall)) network_hopper_make_players_values(
	long scale,
	long minimum_players,
	long maximum_players,
	long *values)
{
	unsigned long numerator= (unsigned long)(maximum_players-1)*(unsigned long)scale;
	long player_count;

	for (player_count= 1; player_count<=NETWORK_HOPPER_MAXIMUM_PLAYERS; player_count++)
	{
		long value;

		if (player_count<minimum_players || player_count>maximum_players)
		{
			value= NETWORK_HOPPER_PLAYERS_VALUE_NONE;
		}
		else if (player_count==maximum_players)
		{
			value= 0;
		}
		else
		{
			value= (long)numerator/(maximum_players-minimum_players);
		}
		values[player_count-1]= value;
		numerator-= (unsigned long)scale;
	}
}

/* 0x00193a50: the same for one that goes by teams: from the fewest teams by
the players of a team to the most, the part goes by how many whole teams
the players make */
void __attribute__((stdcall)) network_hopper_make_team_values(
	long *values,
	long team_size,
	long maximum_teams,
	long minimum_teams,
	long scale)
{
	long maximum_players= (long)((unsigned long)team_size*(unsigned long)maximum_teams);
	long minimum_players= (long)((unsigned long)team_size*(unsigned long)minimum_teams);
	long player_count;

	for (player_count= 1; player_count<=NETWORK_HOPPER_MAXIMUM_PLAYERS; player_count++)
	{
		long value;

		if (player_count<minimum_players || player_count>maximum_players)
		{
			value= NETWORK_HOPPER_PLAYERS_VALUE_NONE;
		}
		else if (player_count==maximum_players)
		{
			value= 0;
		}
		else
		{
			value= (long)((unsigned long)(maximum_teams-player_count/team_size)*(unsigned long)scale)/
				(maximum_teams-minimum_teams);
		}
		values[player_count-1]= value;
	}
}

/* 0x00193fa0: the file of the list, on the hard disk. The game has the
making of a file from a path written out here */
void __attribute__((stdcall)) network_hoppers_get_file(
	struct file_reference *file)
{
	memset(file, 0, sizeof(*file));
	file->signature= FILE_REFERENCE_SIGNATURE;
	file->location= NONE;
	file_path_add_name(file->path, "n:\\");
	if (file->flags&1)
	{
		path_remove_last_element(file->path);
	}
	file_path_add_name(file->path, network_hoppers_file_name);
	file->flags|= 1;
}

/* 0x00075790: the first of the list (if there is one) that is good and has
the number of 8 that is given; its place, and whether there is one */
boolean __attribute__((stdcall)) network_hoppers_find_unknown8(
	unsigned long unknown8,
	long *index_reference)
{
	boolean found= FALSE;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_HOPPERS && !found; index++)
	{
		const struct network_hopper *hopper= &network_hoppers[index];

		if (network_hoppers_available() && network_hopper_valid(hopper) && hopper->unknown8==unknown8)
		{
			*index_reference= index;
			found= TRUE;
		}
	}

	return found;
}
