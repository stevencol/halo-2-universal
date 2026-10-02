/*
SOUND_DEFINITIONS.H

The tags of the sounds, as a cache file holds them, and what the game tells
of a sound that plays. Only what the code uses: the names of the fields are
guesses.
*/

#ifndef __SOUND_DEFINITIONS_H
#define __SOUND_DEFINITIONS_H

#include <stdint.h>

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

/* the flags of a sound */
enum
{
	/* the distances are those of the class of the sound */
	_sound_class_minimum_distance_bit= 10,
	_sound_class_maximum_distance_bit= 11,
	_sound_unknown12_bit= 12
};

/* the flags of a sound source: what it says in place of the sound */
enum
{
	_sound_source_minimum_distance_bit= 2,
	_sound_source_maximum_distance_bit= 3,
	_sound_source_inner_cone_angle_bit= 4,
	_sound_source_outer_cone_angle_bit= 5,
	_sound_source_outer_cone_gain_bit= 6
};

/* the flags of a sound class */
enum
{
	_sound_class_unknown1_bit= 1,
	_sound_class_unknown3_bit= 3,
	/* of the second flags */
	_sound_class_internal_unknown11_bit= 11
};

/* ---------- structures */

/* the tag of a sound */
struct sound_definition
{
	word flags;
	char sound_class;
	byte unknown3[3];
	/* which of the playback parameters of the sounds are its own */
	short playback_index;
};

/* how a sound is played */
struct sound_playback_parameters
{
	real minimum_distance;
	real maximum_distance;
	byte unknown8[8];
	real gain_base;
	real gain_variance;
	short pitch_lower_bound;
	short pitch_upper_bound;
	real inner_cone_angle;
	real outer_cone_angle;
	real outer_cone_gain;
	byte unknown28[0x10];
};

/* 0x0051ebd4 points to it: what all the sounds of a map share */
struct sound_cache_file_gestalt
{
	struct tag_block playbacks; /* sound_playback_parameters */
	/* the rest of the tag */
};

/* a class of sounds */
struct sound_class
{
	byte unknown0[8];
	word flags;
	word internal_flags;
	byte unknownC[0xC];
	real minimum_distance;
	real maximum_distance;
	byte unknown20[0x3C];
};

/* the tag of the classes of sounds */
struct sound_classes
{
	struct tag_block classes; /* sound_class */
};

/* what the game says of a sound that it plays. An angle is a word: the part
of half a turn that it is */
struct sound_source
{
	word flags;
	byte unknown2[0x32];
	real minimum_distance;
	real maximum_distance;
	word inner_cone_angle;
	word outer_cone_angle;
	real outer_cone_gain;
	/* the rest is not known */
};

/* what hears the sounds: one for each player of the console */
struct sound_listener
{
	byte unknown0[6];
	boolean valid;
	byte unknown7[0x41];
};

typedef char verify_sound_playback_parameters_size[
	sizeof(struct sound_playback_parameters) == 0x38 ? 1 : -1];
typedef char verify_sound_class_size[
	sizeof(struct sound_class) == 0x5C ? 1 : -1];
typedef char verify_sound_source_outer_cone_gain_offset[
	offsetof(struct sound_source, outer_cone_gain) == 0x40 ? 1 : -1];
typedef char verify_sound_listener_size[
	sizeof(struct sound_listener) == 0x48 ? 1 : -1];

/* ---------- prototypes/SOUND_CLASSES.C */

struct sound_class *sound_class_get(short class_index);

/* ---------- prototypes/SOUND_MANAGER.C */

boolean sound_class_unknown_00124f90(const struct sound_class *sound_class);
real sound_get_minimum_distance(const struct sound_source *source, long sound_index);
real sound_get_maximum_distance(const struct sound_source *source, long sound_index);
boolean sound_unknown_00125060(const struct sound_source *source, long sound_index);
real sound_get_inner_cone_angle(const struct sound_source *source, long sound_index);
real sound_get_outer_cone_angle(const struct sound_source *source, long sound_index);
real sound_get_outer_cone_gain(const struct sound_source *source, long sound_index);
real sound_definition_random_gain(const struct sound_definition *sound);
real sound_definition_random_pitch(const struct sound_definition *sound, unsigned long *seed);
struct sound_listener *sound_first_local_listener(void);

/* ---------- globals */

extern struct sound_cache_file_gestalt *global_sound_cache_file_gestalt;
extern byte *game_sound_globals;

#endif
