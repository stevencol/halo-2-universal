/*
SOUND_MANAGER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How a sound is to be played: what the game says of it when it plays it (its
source), and if it says nothing, what the tag of the sound says. The name of
the file and those of the functions are guesses. Not here: 0x001251e0 and
0x001253d0 on.
*/

#include "sound_definitions.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"

/* ---------- constants */

enum
{
	/* in game_sound_globals: the listeners */
	GAME_SOUND_GLOBALS_LISTENERS_OFFSET= 0x88,

	SOUND_SOURCE_ANGLE_MAXIMUM= 0xFFFF
};

#define SOUND_PI 3.14159265358979323846f

/* ---------- globals */

/* 0x0051ebd4 */
struct sound_cache_file_gestalt *global_sound_cache_file_gestalt;

/* 0x004e6380: what the game keeps of the sound in its state. Only where the
listeners are is known */
byte *game_sound_globals;

/* ---------- code */

static inline __attribute__((always_inline)) const struct sound_definition *sound_definition_get(
	long sound_index)
{
	return (const struct sound_definition *)(uintptr_t)global_tag_instances[sound_index&0xFFFF].address;
}

static inline __attribute__((always_inline)) const struct sound_playback_parameters *sound_definition_get_playback(
	const struct sound_definition *sound)
{
	return (const struct sound_playback_parameters *)(uintptr_t)global_sound_cache_file_gestalt->playbacks.address+
		sound->playback_index;
}

/* the angle that a source says: from nothing to half a turn */
static inline __attribute__((always_inline)) real sound_source_angle(
	word angle)
{
	long value= angle;

	if (!value)
	{
		return 0.0f;
	}
	if (value>=SOUND_SOURCE_ANGLE_MAXIMUM)
	{
		return SOUND_PI;
	}

	return ((real)(SOUND_SOURCE_ANGLE_MAXIMUM-value)*0.0f + (real)value*SOUND_PI)*(1.0f/65535.0f);
}

/* 0x00124f90: something of a class of sounds: whether it has any of three
flags */
boolean sound_class_unknown_00124f90(
	const struct sound_class *sound_class)
{
	return (sound_class->flags&FLAG(_sound_class_unknown1_bit)) ||
		(sound_class->flags&FLAG(_sound_class_unknown3_bit)) ||
		(sound_class->internal_flags&FLAG(_sound_class_internal_unknown11_bit));
}

/* 0x00124fc0: the distance up to which a sound is heard at its loudest: what
the source says, or the class of the sound, or the sound */
real sound_get_minimum_distance(
	const struct sound_source *source,
	long sound_index)
{
	const struct sound_definition *sound;

	if (source->flags&FLAG(_sound_source_minimum_distance_bit))
	{
		return source->minimum_distance;
	}
	sound= sound_definition_get(sound_index);
	if (sound->flags&FLAG(_sound_class_minimum_distance_bit))
	{
		return sound_class_get(sound->sound_class)->minimum_distance;
	}

	return sound_definition_get_playback(sound)->minimum_distance;
}

/* 0x00125010: the distance from which a sound is not heard */
real sound_get_maximum_distance(
	const struct sound_source *source,
	long sound_index)
{
	const struct sound_definition *sound;

	if (source->flags&FLAG(_sound_source_maximum_distance_bit))
	{
		return source->maximum_distance;
	}
	sound= sound_definition_get(sound_index);
	if (sound->flags&FLAG(_sound_class_maximum_distance_bit))
	{
		return sound_class_get(sound->sound_class)->maximum_distance;
	}

	return sound_definition_get_playback(sound)->maximum_distance;
}

/* 0x00125060: FALSE for a sound with a flag whose source says no angle of
the cone: most likely, whether the sound has a cone */
boolean sound_unknown_00125060(
	const struct sound_source *source,
	long sound_index)
{
	if (!(source->flags&FLAG(_sound_source_inner_cone_angle_bit)) &&
		!(source->flags&FLAG(_sound_source_outer_cone_angle_bit)) &&
		sound_definition_get(sound_index)->flags&FLAG(_sound_unknown12_bit))
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x001250a0: the angle of the cone in which a sound is at its loudest */
real sound_get_inner_cone_angle(
	const struct sound_source *source,
	long sound_index)
{
	if (source->flags&FLAG(_sound_source_inner_cone_angle_bit))
	{
		return sound_source_angle(source->inner_cone_angle);
	}

	return sound_definition_get_playback(sound_definition_get(sound_index))->inner_cone_angle;
}

/* 0x00125120: the angle of the cone out of which a sound is at its lowest */
real sound_get_outer_cone_angle(
	const struct sound_source *source,
	long sound_index)
{
	if (source->flags&FLAG(_sound_source_outer_cone_angle_bit))
	{
		return sound_source_angle(source->outer_cone_angle);
	}

	return sound_definition_get_playback(sound_definition_get(sound_index))->outer_cone_angle;
}

/* 0x001251a0: how loud a sound is out of its cone */
real sound_get_outer_cone_gain(
	const struct sound_source *source,
	long sound_index)
{
	if (source->flags&FLAG(_sound_source_outer_cone_gain_bit))
	{
		return source->outer_cone_gain;
	}

	return sound_definition_get_playback(sound_definition_get(sound_index))->outer_cone_gain;
}

/* 0x00125260: how loud a sound is played this time: its gain, and something
more at random, with the second seed of the game */
real sound_definition_random_gain(
	const struct sound_definition *sound)
{
	const struct sound_playback_parameters *playback= sound_definition_get_playback(sound);
	unsigned long *seed= &global_random_seed_reference[1];
	real random;

	*seed= *seed*1664525L + 1013904223L;
	random= (real)(*seed>>16)*(1.0f/65535.0f);

	return (playback->gain_variance-0.0f)*random + 0.0f + playback->gain_base;
}

/* 0x001252f0: how high a sound is played this time: at random between its
two bounds */
real sound_definition_random_pitch(
	const struct sound_definition *sound,
	unsigned long *seed)
{
	const struct sound_playback_parameters *playback= sound_definition_get_playback(sound);

	return real_seed_random_range(seed, (real)playback->pitch_lower_bound, (real)playback->pitch_upper_bound);
}

/* 0x00125360: the listener of the first player of the console; NULL if
there is no player or if the listener is not set */
struct sound_listener *sound_first_local_listener(
	void)
{
	struct sound_listener *listener= NULL;
	long index;

	for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
	{
		if (players_globals->local_players[index]!=NONE)
		{
			listener= (struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+index;
			if (!listener->valid)
			{
				listener= NULL;
			}
			break;
		}
	}

	return listener;
}
