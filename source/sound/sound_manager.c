/*
SOUND_MANAGER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How a sound is to be played: what the game says of it when it plays it (its
source), and if it says nothing, what the tag of the sound says. The name of
the file and those of the functions are guesses. Not here: 0x00125b40 and
0x00125f70 on.
*/

#include "sound_definitions.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "main/main_game.h"
#include "cseries/async.h"
#include "memory/lruv_cache.h"
#include "physics/bsp3d.h"
#include "scenario/scenario.h"
#include "structures/structure_bsp_definitions.h"
#include "shell/shell.h"

#include <math.h>
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
	/* a map is loaded */
	GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET= 0x7A,
	GAME_SOUND_GLOBALS_DISABLED_OFFSET= 0x7B,
	/* while the sound is enabled or disabled */
	GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET= 0x7D,
	/* GetTickCount when a map was loaded or the sound enabled */
	GAME_SOUND_GLOBALS_START_TIME_OFFSET= 0x80,
	GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET= 0x84,
	GAME_SOUND_GLOBALS_UNKNOWN1AC_OFFSET= 0x1AC,
	GAME_SOUND_GLOBALS_UNKNOWN1C8_OFFSET= 0x1C8,
	GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET= 0x1E4,
	GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET= 0x1F8,
	GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET= 0x200,
	/* the reals that go with those two */
	GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET= 0x1FC,
	GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET= 0x204,
	GAME_SOUND_GLOBALS_LISTENERS_OFFSET= 0x88,
	GAME_SOUND_GLOBALS_UNKNOWN1B0_OFFSET= 0x1B0,
	GAME_SOUND_GLOBALS_UNKNOWN1CC_OFFSET= 0x1CC,
	GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET= 0x208,
	GAME_SOUND_GLOBALS_MAXIMUM_VOICES_OFFSET= 0x20A,

	SOUND_SOURCE_ANGLE_MAXIMUM= 0xFFFF,

	/* in the globals tag, the block of the sound: where the index of the
	tag that the sounds of a map share is */
	GAME_GLOBALS_SOUND_OFFSET= 0xC0,
	SOUND_GLOBALS_GESTALT_INDEX_OFFSET= 0x20,

	/* of a sound that plays (an element of sound_sources) */
	SOUND_SOURCE_DATUM_FLAGS_OFFSET= 0x4,
	SOUND_SOURCE_DATUM_FADING_FLAG= 0x80,
	SOUND_SOURCE_DATUM_TYPE_OFFSET= 0x1B,
	SOUND_SOURCE_DATUM_TYPE_MASK= 0xF,
	_sound_source_datum_type_positioned= 1,
	SOUND_SOURCE_DATUM_POSITION_OFFSET= 0x24,
	SOUND_SOURCE_DATUM_LOCATION_OFFSET= 0x40,
	SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET= 0xAE,
	SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET= 0xB0,
	SOUND_SOURCE_DATUM_FADE_TIME_OFFSET= 0xB4,
	SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET= 0xB8,

	/* more of a sound that plays */
	SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET= 0xC,
	SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET= 0x9C,
	SOUND_SOURCE_DATUM_PERMUTATION_OFFSET= 0x9D,
	SOUND_SOURCE_DATUM_CHUNK_OFFSET= 0x9E,

	/* in what the globals tag says of the sound: the index of the tag of
	the sound of the places with none of their own */
	SOUND_GLOBALS_ENVIRONMENT_INDEX_OFFSET= 0x14,
	GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET= 0x1EC,
	GAME_SOUND_GLOBALS_IDLING_OFFSET= 0x7C,

	/* what unknown_00218850 is asked, and what it answers */
	_sound_cache_request_unknown2= 2,
	_sound_cache_result_unknown0_bit= 0,
	_sound_cache_result_unknown1_bit= 1,

	/* of an element of unknown_00502104 */
	UNKNOWN_00502104_DATUM_SIZE= 0x10,
	UNKNOWN_00502104_DATUM_COUNT_OFFSET= 4,

	/* in milliseconds: how long the sounds take to fade when a map is let go */
	SOUND_FADE_OUT_TIME= 300,

	/* in unknown_0051ebe4 */
	UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET= 0x2A98,
	/* in unknown_0051ebe4, a voice of the console's sound for each channel */
	UNKNOWN_0051EBE4_VOICE_SIZE= 0x34,
	UNKNOWN_0051EBE4_VOICE_OFFSET= 0xC,
	/* in a voice: what it does, in its low three bits (a signed number),
	and its pitch */
	VOICE_STATE_OFFSET= 1,
	VOICE_PITCH_OFFSET= 8,
	/* in the globals of the sound: the seconds that it was given */
	GAME_SOUND_GLOBALS_SECONDS_OFFSET= 0x1E0,
	/* in the globals of the sound: set for a frame by something not decompiled,
	and how many frames it was */
	GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET= 0x7E,
	/* in the globals of the sound: the slots that there are, of 64 bits, and
	how many to look at */
	GAME_SOUND_GLOBALS_SLOTS_VALID_OFFSET= 0x58,
	GAME_SOUND_GLOBALS_SLOT_COUNT_OFFSET= 0x20A,
	/* of a voice of the console's sound */
	VOICE_FLAGS_OFFSET= 3,
	_voice_unknown3_bit= 3,
	GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET= 0x7F,

	_game_mode_main_menu= 3
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
struct data_array *unknown_0051ebd8;
void *unknown_0051ebdc;

/* 0x0051ebe4: of the sound of the console. Only two indices are known */
byte *unknown_0051ebe4;

/* 0x0047f0e0: 0x10000 in the main menu and 0x4000 in a game */
long unknown_0047f0e0;

/* 0x0047f0e4: what makes sounds of the kind 2 be told so by
sound_definition_unknown_00126bd0 */
long unknown_0047f0e4;

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

/* 0x003314b0 */
unsigned long GetTickCount(void);

/* 0x00191270: when a map is loaded */
void unknown_00191270(void);

/* 0x0021ec00, 0x001269f0, 0x001293f0, 0x0021ee80, 0x00218810: what the
sound does each time it is given some time, with sound_channels_update */
/* what sound_update does each frame, in this order, of what is not
decompiled: 0x0021f360 with the globals of the sound, 0x00221900 with the
seconds of the frame, 0x00127400, 0x00219cc0, 0x0021b0e0, 0x00128020,
0x001282d0; 0x00129100, 0x00221a70 for each local player and 0x00129380 with
what the frame has on the stack; 0x0014dde0 gives the first local player */
struct sound_update_listeners;
void __attribute__((stdcall)) unknown_0021f360(byte *globals);
void unknown_00221900(real seconds);
void __attribute__((stdcall)) unknown_00127400(boolean unknown);
void unknown_00219cc0(void);
void __attribute__((stdcall)) unknown_0021b0e0(long unknown);
void unknown_00128020(void);
void unknown_001282d0(void);
void __attribute__((stdcall)) unknown_00129100(byte *first, byte *second, struct sound_update_listeners *listeners);
long unknown_0014dde0(void);
void __attribute__((stdcall)) unknown_00221a70(long player_index, struct sound_update_listeners *listeners);
void unknown_00129380(byte *second, byte *first);

/* 0x00125b40 */
void sound_update(void);

/* 0x00126bd0 and 0x001272c0 */
boolean sound_definition_unknown_00126bd0(long sound_index);
void sound_set_unknown1f8(long index);

/* 0x0012a420, 0x0012af10, 0x0012af90 and 0x0012aff0 */
void sound_channel_set_voice_flag3(short channel_index);
short sound_slot_new(void);
void sound_slot_delete(short slot_index);
real __attribute__((stdcall)) sound_interpolation_fraction(real minimum, real maximum, real value, boolean above);

/* 0x00218de0 and 0x00218d70: the most and the least loudness of a sound,
in decibels; 0x002197f0: a gain in decibels; 0x00127010: the decibels of a
class of sounds */
real unknown_00218de0(const struct sound_definition *sound);
real unknown_00218d70(const struct sound_definition *sound);
real unknown_002197f0(real gain);
real unknown_00127010(short sound_class);

/* 0x001251e0 and 0x002195f0 */
real __attribute__((stdcall)) sound_definition_decibels(const struct sound_definition *sound, real decibels,
	real interpolation);
long double __attribute__((stdcall)) sound_decibels_to_gain(real decibels);

/* 0x0012a450, 0x00219290 and 0x0021f650 */
void sound_channel_update(short channel_index);
long sound_permutation_next_chunk(const struct sound_permutation *permutation, short chunk);
real __attribute__((stdcall)) sound_voice_playback_rate(short voice_index, long kind);

/* 0x001290d0 */
void sound_channels_update(void);

void unknown_0021ec00(void);
void unknown_001269f0(void);
void unknown_001293f0(void);
void unknown_0021ee80(void);
void unknown_00218810(void);

/* 0x0021f6d0: sets the sound of the places of the map, from three parts of
a tag */
void unknown_0021f6d0(const void *unknown20, const void *unknown18, const void *unknown28);

/* 0x00218850: asks for a piece of the samples of a sound. Bits come back */
unsigned long unknown_00218850(long sound_index, const struct sound_chunk *chunk, long unknown);

/* 0x001268e0: of a sound that plays, when its samples are not there */
void unknown_001268e0(byte *source);

/* 0x0012a810: of a sound that plays; what it gives may be a real */
long unknown_0012a810(long source_index);

/* 0x001264c0: stops the sounds, most likely. It is one of the procedures of
the state of the game, which take their argument off the stack: the tool
that builds this for the tests sees to that */
void unknown_001264c0(unsigned long flags);


/* 0x0021dde0, 0x0021a100: the sound voices and the looping sounds, after the
structure bsp changes */
void unknown_0021dde0(void);
void unknown_0021a100(void);

/* 0x0012a060, 0x0012a0b0: a channel, when the sound is disabled and when it
is enabled */
/* 0x002ae750: stops a voice of the console's sound for a while */
void unknown_002ae750(void *voice);

/* 0x00129fe0 and 0x0012a060 */
boolean sound_channel_pausable(short channel_index);
void sound_channel_pause(short channel_index);
/* 0x00218b00: how many bytes of a sound of some kind are played in a
time; 0x00218ac0: the bytes of the first chunks of a permutation */
long __attribute__((stdcall)) unknown_00218b00(long unknown4, long unknown5, long unknown3, real seconds);
long unknown_00218ac0(const struct sound_permutation *permutation, long chunk_count);

/* 0x0021f5d0: a voice of the console's sound starts again from some bytes
into its sound; 0x002ae500: a voice is given a chunk */
void unknown_0021f5d0(short voice_index, long offset);
void unknown_002ae500(void *voice, const struct sound_chunk *chunk);

/* 0x0012a0b0 */
void sound_channel_resume(short channel_index);

/* 0x00219d90, 0x0021f5a0, 0x0021f570: the sound playback controllers and the
sound of the console, when the sound is enabled and disabled */
void unknown_00219d90(boolean unknown);
void unknown_0021f5a0(void);
void unknown_0021f570(void);

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

/* two things of the globals that are set apart: a real, an index and two
more reals */
static inline __attribute__((always_inline)) void game_sound_globals_reset_unknown(
	long offset,
	real unknownC)
{
	byte *unknown= game_sound_globals+offset;

	*(real *)unknown= 0.0f;
	*(long *)(unknown+4)= NONE;
	*(real *)(unknown+8)= 0.0f;
	*(real *)(unknown+0xC)= unknownC;
}

/* 0x00125690: when a map is loaded: what its sounds share is that of the
map, and no listener is set */
void sound_initialize_for_new_map(
	void)
{
	const struct tag_block *block= (const struct tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_SOUND_OFFSET);
	const byte *sound_globals= block->count ? (const byte *)(uintptr_t)block->address : NULL;
	unsigned long gestalt_index= *(const uint32_t *)(sound_globals+SOUND_GLOBALS_GESTALT_INDEX_OFFSET);
	byte *globals;
	long index;

	global_sound_cache_file_gestalt=
		(struct sound_cache_file_gestalt *)(uintptr_t)global_tag_instances[gestalt_index&0xFFFF].address;
	*(real *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET)= 1.0f;
	game_sound_globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET]= TRUE;

	{
		unsigned long time= GetTickCount();

		globals= game_sound_globals;
		*(unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= time;
	}
	*(unsigned long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)= 0;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET)= NONE;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET)= NONE;
	game_sound_globals_reset_unknown(GAME_SOUND_GLOBALS_UNKNOWN1AC_OFFSET, 2.0f*SOUND_PI);
	game_sound_globals_reset_unknown(GAME_SOUND_GLOBALS_UNKNOWN1C8_OFFSET, 0.0f);
	for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
	{
		((struct sound_listener *)(globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET))[index].valid= FALSE;
	}

	sound_load_environment();
	unknown_00191270();
	*(long *)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET)= NONE;
	*(long *)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET+4)= NONE;
	unknown_0047f0e0= main_game_globals->options.game_mode==_game_mode_main_menu ? 0x10000 : 0x4000;
}

/* whether the sound can be used: it is set up, and a map is loaded */
static inline __attribute__((always_inline)) boolean sound_available_inline(
	void)
{
	const byte *globals= game_sound_globals;

	return globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET] && globals[GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET] &&
		globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET];
}

/* 0x001257b0: when a map is let go: the sounds that play fade out, and the
game waits for them (300 ms); then all are stopped */
void sound_dispose_from_old_map(
	void)
{
	if (!game_sound_globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET] && sound_available_inline())
	{
		unsigned long start= GetTickCount();
		struct data_array *data= sound_sources;
		long source_index= data_next_index(data, NONE);

		/* with no sound to fade there is nothing to wait for */
		if (source_index!=NONE)
		{
			real end;

			do
			{
				byte *source= (byte *)data->data+(source_index&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;

				*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= unknown_0012a810(source_index);
				*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= 0;
				*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= SOUND_FADE_OUT_TIME;
				*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
				source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
				source_index= data_next_index(data, source_index);
			} while (source_index!=NONE);

			/* the time it started is taken as a number with a sign, and the
			time it is as one with none */
			end= (real)(long)start + (real)SOUND_FADE_OUT_TIME;
			while ((double)end>(double)GetTickCount())
			{
				sound_idle();
			}
		}
	}

	unknown_001264c0(0);
	game_sound_globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET]= FALSE;
	if (looping_sounds)
	{
		data_delete_all(looping_sounds);
	}
	if (unknown_0051ebd8)
	{
		data_delete_all(unknown_0051ebd8);
	}
	global_sound_cache_file_gestalt= NULL;
	unknown_00191270();
	*(long *)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET)= NONE;
	*(long *)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET+4)= NONE;
}

/* 0x00125920: when the structure bsp changes: where in it are the sounds
that have a place, and the listeners */
void sound_initialize_for_new_structure_bsp(
	void)
{
	if (sound_available_inline())
	{
		struct data_array *data= sound_sources;
		const struct structure_bsp *structure_bsp;
		long source_index;
		long index;

		for (source_index= data_next_index(data, NONE);
			source_index!=NONE;
			source_index= data_next_index(data, source_index))
		{
			byte *source= (byte *)data->data+(source_index&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;

			if ((source[SOUND_SOURCE_DATUM_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)==_sound_source_datum_type_positioned)
			{
				struct scenario_location location;

				scenario_location_from_point(&location, (const real_point3d *)(source+SOUND_SOURCE_DATUM_POSITION_OFFSET));
				memcpy(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET, &location, sizeof(location));
			}
		}

		structure_bsp= global_structure_bsp;
		for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
		{
			struct sound_listener *listener=
				(struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+index;

			if (listener->valid)
			{
				long leaf_index= bsp3d_test_point(global_bsp3d, 0, &listener->position);

				listener->leaf_index= leaf_index;
				listener->cluster_index= leaf_index!=NONE ?
					*(const short *)((const byte *)(uintptr_t)structure_bsp->leaves.address+leaf_index*8) : NONE;
			}
		}
		unknown_0021dde0();
		unknown_0021a100();
	}
}

/* 0x00125a60 */
boolean sound_available(
	void)
{
	return sound_available_inline();
}

/* 0x00125a90: stops the sound or lets it go on: 0 stops it and 1 lets it
go on. Nothing is done if it is so already; any other number counts as one
that lets it go on, but nothing is told */
void sound_enable(
	long enable)
{
	byte *globals= game_sound_globals;
	boolean disabled= !enable;

	if (disabled!=globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET])
	{
		long index;

		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= TRUE;
		globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET]= disabled;
		if (enable==0)
		{
			for (index= 0; index<*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET); index++)
			{
				sound_channel_pause((short)index);
			}
			unknown_0021f570();
			globals= game_sound_globals;
		}
		else if (enable==1)
		{
			unsigned long time;

			for (index= 0; index<*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET); index++)
			{
				sound_channel_resume((short)index);
			}
			unknown_00219d90(TRUE);
			unknown_0021f5a0();
			time= GetTickCount();
			globals= game_sound_globals;
			*(unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= time;
		}
		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
	}
}

/* 0x00125d60: gives the sound some time, while the game waits for something
else: only in a game, and if more than a tick has gone by since the sound
started. It is not done while it is being done */
void sound_idle(
	void)
{
	unsigned long start= *(unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET);
	long elapsed= (long)(GetTickCount()-start);

	if (main_game_globals && main_game_globals->active &&
		(real)elapsed>game_time_globals->seconds_per_tick*1000.0f)
	{
		byte *globals= game_sound_globals;

		globals[GAME_SOUND_GLOBALS_IDLING_OFFSET]= TRUE;
		if (!globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET])
		{
			globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= TRUE;
			if (sound_available())
			{
				unknown_0021ec00();
				unknown_001269f0();
				sound_channels_update();
				unknown_001293f0();
				unknown_0021ee80();
				globals= game_sound_globals;
			}
			unknown_00218810();
			globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
		}
		globals[GAME_SOUND_GLOBALS_IDLING_OFFSET]= FALSE;
	}
}

/* 0x00125df0: when a map is loaded: the sound of the places with none of
their own, if the globals tag tells of one */
void sound_load_environment(
	void)
{
	const struct tag_block *block= (const struct tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_SOUND_OFFSET);
	const byte *sound_globals= block->count ? (const byte *)(uintptr_t)block->address : NULL;
	long environment_index= *(const int32_t *)(sound_globals+SOUND_GLOBALS_ENVIRONMENT_INDEX_OFFSET);

	if (environment_index!=NONE)
	{
		const byte *environment= (const byte *)(uintptr_t)global_tag_instances[environment_index&0xFFFF].address;

		unknown_0021f6d0(environment+0x20, environment+0x18, environment+0x28);
		memcpy(game_sound_globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET, environment+0x48, 3*sizeof(long));
	}
}

/* the piece of the samples that a recording of a sound starts with */
static inline __attribute__((always_inline)) const struct sound_chunk *sound_get_chunk(
	long pitch_range_index,
	long permutation_offset,
	long chunk_offset)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const struct sound_pitch_range *pitch_range=
		(const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address+pitch_range_index;
	const struct sound_permutation *permutation=
		(const struct sound_permutation *)(uintptr_t)gestalt->permutations.address+
		(pitch_range->first_permutation_index+permutation_offset);

	return (const struct sound_chunk *)(uintptr_t)gestalt->chunks.address+
		(permutation->first_chunk_index+chunk_offset);
}

/* 0x00125e60: asks for the samples that a sound that plays is at. Whether
the second bit of the answer is set */
boolean sound_source_request_data(
	byte *source)
{
	long sound_index= *(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET);
	const struct sound_definition *sound= sound_definition_get(sound_index);
	unsigned long result= unknown_00218850(sound_index,
		sound_get_chunk(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET],
			(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET],
			*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)),
		_sound_cache_request_unknown2);

	if (result&(FLAG(_sound_cache_result_unknown0_bit)|FLAG(_sound_cache_result_unknown1_bit)))
	{
		unknown_001268e0(source);
	}

	return (result>>_sound_cache_result_unknown1_bit)&1;
}

/* 0x00125ef0: one less of something that the sound of the console counts:
which, is 8 bytes into what the function is given */
void sound_unknown_00125ef0(
	const byte *unknown)
{
	long index= *(const long *)(unknown+8);
	byte *datum= (byte *)unknown_00502104->data+(index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;

	datum[UNKNOWN_00502104_DATUM_COUNT_OFFSET]--;
}

/* 0x00125f10: asks for the first samples of a sound, before it is played */
void sound_predict(
	long sound_index)
{
	if (sound_index!=NONE)
	{
		const struct sound_definition *sound= sound_definition_get(sound_index);

		if (sound->pitch_range_count)
		{
			const struct sound_pitch_range *pitch_range=
				(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
				sound->first_pitch_range_index;

			if (pitch_range->permutation_count)
			{
				unknown_00218850(sound_index, sound_get_chunk(sound->first_pitch_range_index, 0, 0),
					_sound_cache_request_unknown2);
			}
		}
	}
}

/* 0x001290d0: each sound channel is updated, by 0x0012a450; how many there
are is looked at again after each. The name is a guess */
void sound_channels_update(
	void)
{
	short channel_index;

	for (channel_index= 0;
		channel_index<*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		sound_channel_update(channel_index);
	}
}

/* 0x00129fe0: whether the sound of a channel stops when the game is paused:
not if its class says so. A channel with no sound, or with a sound of a
class that there is not, can be. The name is a guess */
boolean sound_channel_pausable(
	short channel_index)
{
	long source_index= sound_channels[channel_index].unknown0;
	const byte *source;
	char sound_class;

	if (source_index==NONE)
	{
		return TRUE;
	}
	source= (const byte *)sound_sources->data+(source_index&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;
	sound_class= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class;
	if (sound_class<0 || sound_class>=NUMBER_OF_SOUND_CLASSES)
	{
		return TRUE;
	}

	return !TEST_FLAG(sound_class_get(sound_class)->internal_flags, _sound_class_internal_not_pausable_bit);
}

/* 0x0012a060: the sound of a channel stops for a while, if it plays, is not
stopped and can be. The name is a guess */
void sound_channel_pause(
	short channel_index)
{
	struct sound_channel *channel= &sound_channels[channel_index];

	if (channel->unknownC!=NONE && !channel->unknown9 && sound_channel_pausable(channel_index))
	{
		unknown_002ae750(unknown_0051ebe4+channel->unknownC*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET);
		channel->unknown9= TRUE;
	}
}

/* the voice of a channel is given the chunk of a permutation that plays */
static inline __attribute__((always_inline)) void sound_channel_voice_set_chunk(
	const struct sound_channel *channel,
	const struct sound_permutation *permutation,
	short chunk)
{
	const struct sound_chunk *chunks=
		(const struct sound_chunk *)(uintptr_t)global_sound_cache_file_gestalt->chunks.address;

	unknown_002ae500(unknown_0051ebe4+channel->unknownC*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET,
		&chunks[permutation->first_chunk_index+chunk]);
}

/* 0x0012a0b0: the sound of a channel that sound_channel_pause stopped goes
on: its voice starts again where the sound was, in the chunks that play.
The name is a guess */
void sound_channel_resume(
	short channel_index)
{
	struct sound_channel *channel= &sound_channels[channel_index];

	if (channel->unknownC==NONE || !channel->unknown9)
	{
		return;
	}
	if (channel->unknown1C)
	{
		const byte *source=
			(const byte *)sound_sources->data+(channel->unknown0&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;
		const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
		const struct sound_permutation *permutation= (const struct sound_permutation *)(uintptr_t)channel->unknown1C;
		long offset= unknown_00218b00((char)sound->unknown3[1], (char)sound->unknown3[2], (char)sound->unknown3[0],
			channel->unknown10);

		offset-= unknown_00218ac0(permutation, channel->unknown18);
		unknown_0021f5d0(channel->unknownC, offset);
		sound_channel_voice_set_chunk(channel, permutation, channel->unknown18);
		if (channel->unknown20)
		{
			sound_channel_voice_set_chunk(channel, (const struct sound_permutation *)(uintptr_t)channel->unknown20,
				channel->unknown1A);
		}
	}
	channel->unknown9= FALSE;
}

/* the voice of the console's sound of a channel */
static inline __attribute__((always_inline)) byte *sound_channel_voice(
	const struct sound_channel *channel)
{
	return unknown_0051ebe4+channel->unknownC*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;
}

/* what a voice does: a signed number of three bits */
static inline __attribute__((always_inline)) short sound_voice_state(
	const byte *voice)
{
	return (short)((signed char)(voice[VOICE_STATE_OFFSET]<<5)>>5);
}

static inline __attribute__((always_inline)) const struct sound_chunk *sound_permutation_chunk(
	const struct sound_permutation *permutation,
	short chunk)
{
	return (const struct sound_chunk *)(uintptr_t)global_sound_cache_file_gestalt->chunks.address+
		permutation->first_chunk_index+chunk;
}

/* 0x00219290: the chunk of a permutation after one; NONE after the last, or
with no permutation. The name is a guess */
long sound_permutation_next_chunk(
	const struct sound_permutation *permutation,
	short chunk)
{
	if (permutation && chunk<permutation->chunk_count-1)
	{
		return chunk+1;
	}

	return NONE;
}

/* 0x0021f650: how fast a voice of the console's sound plays, from its pitch
and a kind of sound: 2 to the power of their sum over 4096. The name is a
guess */
real __attribute__((stdcall)) sound_voice_playback_rate(
	short voice_index,
	long kind)
{
	const byte *voice= unknown_0051ebe4+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;
	long pitch= *(const long *)(voice+VOICE_PITCH_OFFSET);
	long double exponent;

	switch (kind)
	{
	case 0:
		pitch+= 0x11F5;
		break;
	case 1:
		pitch+= 0x1F5;
		break;
	default:
		pitch+= 0x95C;
		break;
	}
	exponent= (long double)pitch*(long double)(1.0f/4096.0f)*(long double)0.6931471824645996f;

	return (real)exp2l(exponent*1.4426950408889634073599246810018921L);
}

/* 0x0012a450: what is done with a channel that plays each time the sound
is given some time: when its voice is done with a chunk, the next one goes
first and the one that was is let go; and the time of the sound goes on as
fast as the voice plays. The name is a guess */
void sound_channel_update(
	short channel_index)
{
	struct sound_channel *channel= &sound_channels[channel_index];
	const struct sound_permutation *permutation;
	short state;

	if (channel->unknownC==NONE || channel->unknown9 || !channel->unknown1C)
	{
		return;
	}
	permutation= (const struct sound_permutation *)(uintptr_t)channel->unknown1C;
	state= sound_voice_state(sound_channel_voice(channel));
	if (channel->unknown20 && state<3)
	{
		sound_unknown_00125ef0((const byte *)sound_permutation_chunk(permutation, channel->unknown18));
		if ((short)sound_permutation_next_chunk(permutation, channel->unknown18)==NONE)
		{
			channel->unknown10= 0.0f;
		}
		channel->unknown1C= channel->unknown20;
		channel->unknown18= channel->unknown1A;
		channel->unknown20= 0;
		channel->unknown1A= NONE;
		if (!((unknown_00218850(NONE,
			sound_permutation_chunk((const struct sound_permutation *)(uintptr_t)channel->unknown1C, channel->unknown18), 0)>>
			_sound_cache_result_unknown1_bit)&1))
		{
			state= 0;
		}
	}
	if (channel->unknown1C && state<2)
	{
		sound_unknown_00125ef0((const byte *)sound_permutation_chunk(
			(const struct sound_permutation *)(uintptr_t)channel->unknown1C, channel->unknown18));
		channel->unknown1C= 0;
		channel->unknown18= NONE;
	}
	if ((signed char)(sound_channel_voice(channel)[VOICE_STATE_OFFSET]<<5)>0)
	{
		const byte *source=
			(const byte *)sound_sources->data+(channel->unknown0&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;
		const struct sound_definition *sound=
			sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));

		channel->unknown10= sound_voice_playback_rate(channel->unknownC, (char)sound->unknown3[0])*
			*(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET) + channel->unknown10;
	}
}

/* 0x002195f0: a loudness in decibels as a gain: 10 to the power of a
twentieth of them, from -64 to 0, on the x87 and not rounded. The name is a
guess */
long double __attribute__((stdcall)) sound_decibels_to_gain(
	real decibels)
{
	real clamped;

	if (-64.0f>decibels)
	{
		clamped= -64.0f;
	}
	else if (decibels>0.0f)
	{
		clamped= 0.0f;
	}
	else
	{
		clamped= decibels;
	}
	/* the game works it out as it runs, with the precision of the x87: the
	compiler is not to do it before for the ends, with more */
	__asm__("" : "+x"(clamped));

	return exp2l((long double)clamped*(long double)0.05f*(long double)2.3025851249694824f*
		1.4426950408889634073599246810018921L);
}

/* 0x001251e0: how loud a sound is played, in decibels: between its least
and its most loudness, as gains, by an interpolation, with those of its
class and some more. The name is a guess */
real __attribute__((stdcall)) sound_definition_decibels(
	const struct sound_definition *sound,
	real decibels,
	real interpolation)
{
	real most= unknown_00218de0(sound);
	real least_gain= (real)sound_decibels_to_gain(unknown_00218d70(sound));
	long double most_gain= sound_decibels_to_gain(most);
	real gain= (real)((most_gain-(long double)least_gain)*(long double)interpolation+(long double)least_gain);
	real gain_decibels= unknown_002197f0(gain);

	return gain_decibels + (unknown_00127010(sound->sound_class) + decibels);
}

/* what sound_update has on the stack for the listeners: how many, and more
that is not known */
struct sound_update_listeners
{
	long count;
	byte unknown4[0x1434];
};

/* 0x0050210c: the cache of the sound's samples */
struct lruv_cache *sound_lruv_cache;

/* the local player after one, or NONE after the last */
static inline __attribute__((always_inline)) long sound_next_local_player(
	long player_index)
{
	long index= player_index==NONE ? 0 : player_index+1;

	for (; index<MAXIMUM_LOCAL_PLAYERS; index++)
	{
		if (players_globals->local_players[index]!=NONE)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x00125b40: what the sound does each frame: it goes on if it was off and
the game is not paused, its parts are updated, with the listeners of each
local player, and a tick of its cache goes by. The name is a guess */
void sound_update(
	void)
{
	byte *globals= game_sound_globals;

	if (globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET] && globals[GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET] &&
		globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET])
	{
		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= TRUE;
		unknown_0021ec00();
		globals= game_sound_globals;
		if (globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET])
		{
			byte first[0x100];
			byte second[0x200];
			struct sound_update_listeners listeners;
			long player_index;

			if (globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET] &&
				(!game_time_globals->initialized || !game_time_globals->paused))
			{
				long index;
				unsigned long time;

				globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= TRUE;
				globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET]= FALSE;
				for (index= 0; index<*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET); index++)
				{
					sound_channel_resume((short)index);
				}
				unknown_00219d90(TRUE);
				unknown_0021f5a0();
				time= GetTickCount();
				globals= game_sound_globals;
				*(unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= time;
				globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
			}
			unknown_0021f360(globals);
			if (!globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET])
			{
				unknown_001269f0();
				globals= game_sound_globals;
			}
			unknown_00221900(*(real *)(globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET));
			unknown_00127400(TRUE);
			unknown_00219cc0();
			unknown_0021b0e0(game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET]);
			sound_channels_update();
			unknown_00128020();
			unknown_001282d0();
			memset(first, 0, sizeof(first));
			memset(second, 0, sizeof(second));
			listeners.count= 0;
			unknown_00129100(first, second, &listeners);
			for (player_index= unknown_0014dde0(); player_index!=NONE;
				player_index= sound_next_local_player(player_index))
			{
				unknown_00221a70(player_index, &listeners);
			}
			unknown_00129380(second, first);
			unknown_001293f0();
			if (game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET])
			{
				game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET]++;
			}
		}
		unknown_0021ee80();
		unknown_00219d90(FALSE);
		game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET]= FALSE;
		game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
	}
	lruv_idle(sound_lruv_cache);
}

/* 0x0012a420: sets a flag of the voice of a channel (the fourth bit of its
fourth byte), which giving it a chunk clears. The name is a guess */
void sound_channel_set_voice_flag3(
	short channel_index)
{
	byte *voice= sound_channel_voice(&sound_channels[channel_index]);

	voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown3_bit);
}

/* 0x0012af10: takes the first slot of the globals of the sound that there
is and is not taken; NONE if there is none. The name is a guess */
short sound_slot_new(
	void)
{
	byte *globals= game_sound_globals;
	short count= *(short *)(globals+GAME_SOUND_GLOBALS_SLOT_COUNT_OFFSET);
	short slot_index;

	for (slot_index= 0; slot_index<count; slot_index++)
	{
		const unsigned long *valid= (const unsigned long *)(globals+GAME_SOUND_GLOBALS_SLOTS_VALID_OFFSET);
		unsigned long *taken= (unsigned long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN70_OFFSET);

		if ((valid[slot_index>>5]&(1UL<<(slot_index&31))) && !(taken[slot_index>>5]&(1UL<<(slot_index&31))))
		{
			taken[slot_index>>5]|= 1UL<<(slot_index&31);
			return slot_index;
		}
	}

	return NONE;
}

/* 0x0012af90: lets a slot of the globals of the sound go, if it is not
NONE. The name is a guess */
void sound_slot_delete(
	short slot_index)
{
	if (slot_index!=NONE)
	{
		unsigned long *taken= (unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN70_OFFSET);

		taken[slot_index>>5]&= ~(1UL<<(slot_index&31));
	}
}

/* 0x0012aff0: where a value is from a minimum to a maximum, from 0 to 1;
with the two closer than 0.0001, 0 or 1 by whether the minimum is over the
value as it is asked. The name is a guess */
real __attribute__((stdcall)) sound_interpolation_fraction(
	real minimum,
	real maximum,
	real value,
	boolean above)
{
	real fraction;

	if (fabsf(minimum-maximum)<0.0001f)
	{
		return (minimum>value)==above ? 0.0f : 1.0f;
	}
	fraction= (value-minimum)/(maximum-minimum);
	if (0.0f>fraction)
	{
		return 0.0f;
	}

	return fraction>1.0f ? 1.0f : fraction;
}

/* 0x00126bd0: whether a sound is of some kind (its fifth and sixth bytes
say, and unknown_0047f0e4 for the kind 2) and the first of its pitch
ranges has permutations. What it means is not known */
boolean sound_definition_unknown_00126bd0(
	long sound_index)
{
	const struct sound_definition *sound= sound_definition_get(sound_index);
	const struct sound_pitch_range *pitch_ranges;

	if (!(sound->unknown3[2]==1 && sound->unknown3[1]!=2))
	{
		if (sound->unknown3[1]!=2 || !unknown_0047f0e4)
		{
			return FALSE;
		}
	}
	if (!sound->pitch_range_count)
	{
		return FALSE;
	}
	pitch_ranges= (const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address;

	return pitch_ranges[sound->first_pitch_range_index].permutation_count!=0;
}

/* 0x001272c0: the index at 0x1F8 of the globals of the sound becomes
another; the one that it was is kept at 0x200, and going back to it gives
back its real at 0x204. The name is a guess */
void sound_set_unknown1f8(
	long index)
{
	byte *globals= game_sound_globals;
	long current= *(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET);

	if (index==current)
	{
		return;
	}
	if (index!=NONE && index==*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET))
	{
		*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET)= index;
		*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)= *(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET);
		*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET)= 0.0f;
		return;
	}
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET)= current;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET)= index;
	*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)= 0.0f;
	*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET)= 0.0f;
}
