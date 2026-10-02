/*
PLAYER_PROFILE.H

What a player keeps of how he plays: his name, his controls, his looks. Only
what the code uses; the names are guesses.
*/

#ifndef __PLAYER_PROFILE_H
#define __PLAYER_PROFILE_H

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	PLAYER_PROFILE_SIZE= 0x1E0,
	PLAYER_PROFILE_NAME_LENGTH= 31,
	/* what is written of a profile starts here: at its name */
	PLAYER_PROFILE_FILE_OFFSET= 8,

	NUMBER_OF_PLAYER_COLORS= 18,
	NUMBER_OF_PLAYER_APPEARANCE_COLORS= 4,
	NUMBER_OF_PLAYER_APPEARANCE_UNKNOWN4= 4,
	NUMBER_OF_EMBLEM_FOREGROUNDS= 64,
	NUMBER_OF_EMBLEM_BACKGROUNDS= 32,
	PLAYER_APPEARANCE_FLAGS_MASK= 0x0F,

	NUMBER_OF_PLAYER_PROFILE_UNKNOWNEC= 4,
	NUMBER_OF_PLAYER_PROFILE_UNKNOWN151= 3
};

/* how a new profile is made */
enum
{
	_player_profile_new_unknown0,
	_player_profile_new_unknown1
};

/* ---------- structures */

/* how a player looks: his colours and his emblem. NONE is no colour */
struct player_appearance
{
	char colors[NUMBER_OF_PLAYER_APPEARANCE_COLORS];
	char unknown4;
	byte emblem_foreground;
	byte emblem_background;
	byte flags;
	byte unknown8[8];
};

struct player_profile
{
	byte unknown0[4];
	unsigned long flags;
	wchar name[PLAYER_PROFILE_NAME_LENGTH+1];
	byte unknown48[0xA4];
	long unknownEC[NUMBER_OF_PLAYER_PROFILE_UNKNOWNEC];
	long unknownFC;
	byte unknown100;
	byte unknown101;
	byte unknown102;
	byte unknown103[0x15];
	struct player_appearance appearance;
	byte unknown128[0x29];
	byte unknown151;
	byte unknown152[0x8E];
};

struct saved_game_file_result;

typedef char verify_player_profile_size[
	sizeof(struct player_profile) == PLAYER_PROFILE_SIZE ? 1 : -1];
typedef char verify_player_profile_appearance_offset[
	offsetof(struct player_profile, appearance) == 0x118 ? 1 : -1];
typedef char verify_player_profile_unknown151_offset[
	offsetof(struct player_profile, unknown151) == 0x151 ? 1 : -1];

/* ---------- prototypes/PLAYER_PROFILE.C */

boolean player_appearance_valid(const struct player_appearance *appearance);
boolean player_profile_load(struct player_profile *profile, long file);
boolean player_profile_write(long file, struct player_profile *profile);
boolean player_profile_write_async(struct player_profile *profile, long file, struct saved_game_file_result *result);
void player_color_get(real_rgb_color *color, long index);
void player_profile_get_unknownEC(const struct player_profile *profile, long *value, long *index);
void player_profile_set_defaults(struct player_profile *profile);
void player_profile_new(struct player_profile *profile, long how);
boolean player_profile_read(long file, struct player_profile *profile);

/* ---------- globals */

extern const real_rgb_color *player_color_default;

#endif
