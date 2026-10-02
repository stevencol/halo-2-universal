/*
SOUND_MANAGER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How a sound is to be played: what the game says of it when it plays it (its
source), and if it says nothing, what the tag of the sound says. The name of
the file and those of the functions are guesses. Not here: 0x001251e0.
*/

#include "sound_definitions.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "shell/shell.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* in game_sound_globals: the listeners, and the rest of what is known */
	GAME_SOUND_GLOBALS_SIZE= 0x20C,
	GAME_SOUND_GLOBALS_UNKNOWN60_OFFSET= 0x60,
	GAME_SOUND_GLOBALS_UNKNOWN60_BITS= 128,
	GAME_SOUND_GLOBALS_UNKNOWN70_OFFSET= 0x70,
	GAME_SOUND_GLOBALS_UNKNOWN70_BITS= 64,
	GAME_SOUND_GLOBALS_INITIALIZED_OFFSET= 0x78,
	GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET= 0x79,
	GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET= 0x7D,
	GAME_SOUND_GLOBALS_LISTENERS_OFFSET= 0x88,
	GAME_SOUND_GLOBALS_UNKNOWN1B0_OFFSET= 0x1B0,
	GAME_SOUND_GLOBALS_UNKNOWN1CC_OFFSET= 0x1CC,
	GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET= 0x208,
	GAME_SOUND_GLOBALS_MAXIMUM_VOICES_OFFSET= 0x20A,

	SOUND_SOURCE_ANGLE_MAXIMUM= 0xFFFF
};

#define SOUND_PI 3.14159265358979323846f

/* ---------- globals */

/* 0x0051ebd4 */
struct sound_cache_file_gestalt *global_sound_cache_file_gestalt;

/* 0x004e6380: what the game keeps of the sound. Only some of it is known */
byte *game_sound_globals;

/* 0x004e6374: how many times the sound was set up and not yet taken down */
long sound_initialize_count;

/* 0x004e6378 */
struct sound_channel *sound_channels;

/* 0x004e637c: the sounds that play */
struct data_array *sound_sources;

/* 0x00502110, 0x00502114, 0x0051ebe0: the arrays that other files of the
sound make when it is set up */
struct data_array *looping_sounds;
struct data_array *sound_voices;
struct data_array *sound_effects;

/* 0x0051ebd8, 0x0051ebdc: of the sound playback controllers */
void *unknown_0051ebd8;
void *unknown_0051ebdc;

/* 0x0044a0c8 */
const struct sound_configuration sound_configuration=
{
	40,
	41,
	1,
	{ 36, 36, 1 },
	51
};

/* ---------- prototypes of functions that are not decompiled */

/* 0x002185d0, 0x002186b0: set up and take down the sound of the console */
void unknown_002185d0(void);
void unknown_002186b0(void);

/* 0x00219b40, 0x0021a0c0, 0x0021d490, 0x0021dda0, 0x0021e4b0: set up the
sound playback controllers, the looping sounds, the sound effects, the sound
voices and something more. FALSE if they cannot */
boolean unknown_00219b40(void);
boolean unknown_0021a0c0(void);
boolean unknown_0021d490(void);
boolean unknown_0021dda0(const struct sound_configuration *configuration);
boolean unknown_0021e4b0(const struct sound_configuration *configuration);

/* 0x0021eae0: takes down what unknown_0021e4b0 set up, most likely */
void unknown_0021eae0(void);

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

/* 0x001253d0: sets up the sound, the first time it is asked: its memory is
of that which the game takes when it starts. If something cannot be set up the sound is
left as not set up, with the count gone up all the same */
void sound_initialize(
	void)
{
	if (!sound_initialize_count)
	{
		byte *globals= shell_memory_allocate(GAME_SOUND_GLOBALS_SIZE);
		struct data_array *sources;
		struct sound_channel *channels;

		memset(globals, 0, GAME_SOUND_GLOBALS_SIZE);
		game_sound_globals= globals;
		globals[GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET]= TRUE;
		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
		unknown_002185d0();

		{
			long *unknown1b0= (long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN1B0_OFFSET);
			long *unknown1cc= (long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN1CC_OFFSET);

			unknown1b0[0]= NONE;
			((real *)unknown1b0)[1]= 0.0f;
			((real *)unknown1b0)[2]= 2.0f*SOUND_PI;
			unknown1cc[0]= NONE;
			((real *)unknown1cc)[1]= 0.0f;
			((real *)unknown1cc)[2]= 0.0f;
		}

		sources= shell_memory_data_new("sound sources", MAXIMUM_SOUND_SOURCES, SOUND_SOURCE_DATUM_SIZE);
		channels= shell_memory_allocate(MAXIMUM_SOUND_CHANNELS*sizeof(struct sound_channel));
		sound_sources= sources;
		sound_channels= channels;
		if (sources && channels)
		{
			data_make_valid(sources);
			memset(sound_channels, 0, MAXIMUM_SOUND_CHANNELS*sizeof(struct sound_channel));
			if (unknown_00219b40() && unknown_0021a0c0() && unknown_0021d490() &&
				unknown_0021dda0(&sound_configuration) && unknown_0021e4b0(&sound_configuration))
			{
				short channel_index= 0;
				short type;

				bit_vector_fill((unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN60_OFFSET),
					GAME_SOUND_GLOBALS_UNKNOWN60_BITS, 0);
				bit_vector_fill((unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN70_OFFSET),
					GAME_SOUND_GLOBALS_UNKNOWN70_BITS, 0);
				for (type= 0; type<NUMBER_OF_SOUND_CHANNEL_TYPES; type++)
				{
					short index;

					*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET)+=
						sound_configuration.channel_counts[type];
					for (index= 0; index<sound_configuration.channel_counts[type]; index++)
					{
						struct sound_channel *channel= &sound_channels[channel_index++];

						channel->unknown0= NONE;
						channel->type= (char)type;
						channel->unknownA= 0;
						channel->unknownB= 0;
						channel->unknownC= NONE;
						channel->unknownE= NONE;
						channel->unknown4= NONE;
						channel->unknown1C= 0;
						channel->unknown20= 0;
						channel->unknown9= 0;
					}
				}
				*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_MAXIMUM_VOICES_OFFSET)=
					sound_configuration.maximum_voices;
				game_sound_globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET]= TRUE;
			}
		}
	}
	sound_initialize_count++;
}

/* 0x00125600: takes down the sound, when it is asked as many times as it
was set up: the memory is not its own, so only the pointers go */
void sound_dispose(
	void)
{
	if (!--sound_initialize_count)
	{
		unknown_002186b0();
		unknown_0021eae0();
		sound_effects= NULL;
		if (sound_voices)
		{
			sound_voices->valid= FALSE;
			sound_voices= NULL;
		}
		looping_sounds= NULL;
		unknown_0051ebd8= NULL;
		unknown_0051ebdc= NULL;
		game_sound_globals= NULL;
		if (sound_sources)
		{
			sound_sources->valid= FALSE;
			sound_sources= NULL;
		}
		sound_channels= NULL;
	}
}
