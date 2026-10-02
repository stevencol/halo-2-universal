/*
PLAYER_PROFILE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The profiles of the players. The names of the file and of the functions are
guesses. Not here: 0x001a03a0 and 0x001a0560.
*/

#include "player_profile.h"

#include "saved games/game_state.h"
#include "saved games/saved_game_files.h"
#include "scenario/scenario.h"
#include "tag_files/tag_groups.h"

#include <stdint.h>
#include <string.h>

/* ---------- constants */

enum
{
	/* in the globals tag: the colours of the players */
	GAME_GLOBALS_PLAYER_COLORS_OFFSET= 0x160
};

/* ---------- prototypes of functions of the C library */

/* 0x003207ed: of characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, unsigned int count);

/* ---------- prototypes of functions that are not decompiled */

/* 0x0019c1f0: of one of four numbers that a profile keeps: 0 if it is not
of something that there is */
void *unknown_0019c1f0(long unknown);

/* ---------- globals */

static const real_rgb_color player_color_white= { 1.0f, 1.0f, 1.0f };

/* 0x00468710: the colour of a player who has none */
const real_rgb_color *player_color_default= &player_color_white;

/* ---------- code */

/* 0x00153750: whether the looks of a player are some that there can be */
boolean player_appearance_valid(
	const struct player_appearance *appearance)
{
	boolean valid= TRUE;
	long index;

	for (index= 0; index<NUMBER_OF_PLAYER_APPEARANCE_COLORS; index++)
	{
		valid= valid && appearance->colors[index]>=NONE && appearance->colors[index]<NUMBER_OF_PLAYER_COLORS;
	}

	return valid && appearance->unknown4>=NONE && appearance->unknown4<NUMBER_OF_PLAYER_APPEARANCE_UNKNOWN4 &&
		appearance->emblem_foreground<NUMBER_OF_EMBLEM_FOREGROUNDS &&
		appearance->emblem_background<NUMBER_OF_EMBLEM_BACKGROUNDS &&
		!(appearance->flags&~PLAYER_APPEARANCE_FLAGS_MASK);
}

/* 0x001a0540: the profile of a file; a new one, if there is no file */
boolean player_profile_load(
	struct player_profile *profile,
	long file)
{
	if (file==NONE)
	{
		player_profile_new(profile, _player_profile_new_unknown0);

		return TRUE;
	}

	return player_profile_read(file, profile);
}

/* 0x001a0660: writes a profile to its file, and waits for it */
boolean player_profile_write(
	long file,
	struct player_profile *profile)
{
	if (file==NONE)
	{
		return FALSE;
	}

	return saved_game_file_write(file, PLAYER_PROFILE_SIZE, profile, (byte *)profile+PLAYER_PROFILE_FILE_OFFSET);
}

/* 0x001a0680: starts to write it */
boolean player_profile_write_async(
	struct player_profile *profile,
	long file,
	struct saved_game_file_result *result)
{
	if (file==NONE)
	{
		return FALSE;
	}

	return saved_game_file_write_async((byte *)profile+PLAYER_PROFILE_FILE_OFFSET, file, profile, PLAYER_PROFILE_SIZE,
		result);
}

/* 0x001a06a0: one of the colours of the players, which the globals tag has:
white, if no map is loaded or there is no such colour */
void player_color_get(
	real_rgb_color *color,
	long index)
{
	real_rgb_color value= *player_color_default;

	if (global_scenario)
	{
		const struct tag_block *colors=
			(const struct tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_PLAYER_COLORS_OFFSET);

		if (index>=0 && index<colors->count)
		{
			value= ((const real_rgb_color *)(uintptr_t)colors->address)[index];
		}
	}
	*color= value;
}

/* 0x001a06f0: the greatest of four numbers that a profile keeps, of those
that are of something that there is, and which of the four it is: NONE and 1
if none is. Of two that are the same, the last */
void player_profile_get_unknownEC(
	const struct player_profile *profile,
	long *value,
	long *index)
{
	long unknown_index;

	*value= NONE;
	*index= 1;
	for (unknown_index= 0; unknown_index<NUMBER_OF_PLAYER_PROFILE_UNKNOWNEC; unknown_index++)
	{
		long unknown= profile->unknownEC[unknown_index];

		if (unknown!=NONE && unknown_0019c1f0(unknown) && *value<=unknown)
		{
			*value= unknown;
			*index= unknown_index;
		}
	}
}

/* what a new profile has, but for its name */
static inline __attribute__((always_inline)) void player_profile_set_defaults_inline(
	struct player_profile *profile)
{
	profile->appearance.colors[0]= 10;
	profile->appearance.colors[1]= 0;
	profile->appearance.colors[2]= 0;
	profile->appearance.colors[3]= 10;
	profile->appearance.emblem_foreground= 0;
	profile->appearance.emblem_background= 0;
	profile->flags|= 1;
	profile->unknown102= 3;
	profile->unknownFC= 0;
}

/* 0x001a0750: a profile as the game gives it to a player who has none */
void player_profile_set_defaults(
	struct player_profile *profile)
{
	memset(profile, 0, sizeof(*profile));
	player_profile_set_defaults_inline(profile);
	profile->unknown100= 0;
	profile->unknown101= 0;
}

/* 0x001a07b0: a new profile, with the name of a guest */
void player_profile_new(
	struct player_profile *profile,
	long how)
{
	memset(profile, 0, sizeof(*profile));
	wcsncpy(profile->name, u"Guest", PLAYER_PROFILE_NAME_LENGTH);
	profile->name[PLAYER_PROFILE_NAME_LENGTH]= 0;
	player_profile_set_defaults_inline(profile);
	switch (how)
	{
	case _player_profile_new_unknown0:
		profile->unknown100= 0;
		profile->unknown101= 0;
		break;
	case _player_profile_new_unknown1:
		profile->unknown100= 0;
		profile->unknown101= 0;
		profile->unknownFC= 1;
		break;
	}
}

/* 0x001a0850: reads a profile from its file, and sees that what it has can
be: what cannot is set to what a new profile has. If the file cannot be
read, the profile that was given is left, seen to as well */
boolean player_profile_read(
	long file,
	struct player_profile *profile)
{
	struct player_profile read;
	boolean success= saved_game_file_read(file, &read, PLAYER_PROFILE_SIZE);

	if (success)
	{
		memcpy(profile, &read, sizeof(read));
	}
	profile->name[PLAYER_PROFILE_NAME_LENGTH]= 0;
	if (profile->unknown151>=NUMBER_OF_PLAYER_PROFILE_UNKNOWN151)
	{
		profile->unknown151= 0;
	}
	if (!player_appearance_valid(&profile->appearance))
	{
		memset(&profile->appearance, 0, sizeof(profile->appearance));
	}

	return success;
}
