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

	/* more of a sound that plays: set while it is being let go, who is told
	of what happens to it and what it is told with, its playback controller
	and its channel */
	SOUND_SOURCE_DATUM_UNKNOWN2_OFFSET= 0x2,
	SOUND_SOURCE_DATUM_UNKNOWN3_OFFSET= 0x3,
	SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET= 0x10,
	SOUND_SOURCE_DATUM_CALLBACKS_OFFSET= 0x14,
	SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET= 0xA8,
	SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET= 0xAC,
	/* of an element of unknown_0051ebd8 */
	UNKNOWN_0051EBD8_DATUM_SIZE= 0x1C,
	UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET= 0x3,
	UNKNOWN_0051EBD8_DATUM_COUNT_MASK= 0x7F,
	UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET= 0x4,
	/* of a sound that plays: it counts in the piece of the samples that it
	is at, it is not to be told of again, and something more */
	_sound_source_datum_unknown0_bit= 0,
	_sound_source_datum_holds_chunk_bit= 1,
	_sound_source_datum_unknown5_bit= 5,
	SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET= 0x8C,
	/* of an element of unknown_0051ebd8: how many sounds it has, in 7 bits */
	UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET= 0x2,
	/* in what the game says of a sound: where its place comes from, and
	its place */
	SOUND_SOURCE_POSITION_KIND_OFFSET= 0x2,
	SOUND_SOURCE_POSITION_OFFSET= 0xC,
	_sound_source_position_world= 0,
	_sound_source_position_listener_height= 1,
	/* of a sound that plays: its kind, and two parts of it that who is told
	of it is given */
	SOUND_SOURCE_DATUM_KIND_OFFSET= 0x2,
	SOUND_SOURCE_DATUM_KINDS_UNKNOWN= 0x1E,
	SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET= 0x18,
	SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET= 0x5C,
	/* of a sound voice: its slot of the globals of the sound, and how many
	use it */
	SOUND_VOICE_DATUM_SIZE= 0x6C,
	SOUND_VOICE_DATUM_SLOT_OFFSET= 0x2,
	SOUND_VOICE_DATUM_USERS_OFFSET= 0xA,
	/* two classes of sounds, and what a class says of its sounds when the
	sounds are stopped */
	_sound_class_unknown32= 0x20,
	_sound_class_unknown38= 0x26,
	SOUND_CLASS_UNKNOWN4C_OFFSET= 0x4C,
	/* the flags of sound_reset */
	_sound_reset_keep_bit= 5,
	_sound_reset_unknown6_bit= 6,
	UNKNOWN_004ED288_UNKNOWN20_OFFSET= 0x20,
	/* a piece of the samples of a recording: how many bytes it is */
	SOUND_CHUNK_SIZE_OFFSET= 0x4,
	SOUND_CHUNK_SIZE_MASK= 0x3FFFFFFF,
	/* in the tag of a sound: which promotion and which playback of its own
	it has, of those that the sounds of a map share */
	SOUND_DEFINITION_PROMOTION_INDEX_OFFSET= 0xC,
	SOUND_DEFINITION_CUSTOM_PLAYBACK_INDEX_OFFSET= 0xD,
	SOUND_GESTALT_PROMOTIONS_OFFSET= 0x48,
	SOUND_PROMOTION_SIZE= 0x1C,
	SOUND_PROMOTION_RULE_SIZE= 0x10,
	SOUND_CUSTOM_PLAYBACK_SIZE= 0x34,
	/* the arrays of other files of the sound */
	MAXIMUM_LOOPING_SOUNDS= 128,
	LOOPING_SOUND_DATUM_SIZE= 0xD4,
	MAXIMUM_SOUND_EFFECTS= 16,
	SOUND_EFFECT_DATUM_SIZE= 0x14,
	SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET= 0x8,
	/* in unknown_0051ebe4: two streams of the console's sound */
	UNKNOWN_0051EBE4_STREAMS_OFFSET= 0x2A90,
	/* a piece of the samples of a recording: which element of
	unknown_00502104 it is, and of an element, how many times it was asked
	for */
	SOUND_CHUNK_CACHE_INDEX_OFFSET= 0x8,
	UNKNOWN_00502104_DATUM_USES_OFFSET= 0x5,
	SOUND_PERMUTATION_UNKNOWN6_OFFSET= 0x6,
	/* a flag of the tag of a sound: how far it is heard is of its class */
	_sound_definition_class_distance_bit= 11,
	_sound_class_internal_unknown5_bit= 5,
	SOUND_CLASS_UNKNOWN10_OFFSET= 0x10,
	SOUND_CLASS_UNKNOWN50_OFFSET= 0x50,
	LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET= 0x54,
	/* of a sound voice: the time of the sound when it was given the 0x24
	bytes after it */
	SOUND_VOICE_DATUM_START_TIME_OFFSET= 0x44,
	SOUND_VOICE_DATUM_UNKNOWN48_OFFSET= 0x48,
	SOUND_VOICE_DATUM_UNKNOWN48_SIZE= 0x24,
	SOUND_VOICE_DATUM_UNKNOWN_SOURCE_OFFSET= 0x24,
	UNKNOWN_00218420_BYTES_OFFSET= 0x8,
	UNKNOWN_00218420_BYTES_SIZE= 0x14,
	/* in a class of sounds, and in the scale of a sound (one of those that
	the sounds of a map share): the least and the most decibels */
	SOUND_CLASS_DECIBELS_OFFSET= 0x20,
	SOUND_DEFINITION_SCALE_INDEX_OFFSET= 0xB,
	SOUND_SCALE_SIZE= 0x14,
	/* in a pitch range: which of the bits that the sounds of a map share
	are of its recordings */
	SOUND_PITCH_RANGE_PLAYED_BITS_OFFSET= 0x6,
	/* more of an element of unknown_0051ebd8 */
	UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET= 0x8,
	UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG= 0x80,
	UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET= 0xC,
	UNKNOWN_0051EBD8_DATUM_UNKNOWNC_SIZE= 0x10,
	/* in what the game says of a sound: its type, as in a sound that plays */
	SOUND_SOURCE_TYPE_OFFSET= 0x3,
	/* of a sound effect: its kind, its flags and its sound */
	SOUND_EFFECT_DATUM_KIND_OFFSET= 0x3,
	SOUND_EFFECT_DATUM_FLAGS_OFFSET= 0x4,
	_sound_effect_datum_unknown0_bit= 0,
	_sound_effect_datum_unknown1_bit= 1,
	_sound_effect_datum_unknown3_bit= 3,
	_sound_effect_datum_unknown4_bit= 4,
	SOUND_EFFECT_DATUM_UNKNOWN_FLAGS= 0x18,
	SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET= 0xC,
	/* of a sound that plays, of a sound effect: the effect, and who the
	sound had to tell before the effect took its place */
	SOUND_SOURCE_DATUM_EFFECT_INDEX_OFFSET= 0x84,
	SOUND_SOURCE_DATUM_EFFECT_CALLBACKS_OFFSET= 0x88,
	_sound_source_deleted_unknown2= 2,
	/* after the eight pairs of sound_mixbins_add, counting by longs */
	SOUND_MIXBINS_COUNT_INDEX= 0x10,
	/* the ways of sound_decibels_interpolate */
	_sound_interpolation_decibels= 0,
	_sound_interpolation_gain= 1,
	_sound_interpolation_gain_square_root= 2,
	_sound_interpolation_gain_smooth= 3,
	/* in a pitch range: which of the parameters that the sounds of a map
	share are its own; in a scale, two bends of the pitch; in a recording,
	something that says how long it is */
	SOUND_PITCH_RANGE_PARAMETERS_INDEX_OFFSET= 0x2,
	SOUND_PITCH_RANGE_PARAMETERS_SIZE= 0xA,
	SOUND_SCALE_PITCH_BENDS_OFFSET= 0x8,
	SOUND_PERMUTATION_UNKNOWN8_OFFSET= 0x8,
	LOOPING_SOUND_UNKNOWNC_OFFSET= 0xC,
	LOOPING_SOUND_UNKNOWND_OFFSET= 0xD,
	/* why a sound could not start */
	_sound_source_new_failure_unknown2= 2,
	_sound_source_new_failure_unknown3= 3,
	_sound_cache_request_unknown4= 4,
	/* in unknown_0051ebe4: the sound of the places of the map */
	UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET= 0x2A0C,
	UNKNOWN_0051EBE4_ENVIRONMENT_UNKNOWN_SIZE= 0x30,
	UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET= 0x2AB4,
	/* how many reals each class of sounds has in sound_class_gains */
	SOUND_CLASS_GAIN_REALS= 4,
	/* in what the game says of a sound whose place is a column: how high
	the column is */
	SOUND_SOURCE_HEIGHT_OFFSET= 0x30,
	/* in a looping sound: what the game says of it, as of a sound, and
	where it is in the structure bsp */
	LOOPING_SOUND_DATUM_SOURCE_OFFSET= 0xC,
	LOOPING_SOUND_DATUM_LOCATION_OFFSET= 0x34,
	/* in a looping sound: the first sound that plays of each of its
	tracks; in a sound that plays, the next and the one before of its ring */
	LOOPING_SOUND_TRACK_SIZE= 0x14,
	LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET= 0x88,
	SOUND_SOURCE_DATUM_NEXT_OFFSET= 0x6C,
	SOUND_SOURCE_DATUM_PREVIOUS_OFFSET= 0x70,
	/* in unknown_0051ebe4: how many voices and slots there are, and six
	numbers; in a voice: its slot, its state in three bits, and more */
	UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET= 0x4,
	UNKNOWN_0051EBE4_SLOT_COUNT_OFFSET= 0x8,
	UNKNOWN_0051EBE4_COUNTS_OFFSET= 0x1A0C,
	UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET= 0x2AB0,
	VOICE_SLOT_OFFSET= 0,
	VOICE_STATE_MASK= 7,
	VOICE_UNKNOWN24_OFFSET= 0x24,
	VOICE_CONTROLLER_INDEX_OFFSET= 0x28,
	/* what sound_console_get_state fills: a vector of bits for the voices
	of each kind, and six numbers */
	SOUND_CONSOLE_STATE_SIZE= 0x60,
	SOUND_CONSOLE_STATE_KIND_SIZE= 0x10,
	SOUND_CONSOLE_STATE_COUNTS_OFFSET= 0x40,
	_sound_source_datum_kind_unknown2= 2,
	/* more of a looping sound */
	LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET= 0x4,
	LOOPING_SOUND_DATUM_UNKNOWN8_OFFSET= 0x8,
	LOOPING_SOUND_DATUM_SOURCE_SIZE= 0x44,
	/* of a sound voice: where its point is in the structure bsp */
	SOUND_VOICE_DATUM_LOCATION_OFFSET= 0x64,
	/* more of a voice of the console's sound, and of the slots */
	_voice_unknown0_bit= 0,
	_voice_unknown4_bit= 4,
	VOICE_UNKNOWN2C_OFFSET= 0x2C,
	UNKNOWN_0051EBE4_SLOT_SIZE= 0x3C,
	UNKNOWN_0051EBE4_SLOT_STREAM_OFFSET= 0x1A4C,
	/* in what sound_console_voice_start is given */
	SOUND_VOICE_START_SLOT_OFFSET= 0x24,
	SOUND_VOICE_START_CONTROLLER_INDEX_OFFSET= 0x2C,
	/* in what the game says of a sound: where it is in the structure bsp */
	SOUND_SOURCE_LOCATION_OFFSET= 0x28,
	/* what a tag says of a sound effect, and more of a sound effect */
	SOUND_EFFECT_COLLECTION_COUNT_OFFSET= 0x2C,
	SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET= 0x8,
	SOUND_EFFECT_DATUM_UNKNOWN6_OFFSET= 0x6,
	SOUND_EFFECT_DATUM_DEFINITION_OFFSET= 0x10,
	/* more of an element of unknown_00502104: whether its samples are read,
	and of which piece of a recording they are */
	UNKNOWN_00502104_DATUM_DONE_OFFSET= 0x2,
	UNKNOWN_00502104_DATUM_UNKNOWN3_OFFSET= 0x3,
	UNKNOWN_00502104_DATUM_UNKNOWN8_OFFSET= 0x8,
	UNKNOWN_00502104_DATUM_CHUNK_OFFSET= 0xC,
	/* the samples are read from the map in pieces of this many bytes */
	SOUND_CACHE_READ_UNIT= 0x200,
	_sound_cache_read_category= 5,
	_sound_cache_read_priority= 4,
	/* what the sounds of a map have more of some sounds */
	SOUND_DEFINITION_EXTRA_INFO_INDEX_OFFSET= 0xE,
	SOUND_GESTALT_EXTRA_INFOS_OFFSET= 0x50,
	SOUND_EXTRA_INFO_SIZE= 0x2C,
	SOUND_EXTRA_INFO_ENTRY_SIZE= 0x10,
	SOUND_PERMUTATION_UNKNOWN5_OFFSET= 0x5,
	/* in a looping sound: the bits of the recordings that were played of
	each of its tracks */
	LOOPING_SOUND_DATUM_TRACK_PLAYED_OFFSET= 0x8C,
	LOOPING_SOUND_TRACK_OWN_BITS_FLAGS= 0x30,
	/* what the one who is told of a sound is told when all are stopped */
	_sound_source_deleted_unknown10= 10,

	_game_mode_main_menu= 3
};

#define SOUND_PI 3.14159265358979323846f

/* the most that a real can be */
#define SOUND_REAL_MAXIMUM 3.4028234663852886e+38f

/* the least that a sound is heard, in decibels */
#define SOUND_MINIMUM_DECIBELS -64.0f

/* ---------- structures */

/* who is told of what happens to a sound that plays. Only two of its
functions are known */
struct sound_source_callbacks
{
	boolean (__attribute__((stdcall)) *unknown0)(long data, long sound_index, void *unknown5C, void *unknown18);
	void *unknown4[3];
	void (__attribute__((stdcall)) *deleted)(long data, long source_index, long reason);
	void (__attribute__((stdcall)) *stopped)(long data, long source_index);
};

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

/* 0x004ed288: of something else; two numbers of it start again with the sound */
byte *unknown_004ed288;

/* 0x00502108: where the memory of the cache of the sounds starts */
long sound_cache_base;

/* 0x00502118: four reals for each class of sounds */
real *sound_class_gains;

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
void sound_cache_dispose(void);

/* 0x00219b40, 0x0021a0c0, 0x0021d490, 0x0021dda0, 0x0021e4b0: set up the
sound playback controllers, the looping sounds, the sound effects, the sound
voices and something more. FALSE if they cannot */
boolean unknown_00219b40(void);
boolean looping_sounds_initialize(void);
boolean sound_effects_initialize(void);
boolean sound_voices_initialize(const struct sound_configuration *configuration);
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
void __attribute__((stdcall)) sound_console_get_state(byte *state);
void __attribute__((stdcall)) unknown_00127400(boolean unknown);
void sound_playback_controllers_update(void);
void __attribute__((stdcall)) unknown_0021b0e0(long unknown);
void unknown_00128020(void);
void unknown_001282d0(void);
void __attribute__((stdcall)) unknown_00129100(byte *first, byte *second, struct sound_update_listeners *listeners);
long unknown_0014dde0(void);
void __attribute__((stdcall)) unknown_00221a70(long player_index, struct sound_update_listeners *listeners);

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
real sound_definition_get_maximum_decibels(const struct sound_definition *sound);
real sound_definition_get_minimum_decibels(const struct sound_definition *sound);
real sound_gain_to_decibels(real gain);
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
void sound_cache_idle(void);

/* 0x0021f6d0: sets the sound of the places of the map, from three parts of
a tag */

/* 0x00218850: asks for a piece of the samples of a sound. Bits come back */
unsigned long unknown_00218850(long sound_index, const struct sound_chunk *chunk, long unknown);

/* 0x001268e0: of a sound that plays, when its samples are not there */

/* 0x0012a810, 0x00219650 and 0x002197b0 */
real sound_source_fade_decibels(long source_index);
real __attribute__((stdcall)) sound_decibels_interpolate(long kind, real from, real to, real fraction);
real __attribute__((stdcall)) sound_decibels_fade_in(long kind, real decibels, real fraction);



/* 0x0021dde0, 0x0021a100: the sound voices and the looping sounds, after the
structure bsp changes */
void sound_voices_initialize_for_new_structure_bsp(void);
void looping_sounds_initialize_for_new_structure_bsp(void);

/* 0x0012a060, 0x0012a0b0: a channel, when the sound is disabled and when it
is enabled */
/* 0x002ae750: stops a voice of the console's sound for a while */
void unknown_002ae750(void *voice);

/* 0x00129fe0 and 0x0012a060 */
boolean sound_channel_pausable(short channel_index);
void sound_channel_pause(short channel_index);
/* 0x00218b00: how many bytes of a sound of some kind are played in a
time; 0x00218ac0: the bytes of the first chunks of a permutation */
long __attribute__((stdcall)) sound_seconds_to_bytes(long unknown4, long unknown5, long unknown3, real seconds);
long sound_permutation_chunks_size(const struct sound_permutation *permutation, long chunk_count);

/* 0x0021f5d0: a voice of the console's sound starts again from some bytes
into its sound; 0x002ae500: a voice is given a chunk */
void unknown_0021f5d0(short voice_index, long offset);
void unknown_002ae500(void *voice, const struct sound_chunk *chunk);

/* 0x0012a0b0 */
void sound_channel_resume(short channel_index);

/* 0x00219d90, 0x0021f5a0, 0x0021f570: the sound playback controllers and the
sound of the console, when the sound is enabled and disabled */
void sound_console_streams_resume(void);
void sound_console_streams_pause(void);

/* 0x0012a5d0, 0x00126960 and 0x00219c80; 0x0021d4d0: of the sound effects */
void sound_channel_delete(short channel_index);
void sound_source_release_chunk(byte *source);
void sound_playback_controller_release(long controller_index);
void sound_effects_delete_all(void);

/* 0x002ae4b0: a voice of the console's sound stops; 0x0021e050 and
0x00127f10 */
void unknown_002ae4b0(void *voice);
void sound_voice_release(long voice_index);
boolean __attribute__((stdcall)) sound_source_unknown_00127f10(long source_index,
	const struct sound_source_callbacks *callbacks, byte *flags);

/* 0x0021f290: of the sound of the console, and 0x0021a1e0: of the looping
sounds, when the sounds are stopped */
void unknown_0021f290(void);
void looping_sounds_delete_all(void);

/* 0x0037b73f: of the console: a stream of sound stops for a while, or
goes on */
void __attribute__((stdcall)) unknown_0037b73f(void *stream, long pause);

/* 0x00218f10, 0x002194a0, 0x002198f0, 0x00219f60 and 0x0021dcf0 */
short sound_definition_promotion_rule(long rule_index, const struct sound_definition *sound);
const void *sound_definition_get_custom_playback(const struct sound_definition *sound);
void *sound_playback_controller_get(long controller_index);
void __attribute__((stdcall)) sound_playback_controller_add_count(byte *controller, char amount);
void sound_effect_delete(long effect_index);

/* 0x00218950, 0x00218c60, 0x00218d30, 0x002192b0, 0x00219310, 0x00219370,
0x002193d0, 0x0021b070, 0x0021dff0 and 0x0021e1d0 */
long sound_cache_chunk_get_address(const struct sound_chunk *chunk);
real __attribute__((stdcall)) sound_permutation_unknown6(long sound_index, long pitch_range, long permutation);
real sound_definition_get_maximum_distance(long sound_index);
void __attribute__((stdcall)) sound_pitch_range_set_unknown4(struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound, char value);
void __attribute__((stdcall)) sound_pitch_range_set_unknown5(struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound, char value);
long __attribute__((stdcall)) sound_pitch_range_get_unknown4(const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound);
long __attribute__((stdcall)) sound_pitch_range_get_unknown5(const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound);
void looping_sound_delete(long looping_sound_index);
boolean __attribute__((stdcall)) sound_voice_unknown_0021dff0(long voice_index, const byte *unknown, boolean *first);
long sound_classes_match(long class_index, long other_class_index);

/* 0x00218420, 0x00219430, 0x00219910, 0x00219960 and 0x00219a30 */
void unknown_00218420_set_defaults(byte *unknown);
long __attribute__((stdcall)) sound_permutation_get_unknown6(const struct sound_permutation *permutation,
	const struct sound_definition *sound);
void *sound_playback_controller_try_and_get(long controller_index);
long sound_playback_controller_find(long owner);
long sound_playback_controller_acquire(long owner);

/* 0x001462b0: a number from the clocks of the console; 0x00219ed0: of a
sound playback controller */
long unknown_001462b0(void);
void __attribute__((stdcall)) sound_playback_controller_unknown_00219ed0(boolean unknown, byte *controller,
	const struct sound_source *source);

/* 0x0014de10: the local player after one; 0x0012a9d0: something of a
listener and a place; 0x0021d630: of a sound effect */
long unknown_0014de10(long player_index);
real unknown_0012a9d0(long listener_index, const real_point3d *position);
void unknown_0021d630(long effect_index, long unknown);

/* 0x0021d360, 0x0021d5a0 and 0x0021da90 */
void sound_effect_unknown_0021d360(long effect_index);
boolean sound_effect_unknown_0021d5a0(long effect_index);
void __attribute__((stdcall)) sound_effect_source_deleted(long data, long source_index, long reason);

/* 0x002194c0, 0x00219560, 0x00219e90, 0x00219a90 and 0x00219c30 */
unsigned long __attribute__((stdcall)) sound_pitch_range_get_played_bits(const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound);
void __attribute__((stdcall)) sound_pitch_range_set_played_bits(const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound, unsigned long bits);
void __attribute__((stdcall)) sound_playback_controller_initialize(byte *controller, long owner, long unknown8,
	boolean unknown);
long sound_playback_controller_new(long owner);
void __attribute__((stdcall)) sound_playback_controller_unknown_00219c30(long owner, void *unknown);

/* 0x0021e230, 0x0021e270, 0x0021e2d0 and 0x0021e330 */
void __attribute__((stdcall)) sound_mixbins_add(long *mixbins, long mixbin, real decibels);
long __attribute__((stdcall)) sound_frequency_to_filter_coefficient(real frequency);
unsigned long __attribute__((stdcall)) sound_unknown_0021e2d0(real decibels);
long __attribute__((stdcall)) sound_unknown_0021e330(real decibels);

/* 0x00218b80 and 0x00218ca0 */
long double __attribute__((stdcall)) sound_permutation_get_maximum_milliseconds(const struct sound_definition *sound,
	const struct sound_pitch_range *pitch_range, long unknown8);
long double __attribute__((stdcall)) sound_get_maximum_milliseconds(long sound_index, long pitch_range_offset,
	long permutation_offset, real pitch);

/* 0x00219880, 0x0021a050, 0x0021a080 and 0x0021cc30 */
long double __attribute__((stdcall)) sound_unknown_00219880(const real *unknown, unsigned long *seed, real fraction);
long __attribute__((stdcall)) looping_sound_unknown_0021a050(const byte *unknown, const byte *definition);
boolean looping_sound_unknown_0021a080(const byte *unknown);
long __attribute__((stdcall)) sound_unknown_0021cc30(const byte *flags, const struct sound_definition *sound,
	long pitch_range_offset);

/* 0x00126c30 and 0x00126ec0: look at what the game says of a sound that
is to play; 0x00126000: makes it; 0x00129790: of a channel; 0x0021e3b0: of
the console's sound */
boolean unknown_00126c30(long unknown2, void *unknown, long *first, long *failure);
long unknown_00126ec0(void *unknown, long *second);
long unknown_00126000(void *unknown, long first, long unknown2, long second);
void unknown_00129790(long channel_index, long value);
void unknown_0021e3b0(void *unknown);

/* 0x00125f70, 0x001268e0, 0x00129380, 0x0021f6d0 and 0x00221900 */
long __attribute__((stdcall)) sound_source_new(void *unknown, long *failure, long unknown2);
void sound_source_hold_chunk(byte *source);
void sound_channels_unknown_00129380(const long *voice_values, const long *slot_values);
void __attribute__((stdcall)) sound_console_set_environment(const long *second, const long *first, const void *unknown);
void sound_classes_update(real seconds);

/* 0x00126df0, 0x00127e20, 0x00129f20 and 0x0012ac20 */
void __attribute__((stdcall)) sound_sources_cross_fade(long out_source_index, long in_source_index, short kind,
	real seconds);
real sound_source_unknown_00127e20(const struct sound_source *source, long listener_index);
void __attribute__((stdcall)) sound_channel_queue_chunk(short channel_index, const struct sound_permutation *permutation,
	short chunk);
real __attribute__((stdcall)) sound_source_distance_gain(long sound_index, const struct sound_source *source,
	real distance);

/* 0x0021f430 */
void __attribute__((stdcall)) sound_console_unknown_0021f430(long controller_index);

/* 0x0037b822 and 0x0037b657: of the console */
void unknown_0037b822(void *unknown, long unknown2);
void unknown_0037b657(void *unknown);

/* 0x0021b870, 0x0021bc80 and 0x0021d390 */
long __attribute__((stdcall)) looping_sound_find_source(long looping_sound_index, short track, long sound_index,
	long pitch_range);
void looping_sound_add_source(long looping_sound_index, short track, long source_index);
void sound_effects_update(void);

/* 0x00219d90 */
void __attribute__((stdcall)) sound_playback_controllers_unknown_00219d90(boolean enable);

/* 0x0021be20, 0x0021bf00 and 0x0021cf80 */
void __attribute__((stdcall)) sound_source_ring_fade_out(long first_source_index, short kind, real seconds);
boolean __attribute__((stdcall)) looping_sound_get_source(long looping_sound_index, long unused, long unused2,
	struct sound_source *source);
long __attribute__((stdcall)) looping_sound_find(long unknown4, long unknown8);

/* 0x002ae7a0, 0x0037c705, 0x002201f0 and 0x0021fa80: of the console's
sound; 0x00249d60: something of two clusters of a structure bsp */
void unknown_002ae7a0(void *voice, real unknown, real unknown2);
void unknown_0037c705(void *unknown, void *stream);
void unknown_002201f0(long voice_index, long unknown);
void unknown_0021fa80(long voice_index, long unknown, long unknown2);
real unknown_00249d60(long cluster_index, long other_cluster_index, const void *structure_bsp);

/* 0x0021f8a0 and 0x00221da0 */
void __attribute__((stdcall)) sound_console_voice_start(const byte *unknown, long voice_index);
boolean __attribute__((stdcall)) sound_source_unknown_00221da0(long listener_index, const struct sound_source *source,
	real unknown);

/* 0x0018d090 and 0x0018d170: what a tag has of the sound effects;
0x0018cbc0: what the game says of the sound of something */
const byte *unknown_0018d090(long tag_index, long unknown);
const byte *unknown_0018d170(long tag_index);
void unknown_0018cbc0(long unknown, struct sound_source *source);

/* 0x0021d2c0, 0x0021db80 and 0x0021dd30 */
long __attribute__((stdcall)) sound_effect_new(long unknown, real unknownC, short unknown6);
void sound_effect_get_source(long effect_index, struct sound_source *source);
boolean __attribute__((stdcall)) sound_effect_definition_get(long tag_index, long unknown, const byte **definition);

/* 0x00219110: chooses a recording of a pitch range; 0x0021b940: makes the
sound that plays of a track of a looping sound */
long unknown_00219110(const struct sound_definition *sound, short pitch_range_offset, unsigned long *played,
	long last_permutation, unsigned long *seed, long unknown, long unknown2);
long unknown_0021b940(long looping_sound_index, long sound_index, long pitch_range, long track, long unknown,
	long unknown2);

/* 0x00219070 and 0x0021b910 */
short __attribute__((stdcall)) sound_choose_permutation(const struct sound_definition *sound, short pitch_range_offset);
long __attribute__((stdcall)) looping_sound_get_or_new_source(long sound_index, short track, long pitch_range,
	long looping_sound_index, long unknown, long unknown2);

/* 0x00218a10 and 0x00218e50 */
void __attribute__((stdcall)) sound_cache_chunk_load(struct sound_chunk *chunk, long unknown);
real __attribute__((stdcall)) sound_unknown_00218e50(long sound_index, short permutation, short pitch_range_offset,
	short index);

/* 0x0021cb90 and 0x0021cbe0 */
unsigned long __attribute__((stdcall)) looping_sound_track_get_played_bits(long track,
	const struct sound_definition *sound, byte *looping_sound, const byte *flags, short pitch_range_offset, boolean clear);
void __attribute__((stdcall)) looping_sound_track_set_played_bits(long track, unsigned long bits, byte *looping_sound,
	const struct sound_definition *sound, const byte *flags, short pitch_range_offset, char last_permutation);

/* 0x001286b0, 0x00127fc0 and 0x0012abe0 */
void __attribute__((stdcall)) sound_source_set_sound(long source_index, long sound_index, char pitch_range,
	char permutation, short chunk);
boolean sound_source_unknown_00127fc0(long source_index);
void sound_source_get_position(const struct sound_source *source, long listener_index, real_point3d *position);

/* 0x00126360, 0x00126400, 0x00127320 and 0x00127390 */
void sound_source_stop(long source_index);
void sound_stop_all(void);
void __attribute__((stdcall)) sound_source_delete(long source_index, long reason);
void sound_source_free(long source_index);

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
			if (unknown_00219b40() && looping_sounds_initialize() && sound_effects_initialize() &&
				sound_voices_initialize(&sound_configuration) && unknown_0021e4b0(&sound_configuration))
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
		sound_cache_dispose();
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

				*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(source_index);
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

	sound_reset(0);
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
		sound_voices_initialize_for_new_structure_bsp();
		looping_sounds_initialize_for_new_structure_bsp();
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
			sound_console_streams_pause();
			globals= game_sound_globals;
		}
		else if (enable==1)
		{
			unsigned long time;

			for (index= 0; index<*(short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET); index++)
			{
				sound_channel_resume((short)index);
			}
			sound_playback_controllers_unknown_00219d90(TRUE);
			sound_console_streams_resume();
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
			sound_cache_idle();
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

		sound_console_set_environment((const long *)(environment+0x20), (const long *)(environment+0x18), environment+0x28);
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
		sound_source_hold_chunk(source);
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
		long offset= sound_seconds_to_bytes((char)sound->unknown3[1], (char)sound->unknown3[2], (char)sound->unknown3[0],
			channel->unknown10);

		offset-= sound_permutation_chunks_size(permutation, channel->unknown18);
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
	real most= sound_definition_get_maximum_decibels(sound);
	real least_gain= (real)sound_decibels_to_gain(sound_definition_get_minimum_decibels(sound));
	long double most_gain= sound_decibels_to_gain(most);
	real gain= (real)((most_gain-(long double)least_gain)*(long double)interpolation+(long double)least_gain);
	real gain_decibels= sound_gain_to_decibels(gain);

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
				sound_playback_controllers_unknown_00219d90(TRUE);
				sound_console_streams_resume();
				time= GetTickCount();
				globals= game_sound_globals;
				*(unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= time;
				globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
			}
			sound_console_get_state(globals);
			if (!globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET])
			{
				unknown_001269f0();
				globals= game_sound_globals;
			}
			sound_classes_update(*(real *)(globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET));
			unknown_00127400(TRUE);
			sound_playback_controllers_update();
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
			sound_channels_unknown_00129380((const long *)second, (const long *)first);
			unknown_001293f0();
			if (game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET])
			{
				game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET]++;
			}
		}
		unknown_0021ee80();
		sound_playback_controllers_unknown_00219d90(FALSE);
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

static inline __attribute__((always_inline)) byte *sound_source_get(
	long source_index)
{
	return (byte *)sound_sources->data+(source_index&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;
}

/* whether a sound that plays is of one of the kinds 1 to 4 */
static inline __attribute__((always_inline)) boolean sound_source_kind_test(
	const byte *source)
{
	return ((1UL<<(source[SOUND_SOURCE_DATUM_KIND_OFFSET]&31))&SOUND_SOURCE_DATUM_KINDS_UNKNOWN)!=0;
}

/* 0x00126360: a sound that plays starts to fade out, as all do when a map
is let go, and who is told of it is told. Nothing for an index that is of no
sound, or of one that is being let go. The name is a guess */
void sound_source_stop(
	long source_index)
{
	if (source_index!=NONE && datum_try_and_get(sound_sources, source_index))
	{
		byte *source= sound_source_get(source_index);

		if (!source[SOUND_SOURCE_DATUM_UNKNOWN2_OFFSET])
		{
			const struct sound_source_callbacks *callbacks=
				*(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);

			if (callbacks && callbacks->stopped)
			{
				callbacks->stopped(*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET), source_index);
			}
			source= sound_source_get(source_index);
			*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(source_index);
			*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= 0;
			*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= SOUND_FADE_OUT_TIME;
			*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
			source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
		}
	}
}

/* 0x00126400: all the sounds that play are let go at once, with no fading.
The name is a guess */
void sound_stop_all(
	void)
{
	if (game_sound_globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET])
	{
		long source_index;

		sound_effects_delete_all();
		for (source_index= data_next_index(sound_sources, NONE);
			source_index!=NONE;
			source_index= data_next_index(sound_sources, source_index))
		{
			if (!sound_source_get(source_index)[SOUND_SOURCE_DATUM_UNKNOWN2_OFFSET])
			{
				sound_source_delete(source_index, _sound_source_deleted_unknown10);
			}
		}
	}
}

/* 0x00127320: a sound that plays is let go: its channel, what it has, and
its place in the array; who is told of it is told why. The name is a guess */
void __attribute__((stdcall)) sound_source_delete(
	long source_index,
	long reason)
{
	byte *source= sound_source_get(source_index);
	const struct sound_source_callbacks *callbacks;

	if (*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)!=NONE)
	{
		sound_channel_delete(*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET));
		*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)= NONE;
	}
	sound_source_release_chunk(source);
	callbacks= *(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);
	if (callbacks && callbacks->deleted)
	{
		callbacks->deleted(*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET), source_index, reason);
	}
	sound_source_free(source_index);
}

/* 0x00127390: the place of a sound in the array of those that play is let
go, and its sound playback controller has one sound less. The name is a
guess */
void sound_source_free(
	long source_index)
{
	struct data_array *data= sound_sources;
	const byte *source= (const byte *)data->data+(source_index&0xFFFF)*SOUND_SOURCE_DATUM_SIZE;
	long controller_index= *(const long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET);

	if (controller_index!=NONE)
	{
		byte *controller= (byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;

		if (source[SOUND_SOURCE_DATUM_UNKNOWN3_OFFSET]==0xFF && *(long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE)
		{
			controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]=
				(controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]-1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
		}
		sound_playback_controller_release(*(const long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET));
	}
	datum_delete(data, source_index);
}

/* a piece of the samples of a recording has one less that counts in it */
static inline __attribute__((always_inline)) void sound_chunk_release(
	const struct sound_chunk *chunk)
{
	long index= *(const long *)((const byte *)chunk+8);
	byte *datum= (byte *)unknown_00502104->data+(index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;

	datum[UNKNOWN_00502104_DATUM_COUNT_OFFSET]--;
}

/* 0x00126960: a sound that plays does not count any more in the piece of
the samples that it is at, if it did. The name is a guess */
void sound_source_release_chunk(
	byte *source)
{
	if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_holds_chunk_bit)&1)
	{
		const struct sound_definition *sound=
			sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));

		sound_chunk_release(sound_get_chunk(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET],
			(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET],
			*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)));
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]&= ~FLAG(_sound_source_datum_holds_chunk_bit);
	}
}

/* 0x001286b0: what a sound that plays is of changes: the sound, and the
pitch range, the recording and the piece of the samples that it is at. The
name is a guess */
void __attribute__((stdcall)) sound_source_set_sound(
	long source_index,
	long sound_index,
	char pitch_range,
	char permutation,
	short chunk)
{
	byte *source= sound_source_get(source_index);

	sound_source_release_chunk(source);
	*(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)= sound_index;
	source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]= pitch_range;
	source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]= permutation;
	*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)= chunk;
}

/* 0x0012a5d0: a channel lets go of the sound that it plays: the pieces of
the samples of its recordings, its voice of the console's sound and its
sound voice. The name is a guess */
void sound_channel_delete(
	short channel_index)
{
	struct sound_channel *channel= &sound_channels[channel_index];

	channel->unknown0= NONE;
	if (channel->unknown20)
	{
		sound_chunk_release(sound_permutation_chunk((const struct sound_permutation *)(uintptr_t)channel->unknown20,
			channel->unknown1A));
		channel->unknown20= 0;
	}
	if (channel->unknown1C)
	{
		sound_chunk_release(sound_permutation_chunk((const struct sound_permutation *)(uintptr_t)channel->unknown1C,
			channel->unknown18));
		channel->unknown1C= 0;
	}
	if (channel->unknownC!=NONE)
	{
		unsigned long *taken;

		unknown_002ae4b0(sound_channel_voice(channel));
		taken= (unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN60_OFFSET);
		taken[channel->unknownC>>5]&= ~(1UL<<(channel->unknownC&31));
	}
	if (channel->unknown4!=NONE)
	{
		sound_voice_release(channel->unknown4);
	}
	channel->unknownA= 0;
	channel->unknownB= 0;
	channel->unknown9= 0;
	channel->unknownC= NONE;
	channel->unknownE= NONE;
	channel->unknown4= NONE;
}

/* 0x00219c80: a sound playback controller has one sound less, and is let
go with none. The name is a guess */
void sound_playback_controller_release(
	long controller_index)
{
	if (controller_index!=NONE)
	{
		byte *controller= (byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;
		byte sounds= controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET];

		sounds= (sounds&~UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|((sounds-1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK);
		controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]= sounds;
		if (!(sounds&UNKNOWN_0051EBD8_DATUM_COUNT_MASK))
		{
			datum_delete(unknown_0051ebd8, controller_index);
		}
	}
}

/* 0x00127fc0: of a sound that plays with someone to tell, when the time at
0x8C of it is not come or the sound is disabled: what sound_source_unknown_00127f10 says.
TRUE for the rest. What it means is not known */
boolean sound_source_unknown_00127fc0(
	long source_index)
{
	byte *source= sound_source_get(source_index);
	byte flags= source[SOUND_SOURCE_DATUM_FLAGS_OFFSET];

	if (!(flags&FLAG(_sound_source_datum_unknown0_bit)) && !((flags>>_sound_source_datum_unknown5_bit)&1))
	{
		const struct sound_source_callbacks *callbacks=
			*(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);

		if (callbacks && callbacks->unknown0)
		{
			const byte *globals= game_sound_globals;

			if (*(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)<*(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET) ||
				globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET])
			{
				return sound_source_unknown_00127f10(source_index, callbacks, source+SOUND_SOURCE_DATUM_FLAGS_OFFSET);
			}
		}
	}

	return TRUE;
}

/* 0x0012abe0: where a sound is, of what the game says of it: its place, or
its place at the height of a listener. Nothing for any other kind. The name
is a guess */
void sound_source_get_position(
	const struct sound_source *source,
	long listener_index,
	real_point3d *position)
{
	const long *place= (const long *)((const byte *)source+SOUND_SOURCE_POSITION_OFFSET);
	long *result= (long *)position;

	switch (((const char *)source)[SOUND_SOURCE_POSITION_KIND_OFFSET])
	{
	case _sound_source_position_world:
		result[0]= place[0];
		result[1]= place[1];
		result[2]= place[2];
		break;
	case _sound_source_position_listener_height:
	{
		const struct sound_listener *listeners=
			(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET);

		result[0]= place[0];
		result[1]= place[1];
		result[2]= *(const long *)&listeners[listener_index].position.z;
		break;
	}
	}
}

/* 0x0021e050: a sound voice has one less that uses it; with none, its slot
of the globals of the sound and its place in the array are let go. The name
is a guess */
void sound_voice_release(
	long voice_index)
{
	if (voice_index!=NONE)
	{
		byte *voice= (byte *)sound_voices->data+(voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE;

		if (!--*(short *)(voice+SOUND_VOICE_DATUM_USERS_OFFSET))
		{
			sound_slot_delete(*(short *)(voice+SOUND_VOICE_DATUM_SLOT_OFFSET));
			datum_delete(sound_voices, voice_index);
		}
	}
}

/* 0x00127f10: who is told of a sound that plays is asked something of it,
with two parts of the sound. If the answer is no, the sound is marked (the
bit 5 of the flags that the function is given), and what comes back is
whether the sound is of the kind 0 or of a kind over 4 and its class is not
one of those that sound_class_unknown_00124f90 tells of. TRUE with no one to
ask. What it means is not known */
boolean __attribute__((stdcall)) sound_source_unknown_00127f10(
	long source_index,
	const struct sound_source_callbacks *callbacks,
	byte *flags)
{
	byte *source= sound_source_get(source_index);
	long sound_index= *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET);
	const struct sound_definition *sound= sound_definition_get(sound_index);

	if (!callbacks)
	{
		return TRUE;
	}
	if (callbacks->unknown0(*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET), sound_index,
		source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET))
	{
		return TRUE;
	}
	if (!sound_source_kind_test(sound_source_get(source_index)) &&
		!sound_class_unknown_00124f90(sound_class_get(sound->sound_class)))
	{
		*flags|= FLAG(_sound_source_datum_unknown5_bit);
		return TRUE;
	}
	*flags|= FLAG(_sound_source_datum_unknown5_bit);

	return FALSE;
}

/* whether a sound that plays goes on when the sounds are stopped and the
state of the game is kept */
static inline __attribute__((always_inline)) boolean sound_reset_keeps_source(
	const byte *source,
	boolean unknown6)
{
	short sound_class= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class;

	if (sound_class==_sound_class_unknown32)
	{
		return TRUE;
	}
	if (sound_class==_sound_class_unknown38)
	{
		return unknown6;
	}
	if (!sound_source_kind_test(source))
	{
		return FALSE;
	}

	return ((const byte *)sound_class_get(sound_class))[SOUND_CLASS_UNKNOWN4C_OFFSET]==1;
}

/* 0x001264c0: the sounds that play are let go, and the time of the sound
starts again. It is one of the procedures of the state of the game, after it
is read; its flags say then (the bit 5) that some sounds go on: those of two
classes, and those of some kinds whose class says so. The name is a guess */
void __attribute__((stdcall)) sound_reset(
	unsigned long flags)
{
	boolean unknown6= (flags>>_sound_reset_unknown6_bit)&1;
	boolean keep= (flags>>_sound_reset_keep_bit)&1;
	byte *globals= game_sound_globals;
	byte *unknown;

	if (!keep && globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET])
	{
		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= TRUE;
		globals[GAME_SOUND_GLOBALS_DISABLED_OFFSET]= FALSE;
		globals[GAME_SOUND_GLOBALS_UNKNOWN7D_OFFSET]= FALSE;
	}
	if (globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET])
	{
		long source_index;

		sound_effects_delete_all();
		for (source_index= data_next_index(sound_sources, NONE);
			source_index!=NONE;
			source_index= data_next_index(sound_sources, source_index))
		{
			byte *source= sound_source_get(source_index);

			if (keep && sound_reset_keeps_source(source, unknown6))
			{
				*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)= NONE;
			}
			else
			{
				sound_source_delete(source_index, _sound_source_deleted_unknown10);
			}
		}
		if (keep)
		{
			sound_idle();
		}
		else
		{
			unknown_0021f290();
		}
	}
	*(unsigned long *)(game_sound_globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= GetTickCount();
	globals= game_sound_globals;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET)= NONE;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET)= NONE;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)= 0;
	unknown= unknown_004ed288;
	*(long *)(unknown+UNKNOWN_004ED288_UNKNOWN20_OFFSET)= 0;
	*(long *)(unknown+UNKNOWN_004ED288_UNKNOWN20_OFFSET+4)= 0;
	if (!keep)
	{
		looping_sounds_delete_all();
	}
}

/* ---------- of other files of the sound, most likely: the sound cache
(0x00218810, 0x00218ac0, 0x00218f10), the sound definitions (0x002194a0), the
sound playback controllers (0x002198f0, 0x00219f60), the looping sounds
(0x0021a0c0), the sound effects (0x0021d490, 0x0021dcf0), the sound voices
(0x0021dda0) and the sound of the console (0x0021f570, 0x0021f5a0). The
names are guesses */

/* 0x00218810: a tick of the cache of the sounds goes by */
void sound_cache_idle(
	void)
{
	lruv_idle(sound_lruv_cache);
}

/* 0x00218ac0: how many bytes the first pieces of the samples of a recording
are */
long sound_permutation_chunks_size(
	const struct sound_permutation *permutation,
	long chunk_count)
{
	long size= 0;
	long index;

	for (index= 0; index<chunk_count; index++)
	{
		size+= *(const long *)((const byte *)sound_permutation_chunk(permutation, 0)+index*(long)sizeof(struct sound_chunk)+
			SOUND_CHUNK_SIZE_OFFSET)&SOUND_CHUNK_SIZE_MASK;
	}

	return size;
}

/* 0x00218f10: what one of the rules of the promotion of a sound says, in
its first word; 0 if the sound has no promotion or the rule says NONE */
short sound_definition_promotion_rule(
	long rule_index,
	const struct sound_definition *sound)
{
	char promotion_index= ((const char *)sound)[SOUND_DEFINITION_PROMOTION_INDEX_OFFSET];
	uintptr_t promotion;

	if (promotion_index==NONE)
	{
		return 0;
	}
	promotion= (uintptr_t)*(const long *)((const byte *)global_sound_cache_file_gestalt+SOUND_GESTALT_PROMOTIONS_OFFSET+4)+
		(uintptr_t)(promotion_index*SOUND_PROMOTION_SIZE);
	if (promotion)
	{
		const byte *rules= (const byte *)(uintptr_t)*(const long *)(promotion+4);
		short rule= *(const short *)(rules+rule_index*SOUND_PROMOTION_RULE_SIZE);

		if (rule!=NONE)
		{
			return rule;
		}
	}

	return 0;
}

/* 0x002194a0: how a sound is played that is its own, and not of its class;
NULL if it has none */
const void *sound_definition_get_custom_playback(
	const struct sound_definition *sound)
{
	char custom_playback_index= ((const char *)sound)[SOUND_DEFINITION_CUSTOM_PLAYBACK_INDEX_OFFSET];

	if (custom_playback_index==NONE)
	{
		return NULL;
	}

	return (const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown30[0].address+
		custom_playback_index*SOUND_CUSTOM_PLAYBACK_SIZE;
}

/* 0x002198f0: a sound playback controller; the one of unknown_0051ebdc for
NONE */
void *sound_playback_controller_get(
	long controller_index)
{
	if (controller_index==NONE)
	{
		return unknown_0051ebdc;
	}

	return (byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;
}

/* 0x00219f60: the count of 7 bits at 3 of a sound playback controller
changes by some, if what the controller has at 4 is not NONE */
void __attribute__((stdcall)) sound_playback_controller_add_count(
	byte *controller,
	char amount)
{
	if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE)
	{
		byte count= controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET];

		controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]=
			(count&~UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|((count+amount)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK);
	}
}

/* 0x0021a0c0: makes the array of the looping sounds. FALSE if it cannot */
boolean looping_sounds_initialize(
	void)
{
	struct data_array *data= shell_memory_data_new("looping sounds", MAXIMUM_LOOPING_SOUNDS, LOOPING_SOUND_DATUM_SIZE);

	looping_sounds= data;
	if (data)
	{
		data_make_valid(data);
	}

	return looping_sounds!=NULL;
}

/* 0x0021d490: makes the array of the sound effects. FALSE if it cannot */
boolean sound_effects_initialize(
	void)
{
	struct data_array *data= shell_memory_data_new("sounds effects", MAXIMUM_SOUND_EFFECTS, SOUND_EFFECT_DATUM_SIZE);

	sound_effects= data;
	if (data)
	{
		data_make_valid(data);
	}

	return sound_effects!=NULL;
}

/* 0x0021dcf0: a sound effect is let go, and its sound playback controller
has one sound less */
void sound_effect_delete(
	long effect_index)
{
	struct data_array *data= sound_effects;
	byte *effect= (byte *)data->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;

	if (*(const long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)!=NONE)
	{
		sound_playback_controller_release(*(const long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET));
		*(long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)= NONE;
	}
	datum_delete(data, effect_index);
}

/* 0x0021dda0: makes the array of the sound voices, of as many as the
sound has. FALSE if it cannot */
boolean sound_voices_initialize(
	const struct sound_configuration *configuration)
{
	struct data_array *data= shell_memory_data_new("sound voices", configuration->maximum_voices, SOUND_VOICE_DATUM_SIZE);

	sound_voices= data;
	if (!data)
	{
		return FALSE;
	}
	data_make_valid(data);

	return TRUE;
}

/* 0x0021f570 and 0x0021f5a0: the two streams of the console's sound stop
for a while, and go on */
void sound_console_streams_pause(
	void)
{
	long offset;

	for (offset= UNKNOWN_0051EBE4_STREAMS_OFFSET; offset<UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET; offset+= 4)
	{
		unknown_0037b73f(*(void **)(unknown_0051ebe4+offset), TRUE);
	}
}

void sound_console_streams_resume(
	void)
{
	long offset;

	for (offset= UNKNOWN_0051EBE4_STREAMS_OFFSET; offset<UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET; offset+= 4)
	{
		unknown_0037b73f(*(void **)(unknown_0051ebe4+offset), FALSE);
	}
}

/* ---------- more of other files of the sound, most likely. The names are
guesses */

/* 0x00218950: where the samples of a piece of a recording are in the cache
of the sounds; it is counted too that they were asked for, up to 255 times */
long sound_cache_chunk_get_address(
	const struct sound_chunk *chunk)
{
	long index= *(const long *)((const byte *)chunk+SOUND_CHUNK_CACHE_INDEX_OFFSET);
	byte *datum= (byte *)unknown_00502104->data+(index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;
	const struct lruv_cache *cache;
	const struct lruv_cache_block *blocks;

	if (datum[UNKNOWN_00502104_DATUM_USES_OFFSET]<0xFF)
	{
		datum[UNKNOWN_00502104_DATUM_USES_OFFSET]++;
	}
	index= *(const long *)((const byte *)chunk+SOUND_CHUNK_CACHE_INDEX_OFFSET);
	cache= sound_lruv_cache;
	blocks= (const struct lruv_cache_block *)cache->blocks->data;

	return (long)((unsigned long)blocks[index&0xFFFF].first_page_index<<(cache->page_size_bits&31))+sound_cache_base;
}

/* 0x00218c60: the word at 6 of a recording of a sound, as a real: the
recording is one of a pitch range of the sound */
real __attribute__((stdcall)) sound_permutation_unknown6(
	long sound_index,
	long pitch_range,
	long permutation)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const struct sound_pitch_range *pitch_ranges= (const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address;
	const byte *permutations= (const byte *)(uintptr_t)gestalt->permutations.address;
	long index= pitch_ranges[sound_definition_get(sound_index)->first_pitch_range_index+pitch_range].first_permutation_index+
		permutation;

	return (real)(long)*(const word *)(permutations+index*(long)sizeof(struct sound_permutation)+SOUND_PERMUTATION_UNKNOWN6_OFFSET);
}

/* 0x00218d30: how far a sound is heard: what its class says, if the sound
says to ask it, or else how the sound is played */
real sound_definition_get_maximum_distance(
	long sound_index)
{
	const struct sound_definition *sound= sound_definition_get(sound_index);

	if (sound->flags&FLAG(_sound_definition_class_distance_bit))
	{
		return sound_class_get(sound->sound_class)->maximum_distance;
	}

	return sound_definition_get_playback(sound)->maximum_distance;
}

/* whether the class of a sound says that its pitch ranges keep nothing of
what was played */
static inline __attribute__((always_inline)) boolean sound_definition_class_keeps_nothing(
	const struct sound_definition *sound)
{
	return (sound_class_get(sound->sound_class)->internal_flags>>_sound_class_internal_unknown5_bit)&1;
}

/* 0x002192b0 and 0x00219310: the bytes at 4 and at 5 of a pitch range of a
sound are set, if the class of the sound lets them be */
void __attribute__((stdcall)) sound_pitch_range_set_unknown4(
	struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound,
	char value)
{
	if (!sound_definition_class_keeps_nothing(sound))
	{
		pitch_range->unknown0[4]= value;
	}
}

void __attribute__((stdcall)) sound_pitch_range_set_unknown5(
	struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound,
	char value)
{
	if (!sound_definition_class_keeps_nothing(sound))
	{
		pitch_range->unknown0[5]= value;
	}
}

/* 0x00219370 and 0x002193d0: the bytes at 4 and at 5 of a pitch range of a
sound; NONE if the class of the sound does not let them be kept */
long __attribute__((stdcall)) sound_pitch_range_get_unknown4(
	const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound)
{
	if (!sound_definition_class_keeps_nothing(sound))
	{
		return (char)pitch_range->unknown0[4];
	}

	return NONE;
}

long __attribute__((stdcall)) sound_pitch_range_get_unknown5(
	const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound)
{
	if (!sound_definition_class_keeps_nothing(sound))
	{
		return (char)pitch_range->unknown0[5];
	}

	return NONE;
}

/* 0x0021b070: a looping sound is let go, and its sound playback controller
has one sound less */
void looping_sound_delete(
	long looping_sound_index)
{
	struct data_array *data= looping_sounds;
	const byte *looping_sound= (const byte *)data->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;

	sound_playback_controller_release(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET));
	datum_delete(data, looping_sound_index);
}

/* 0x0021dff0: a sound voice is given 0x24 bytes of something, once each
time that the time of the sound starts: whether it took them. It says too
whether the voice had never been given any */
boolean __attribute__((stdcall)) sound_voice_unknown_0021dff0(
	long voice_index,
	const byte *unknown,
	boolean *first)
{
	byte *voice= (byte *)sound_voices->data+(voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE;
	long start_time= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET);

	*first= *(const long *)(voice+SOUND_VOICE_DATUM_START_TIME_OFFSET)==NONE;
	if (*(const long *)(voice+SOUND_VOICE_DATUM_START_TIME_OFFSET)==start_time)
	{
		return FALSE;
	}
	memcpy(voice+SOUND_VOICE_DATUM_UNKNOWN48_OFFSET, unknown+SOUND_VOICE_DATUM_UNKNOWN_SOURCE_OFFSET,
		SOUND_VOICE_DATUM_UNKNOWN48_SIZE);
	*(long *)(voice+SOUND_VOICE_DATUM_START_TIME_OFFSET)= start_time;

	return TRUE;
}

/* 0x0021e1d0: whether two classes of sounds are the same, or have the same
real at 0x10 and reals at 0x50 that are closer than 0.0001 */
long sound_classes_match(
	long class_index,
	long other_class_index)
{
	const struct sound_class *sound_class;
	const struct sound_class *other;
	long double difference;

	if (class_index==other_class_index)
	{
		return TRUE;
	}
	sound_class= sound_class_get((short)class_index);
	other= sound_class_get((short)other_class_index);
	if (*(const real *)((const byte *)sound_class+SOUND_CLASS_UNKNOWN10_OFFSET)!=
		*(const real *)((const byte *)other+SOUND_CLASS_UNKNOWN10_OFFSET))
	{
		return FALSE;
	}
	sound_class= sound_class_get((short)class_index);
	other= sound_class_get((short)other_class_index);
	difference= (long double)*(const real *)((const byte *)sound_class+SOUND_CLASS_UNKNOWN50_OFFSET)-
		(long double)*(const real *)((const byte *)other+SOUND_CLASS_UNKNOWN50_OFFSET);
	if (difference<0.0L)
	{
		difference= -difference;
	}

	return 0.0001f>difference;
}

/* 0x00218420: what something of 0x1C bytes is when it is new: two reals,
120 and 60, and twenty bytes. What it is of is not known */
void unknown_00218420_set_defaults(
	byte *unknown)
{
	static const byte defaults[UNKNOWN_00218420_BYTES_SIZE]=
	{
		0, 4, 2, 3, 1, 5, 6, 7, 12, 13, 14, 15, 10, 11, 0, 1, 0, 0, 0, 0
	};

	((real *)unknown)[0]= 120.0f;
	((real *)unknown)[1]= 60.0f;
	memcpy(unknown+UNKNOWN_00218420_BYTES_OFFSET, defaults, sizeof(defaults));
}

/* the reals of a class of sounds and of the scale of a sound that say how
loud the sound is, in decibels: the least, and then the most */
static inline __attribute__((always_inline)) const real *sound_definition_class_decibels(
	const struct sound_definition *sound)
{
	return (const real *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_DECIBELS_OFFSET);
}

static inline __attribute__((always_inline)) const real *sound_definition_scale_decibels(
	const struct sound_definition *sound)
{
	return (const real *)((const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown8[0].address+
		((const char *)sound)[SOUND_DEFINITION_SCALE_INDEX_OFFSET]*SOUND_SCALE_SIZE);
}

/* 0x00218d70 and 0x00218de0: the least and the most that a sound is heard,
in decibels: what its scale says and what its class says, added */
real sound_definition_get_minimum_decibels(
	const struct sound_definition *sound)
{
	real class_decibels= sound_definition_class_decibels(sound)[0];

	return sound_definition_scale_decibels(sound)[0]+class_decibels;
}

real sound_definition_get_maximum_decibels(
	const struct sound_definition *sound)
{
	real class_decibels= sound_definition_class_decibels(sound)[1];

	return sound_definition_scale_decibels(sound)[1]+class_decibels;
}

/* 0x00219430: the word at 6 of a recording of a sound, if the class of the
sound does not let its pitch ranges keep what was played; NONE if it does,
or if the word is NONE */
long __attribute__((stdcall)) sound_permutation_get_unknown6(
	const struct sound_permutation *permutation,
	const struct sound_definition *sound)
{
	if (sound_definition_class_keeps_nothing(sound))
	{
		word unknown6= *(const word *)((const byte *)permutation+SOUND_PERMUTATION_UNKNOWN6_OFFSET);

		return unknown6==0xFFFF ? NONE : unknown6;
	}

	return NONE;
}

/* 0x00219910: a sound playback controller: the one of unknown_0051ebdc for
NONE, and NULL if the index is of none */
void *sound_playback_controller_try_and_get(
	long controller_index)
{
	if (controller_index==NONE)
	{
		return unknown_0051ebdc;
	}

	return datum_try_and_get(unknown_0051ebd8, controller_index);
}

/* 0x00219960: the sound playback controller of something: the one that has
it at 4. NONE if there is none, or with no sound */
long sound_playback_controller_find(
	long owner)
{
	long controller_index;

	if (!sound_available_inline() || owner==NONE)
	{
		return NONE;
	}
	for (controller_index= data_next_index(unknown_0051ebd8, NONE);
		controller_index!=NONE;
		controller_index= data_next_index(unknown_0051ebd8, controller_index))
	{
		const byte *controller= sound_playback_controller_get(controller_index);

		if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)==owner)
		{
			return controller_index;
		}
	}

	return NONE;
}

/* 0x00219a30: the sound playback controller of something has one sound
more. Gives the controller; NONE if there is none */
long sound_playback_controller_acquire(
	long owner)
{
	long controller_index;

	if (!sound_available_inline() || owner==NONE)
	{
		return NONE;
	}
	controller_index= sound_playback_controller_find(owner);
	if (controller_index!=NONE)
	{
		byte *controller= (byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;
		byte sounds= controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET];

		controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]=
			(sounds&~UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|((sounds+1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK);
	}

	return controller_index;
}

/* 0x0044a054 to 0x0044a0a0: what the three numbers of the tag of a sound
that say how its samples are kept mean: the samples in a second (the byte at
3 of the tag), two numbers of each of the three ways at 4, and two of each of
the five ways at 5 */
static const long sound_sample_rates[]= { 22050, 44100, 32000 };
static const real sound_unknown4_reals[]= { 1.0f, 0.5f, 1.0f };
static const real sound_unknown5_reals[]= { 0.5f, 16.0f/9.0f, 0.5f, 0.5f, 1.0f };
static const long sound_unknown5_block_sizes[]= { 2, 36, 2, 2, 1 };
static const long sound_unknown4_channels[]= { 1, 2, 1 };

/* 0x00218b00: how many bytes of the samples of a sound are played in some
seconds: a whole number of blocks */
long __attribute__((stdcall)) sound_seconds_to_bytes(
	long unknown4,
	long unknown5,
	long unknown3,
	real seconds)
{
	real blocks= 1.0f/sound_unknown5_reals[unknown5];

	blocks/= sound_unknown4_reals[unknown4];
	blocks*= (real)sound_sample_rates[unknown3];
	blocks*= seconds;
	blocks/= (real)sound_unknown5_block_sizes[unknown5];
	blocks/= (real)sound_unknown4_channels[unknown4];

	return sound_unknown4_channels[unknown4]*sound_unknown5_block_sizes[unknown5]*(long)blocks;
}

/* how many bytes the bits of the recordings of a pitch range are: none if
the class of the sound keeps nothing of what was played, or with one
recording or less */
static inline __attribute__((always_inline)) unsigned long sound_pitch_range_played_bits_size(
	const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound)
{
	if (sound_definition_class_keeps_nothing(sound) || pitch_range->permutation_count<=1)
	{
		return 0;
	}

	return (unsigned long)(pitch_range->permutation_count+7)>>3;
}

static inline __attribute__((always_inline)) byte *sound_pitch_range_played_bits(
	const struct sound_pitch_range *pitch_range)
{
	return (byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown30[1].address+
		(*(const word *)(pitch_range->unknown0+SOUND_PITCH_RANGE_PLAYED_BITS_OFFSET)>>3);
}

/* 0x002194c0 and 0x00219560: the bits of the recordings of a pitch range of
a sound that were played, of those that the sounds of a map share: one for
each recording. 0 if they are not kept */
unsigned long __attribute__((stdcall)) sound_pitch_range_get_played_bits(
	const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound)
{
	unsigned long bits= 0;
	unsigned long size= sound_pitch_range_played_bits_size(pitch_range, sound);

	if (size)
	{
		memcpy(&bits, sound_pitch_range_played_bits(pitch_range), size);
	}

	return bits;
}

void __attribute__((stdcall)) sound_pitch_range_set_played_bits(
	const struct sound_pitch_range *pitch_range,
	const struct sound_definition *sound,
	unsigned long bits)
{
	unsigned long size= sound_pitch_range_played_bits_size(pitch_range, sound);

	if (size)
	{
		memcpy(sound_pitch_range_played_bits(pitch_range), &bits, size);
	}
}

/* 0x00219e90: a sound playback controller is made new for something */
void __attribute__((stdcall)) sound_playback_controller_initialize(
	byte *controller,
	long owner,
	long unknown8,
	boolean unknown)
{
	*(long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)= owner;
	*(long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET)= unknown8;
	controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]=
		(controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]&UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|
		(unknown ? 0 : UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG);
	controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]= UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG;
	memset(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET, 0, UNKNOWN_0051EBD8_DATUM_UNKNOWNC_SIZE);
}

/* 0x00219a90: the sound playback controller of something has one sound
more; if there was none, one is made. Gives the controller; NONE if it
cannot */
long sound_playback_controller_new(
	long owner)
{
	const byte *globals= game_sound_globals;
	long controller_index;
	byte *controller;
	byte sounds;

	if (!sound_available_inline() || owner==NONE)
	{
		return NONE;
	}
	controller_index= sound_playback_controller_find(owner);
	if (controller_index==NONE)
	{
		controller_index= datum_new(unknown_0051ebd8);
		if (controller_index==NONE)
		{
			return NONE;
		}
	}
	controller= sound_playback_controller_get(controller_index);
	sounds= controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET];
	sounds= (sounds&~UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|((sounds+1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK);
	controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]= sounds;
	if ((sounds&UNKNOWN_0051EBD8_DATUM_COUNT_MASK)==1)
	{
		boolean unknown= globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET]&1;

		sound_playback_controller_initialize(controller, owner, unknown_001462b0(), unknown);
	}

	return controller_index;
}

/* 0x00219c30: the sound playback controller of something, if it has one,
is given to sound_playback_controller_unknown_00219ed0 with what the function is given */
void __attribute__((stdcall)) sound_playback_controller_unknown_00219c30(
	long owner,
	void *unknown)
{
	const byte *globals= game_sound_globals;
	long controller_index;
	byte *controller;

	if (!sound_available_inline() || owner==NONE)
	{
		return;
	}
	controller_index= sound_playback_controller_find(owner);
	if (controller_index==NONE)
	{
		return;
	}
	controller= sound_playback_controller_try_and_get(controller_index);
	if (controller)
	{
		sound_playback_controller_unknown_00219ed0(globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET]&1, controller, unknown);
	}
}

/* 0x00219ed0: a sound playback controller with something takes a flag, if
it is not the one that it has: for a sound with a place, it is given too a
real of each local player (what unknown_0012a9d0 says of the player and the
place); for any other, its four reals are 0 */
void __attribute__((stdcall)) sound_playback_controller_unknown_00219ed0(
	boolean unknown,
	byte *controller,
	const struct sound_source *source)
{
	if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)==NONE ||
		(byte)(controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]>>7)==unknown)
	{
		return;
	}
	if ((((const byte *)source)[SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)==_sound_source_datum_type_positioned)
	{
		long player_index;

		for (player_index= unknown_0014dde0(); player_index!=NONE; player_index= unknown_0014de10(player_index))
		{
			*(real *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET+player_index*4)=
				unknown_0012a9d0(player_index, (const real_point3d *)((const byte *)source+SOUND_SOURCE_POSITION_OFFSET));
		}
	}
	else
	{
		memset(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET, 0, UNKNOWN_0051EBD8_DATUM_UNKNOWNC_SIZE);
	}
	controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]=
		(controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]&UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|(byte)(unknown<<7);
}

static inline __attribute__((always_inline)) byte *sound_effect_get(
	long effect_index)
{
	return (byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
}

/* 0x0021d360: a sound effect is marked (the bits 3 and 4 of its flags),
unknown_0021d630 is given it with a 2, and then its bit 0 is set */
void sound_effect_unknown_0021d360(
	long effect_index)
{
	byte *effect= sound_effect_get(effect_index);

	effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= SOUND_EFFECT_DATUM_UNKNOWN_FLAGS;
	unknown_0021d630(effect_index, 2);
	effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= FLAG(_sound_effect_datum_unknown0_bit);
}

/* 0x0021d5a0: something of a sound effect, by its kind (the byte at 3). Of
the kind 0: whether the bit 4 of its flags is not set. Of the kind 1: TRUE
with any of the bits 3 and 4; if not, what sound_source_unknown_00127f10 says
of its sound, and the effect is marked if the sound is. FALSE for the rest */
boolean sound_effect_unknown_0021d5a0(
	long effect_index)
{
	byte *effect= sound_effect_get(effect_index);

	switch ((char)effect[SOUND_EFFECT_DATUM_KIND_OFFSET])
	{
	case 0:
		return !((effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]>>_sound_effect_datum_unknown4_bit)&1);
	case 1:
	{
		long source_index;
		const byte *source;
		word flags;
		boolean result;

		if (effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]&SOUND_EFFECT_DATUM_UNKNOWN_FLAGS)
		{
			return TRUE;
		}
		source_index= *(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET);
		source= sound_source_get(source_index);
		flags= 0;
		result= sound_source_unknown_00127f10(source_index,
			*(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_EFFECT_CALLBACKS_OFFSET), (byte *)&flags);
		if ((flags>>_sound_source_datum_unknown5_bit)&1)
		{
			effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= FLAG(_sound_effect_datum_unknown3_bit);
		}

		return result;
	}
	}

	return FALSE;
}

/* 0x0021da90: what a sound of a sound effect has told when it is let go,
if it is not that all are stopped: who the sound had to tell before the
effect is told, the bit 1 of the flags of the effect says whether the reason
was not 2, and then as sound_effect_unknown_0021d360 */
void __attribute__((stdcall)) sound_effect_source_deleted(
	long data,
	long source_index,
	long reason)
{
	const byte *source;
	const struct sound_source_callbacks *callbacks;
	byte *effect;
	long effect_index;

	if (reason==_sound_source_deleted_unknown10)
	{
		return;
	}
	source= sound_source_get(source_index);
	effect= sound_effect_get(*(const long *)(source+SOUND_SOURCE_DATUM_EFFECT_INDEX_OFFSET));
	callbacks= *(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_EFFECT_CALLBACKS_OFFSET);
	if (callbacks && callbacks->deleted)
	{
		callbacks->deleted(data, source_index, reason);
	}
	if (reason==_sound_source_deleted_unknown2)
	{
		effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]&= ~FLAG(_sound_effect_datum_unknown1_bit);
	}
	else
	{
		effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= FLAG(_sound_effect_datum_unknown1_bit);
	}
	effect_index= *(const long *)(source+SOUND_SOURCE_DATUM_EFFECT_INDEX_OFFSET);
	effect= sound_effect_get(effect_index);
	effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= SOUND_EFFECT_DATUM_UNKNOWN_FLAGS;
	unknown_0021d630(effect_index, 2);
	effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= FLAG(_sound_effect_datum_unknown0_bit);
}

/* 0x002197f0: a gain, from 0 to 1, in decibels: from -64 to 0 */
real sound_gain_to_decibels(
	real gain)
{
	real clamped;
	real decibels;

	if (0.0f>gain)
	{
		clamped= 0.0f;
	}
	else
	{
		clamped= 1.0f;
		if (!(gain>1.0f))
		{
			clamped= gain;
		}
	}
	decibels= SOUND_MINIMUM_DECIBELS;
	if (gain>0.0f)
	{
		long double exact= log10l(clamped)*(long double)20.0f;

		/* the least is looked for with all the precision of the x87 */
		if (!((long double)SOUND_MINIMUM_DECIBELS>exact))
		{
			decibels= (real)exact;
			if (decibels>0.0f)
			{
				decibels= 0.0f;
			}
		}
	}
	if (SOUND_MINIMUM_DECIBELS>decibels)
	{
		return SOUND_MINIMUM_DECIBELS;
	}
	if (decibels>0.0f)
	{
		return 0.0f;
	}

	return decibels;
}

/* 0x0021e230: one more pair for the console's sound, of those that say how
loud each of its outputs is: an output, and some decibels from -64 to 0 as
hundredths of a decibel from -6400 to 0. The pairs are 8 bytes each, and how
many there are is after eight of them */
void __attribute__((stdcall)) sound_mixbins_add(
	long *mixbins,
	long mixbin,
	real decibels)
{
	real fraction;

	mixbins[mixbins[SOUND_MIXBINS_COUNT_INDEX]*2]= mixbin;
	fraction= sound_interpolation_fraction(SOUND_MINIMUM_DECIBELS, 0.0f, decibels, TRUE);
	mixbins[mixbins[SOUND_MIXBINS_COUNT_INDEX]*2+1]= (long)(fraction*6400.0f-6400.0f);
	mixbins[SOUND_MIXBINS_COUNT_INDEX]++;
}

/* 0x0021e270: a number of 16 bits for the console's sound from a frequency:
0x8000 under 30, 0 over 8000, and between them the logarithm of twice the
sine of the part of half a turn that the frequency is of 48000, times
5909.28 */
long __attribute__((stdcall)) sound_frequency_to_filter_coefficient(
	real frequency)
{
	long double sine;
	real coefficient;

	if (30.0f>frequency)
	{
		return 0x8000;
	}
	if (frequency>8000.0f)
	{
		return 0;
	}
	sine= sinl((long double)frequency*(long double)2.0833333110203966e-05f*(long double)3.1415927410125732f);
	coefficient= (real)(logl(sine+sine)*(long double)5909.27880859375f);

	return (long)coefficient&0xFFFF;
}

/* 0x0021e2d0: a number of 16 bits for the console's sound from some
decibels that take away, up to 22.5: the gain that they are, times 32768 */
unsigned long __attribute__((stdcall)) sound_unknown_0021e2d0(
	real decibels)
{
	unsigned long result;

	if (decibels>22.5f)
	{
		decibels= 22.5f;
	}
	result= (unsigned long)(long long)(exp2l((long double)decibels*(long double)-0.115129254758358f*
		1.4426950408889634073599246810018921L)*(long double)32768.0f);

	return result>0xFFFF ? 0xFFFF : result;
}

/* 0x0021e330: a number for the console's sound from some decibels: the
gain that they are less one, times 8192; up to 0x7FFF, and for those under 0,
0xC000 less it, up to 0xDFFF */
long __attribute__((stdcall)) sound_unknown_0021e330(
	real decibels)
{
	long double gain= exp2l((long double)decibels*(long double)0.115129254758358f*1.4426950408889634073599246810018921L);
	real scaled= (real)((gain-(long double)1.0f)*(long double)8192.0f);
	long value= (long)scaled;
	long negative;

	if (value>=0)
	{
		return value>0x7FFF ? 0x7FFF : value;
	}
	negative= (long)(0xC000UL-(unsigned long)value);
	if (negative<0xC000)
	{
		return 0xC000;
	}

	return negative>0xDFFF ? 0xDFFF : negative;
}

/* 0x00219650: some decibels between two others, by a fraction and in one
of four ways: along the decibels, along the gains, along the gains with a
square root (which goes fast at first towards the louder one), and along
the gains slow at both ends. In any other way, the second ones */
real __attribute__((stdcall)) sound_decibels_interpolate(
	long kind,
	real from,
	real to,
	real fraction)
{
	real from_gain;
	real to_gain;
	real amount;

	switch (kind)
	{
	case _sound_interpolation_decibels:
		return (to-from)*fraction+from;
	case _sound_interpolation_gain:
		from_gain= (real)sound_decibels_to_gain(from);
		return sound_gain_to_decibels((real)((sound_decibels_to_gain(to)-(long double)from_gain)*(long double)fraction+
			(long double)from_gain));
	case _sound_interpolation_gain_square_root:
		from_gain= (real)sound_decibels_to_gain(from);
		to_gain= (real)sound_decibels_to_gain(to);
		if (to_gain>from_gain)
		{
			amount= (real)sqrtl(fraction);
		}
		else
		{
			amount= (real)((long double)1.0f-sqrtl((long double)1.0f-(long double)fraction));
		}
		return sound_gain_to_decibels((to_gain-from_gain)*amount+from_gain);
	case _sound_interpolation_gain_smooth:
	{
		real square;

		from_gain= (real)sound_decibels_to_gain(from);
		to_gain= (real)sound_decibels_to_gain(to);
		square= fraction*fraction;
		amount= square*3.0f-square*fraction*2.0f;
		if (0.0f>amount)
		{
			amount= 0.0f;
		}
		else if (amount>1.0f)
		{
			amount= 1.0f;
		}
		return sound_gain_to_decibels((to_gain-from_gain)*amount+from_gain);
	}
	}

	return to;
}

/* 0x002197b0: some decibels as they come up from silence, by a fraction
from 0 to 1 */
real __attribute__((stdcall)) sound_decibels_fade_in(
	long kind,
	real decibels,
	real fraction)
{
	if (0.0f>fraction)
	{
		fraction= 0.0f;
	}
	else if (fraction>1.0f)
	{
		fraction= 1.0f;
	}

	return sound_decibels_interpolate(kind, SOUND_MINIMUM_DECIBELS, decibels, fraction);
}

/* 0x0012a810: how loud a sound that fades is, in decibels, from -64 to 0;
0 for one that does not fade. Its two times, at 0xB4 and 0xB8, are from now
the first time, when either is NONE; the sound goes from one to the other:
towards its decibels at 0xB0 from silence if the second is not later, and
from them to 0 if it is. The name is a guess */
real sound_source_fade_decibels(
	long source_index)
{
	byte *source= sound_source_get(source_index);
	const byte *globals;
	long first;
	long second;
	long latest;
	real fraction;
	real from;
	real to;
	real decibels= 0.0f;

	if (!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>7)&1))
	{
		return 0.0f;
	}
	globals= game_sound_globals;
	if (*(const long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)==NONE ||
		*(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)==NONE)
	{
		*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)+= *(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)+1;
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)+= *(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)+1;
	}
	first= *(const long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET);
	second= *(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET);
	latest= first<=second ? second : first;
	fraction= sound_interpolation_fraction((real)(first-latest), (real)(second-latest),
		(real)(*(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)-latest), first<second);
	from= first<second ? *(const real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET) : SOUND_MINIMUM_DECIBELS;
	to= first>=second ? *(const real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET) : 0.0f;
	switch (*(const short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET))
	{
	case 0:
		decibels= sound_decibels_interpolate(_sound_interpolation_gain, from, to, fraction);
		break;
	case 1:
		decibels= sound_decibels_interpolate(_sound_interpolation_gain_square_root, from, to, fraction);
		break;
	}
	if (SOUND_MINIMUM_DECIBELS>decibels)
	{
		return SOUND_MINIMUM_DECIBELS;
	}
	if (decibels>0.0f)
	{
		return 0.0f;
	}

	return decibels;
}

/* the pitch of the recordings of a pitch range as they were made, in
hundredths of a semitone: of the parameters of the pitch ranges that the
sounds of a map share */
static inline __attribute__((always_inline)) short sound_pitch_range_natural_pitch(
	const struct sound_pitch_range *pitch_range)
{
	const byte *parameters= (const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown8[2].address;

	return *(const short *)(parameters+*(const short *)(pitch_range->unknown0+SOUND_PITCH_RANGE_PARAMETERS_INDEX_OFFSET)*
		SOUND_PITCH_RANGE_PARAMETERS_SIZE);
}

/* how many times faster a sound is played some hundredths of a semitone up:
1200 of them are twice as fast */
static inline __attribute__((always_inline)) long double sound_pitch_to_rate(
	long double pitch)
{
	return exp2l(pitch*(long double)0.0005776226753368974f*1.4426950408889634073599246810018921L);
}

/* 0x00218b80: how long a recording of a sound lasts at the most, in
thousandths of a second, from the number at 8 of the recording: played as
low as how the sound is played and its scale let it be */
long double __attribute__((stdcall)) sound_permutation_get_maximum_milliseconds(
	const struct sound_definition *sound,
	const struct sound_pitch_range *pitch_range,
	long unknown8)
{
	const struct sound_playback_parameters *playback= sound_definition_get_playback(sound);
	const short *scale_bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
	long double length= sound_pitch_to_rate(sound_pitch_range_natural_pitch(pitch_range));
	short playback_bend= playback->pitch_lower_bound<=playback->pitch_upper_bound ? playback->pitch_lower_bound :
		playback->pitch_upper_bound;
	short scale_bend= scale_bends[0]>scale_bends[1] ? scale_bends[1] : scale_bends[0];
	long bend= scale_bend+playback_bend;
	long double rate;

	length*= (long double)sound_unknown5_reals[(char)sound->unknown3[2]];
	length*= (long double)sound_unknown4_reals[(char)sound->unknown3[1]];
	length*= (long double)unknown8;
	length*= (long double)1000.0f;
	rate= sound_pitch_to_rate(bend);

	return length/(rate*(long double)sound_sample_rates[(char)sound->unknown3[0]]);
}

/* 0x00218ca0: how long a recording of a pitch range of a sound lasts at the
most, played at a pitch */
long double __attribute__((stdcall)) sound_get_maximum_milliseconds(
	long sound_index,
	long pitch_range_offset,
	long permutation_offset,
	real pitch)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const struct sound_definition *sound= sound_definition_get(sound_index);
	const struct sound_pitch_range *pitch_range=
		(const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset);
	const byte *permutation= (const byte *)(uintptr_t)gestalt->permutations.address+
		(pitch_range->first_permutation_index+permutation_offset)*(long)sizeof(struct sound_permutation);
	long double milliseconds= sound_permutation_get_maximum_milliseconds(sound, pitch_range,
		*(const long *)(permutation+SOUND_PERMUTATION_UNKNOWN8_OFFSET));

	return sound_pitch_to_rate((long double)pitch-(long double)sound_pitch_range_natural_pitch(pitch_range))*milliseconds;
}

/* 0x00219880: a number from four reals and a fraction: between the first
two by the fraction, plus the third, plus one at random from less the
fourth to the fourth. What the reals are of is not known */
long double __attribute__((stdcall)) sound_unknown_00219880(
	const real *unknown,
	unsigned long *seed,
	real fraction)
{
	real base= (unknown[1]-unknown[0])*fraction+unknown[0];
	long double range= unknown[3];
	long double random;

	*seed= *seed*1664525UL+1013904223UL;
	random= (long double)(*seed>>16)*(long double)(1.0f/65535.0f);

	return (range-(-range))*random+(-range)+(long double)unknown[2]+(long double)base;
}

/* 0x0044a150, 0x0044a16c and 0x0044a188: of the seven states of a looping
sound, most likely: two numbers of each, and where something of each is in
what the second function is given */
static const long looping_sound_unknown_0044a150[]= { 1, 1, 2, 5, 5, 1, 6 };
static const long looping_sound_unknown_0044a16c[]= { 3, 3, 2, 4, 4, 3, 6 };
static const long looping_sound_unknown_0044a188[]= { 0x14, 0x1C, 0x24, 0x40, 0x2C, 0x48, 0x34 };

/* 0x0021a050: with the bytes at 0xC and 0xD of something not the same:
whether what the second one says there is, in what the function is given, is
not NONE */
long __attribute__((stdcall)) looping_sound_unknown_0021a050(
	const byte *unknown,
	const byte *definition)
{
	char next= (char)unknown[LOOPING_SOUND_UNKNOWND_OFFSET];

	if (unknown[LOOPING_SOUND_UNKNOWNC_OFFSET]==unknown[LOOPING_SOUND_UNKNOWND_OFFSET])
	{
		return FALSE;
	}

	return *(const long *)(definition+looping_sound_unknown_0044a188[next]+4)!=NONE;
}

/* 0x0021a080: FALSE if the byte at 0xC of something is 0, 3 or 5 and the
one at 0xD is one of the two numbers of the first; TRUE if not */
boolean looping_sound_unknown_0021a080(
	const byte *unknown)
{
	char current= (char)unknown[LOOPING_SOUND_UNKNOWNC_OFFSET];
	boolean result= FALSE;

	if ((1UL<<(current&31))&0x29)
	{
		char next= (char)unknown[LOOPING_SOUND_UNKNOWND_OFFSET];

		if (looping_sound_unknown_0044a16c[current]==next || looping_sound_unknown_0044a150[current]==next)
		{
			result= TRUE;
		}
	}

	return !result;
}

/* 0x0021cc30: the byte at 4 of a pitch range of a sound, as
sound_pitch_range_get_unknown4 gives it; NONE if either of two flags is set */
long __attribute__((stdcall)) sound_unknown_0021cc30(
	const byte *flags,
	const struct sound_definition *sound,
	long pitch_range_offset)
{
	if (*flags&0x30)
	{
		return NONE;
	}

	return (short)sound_pitch_range_get_unknown4(
		(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset), sound);
}

/* 0x00125f70: a sound starts to play: what the game says of it is looked at
by unknown_00126c30 and unknown_00126ec0, and unknown_00126000 makes it.
Gives the sound that plays; NONE if it cannot, and then why, if it is asked:
2 if the second says no, and 3 if it could not be made. The name is a guess */
long __attribute__((stdcall)) sound_source_new(
	void *unknown,
	long *failure,
	long unknown2)
{
	long source_index= NONE;

	if (sound_available_inline())
	{
		long first;
		long second= unknown2;

		if (unknown_00126c30(unknown2, unknown, &first, failure))
		{
			if (unknown_00126ec0(unknown, &second))
			{
				if (failure)
				{
					*failure= _sound_source_new_failure_unknown2;
				}
			}
			else
			{
				source_index= unknown_00126000(unknown, first, unknown2, second);
				if (failure && source_index==NONE)
				{
					*failure= _sound_source_new_failure_unknown3;
				}
			}
		}
	}

	return source_index;
}

/* 0x001268e0: a sound that plays counts in the piece of the samples that it
is at, if it did not, and the piece is asked for */
void sound_source_hold_chunk(
	byte *source)
{
	if (!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_holds_chunk_bit)&1))
	{
		const struct sound_definition *sound;

		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_holds_chunk_bit);
		sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
		unknown_00218850(NONE,
			sound_get_chunk(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET],
				(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET],
				*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)),
			_sound_cache_request_unknown4);
	}
}

/* 0x00129380: each channel that plays and is not stopped for a while is
given, by unknown_00129790, a number of one of two tables: that of its slot,
or with no slot, that of its voice of the console's sound */
void sound_channels_unknown_00129380(
	const long *voice_values,
	const long *slot_values)
{
	long channel_index;

	for (channel_index= 0;
		channel_index<*(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		const struct sound_channel *channel= &sound_channels[(short)channel_index];

		if (channel->unknown0!=NONE && !channel->unknown9)
		{
			if (channel->unknownE!=NONE)
			{
				unknown_00129790(channel_index, slot_values[channel->unknownE]);
			}
			else if (channel->unknownC!=NONE)
			{
				unknown_00129790(channel_index, voice_values[channel->unknownC]);
			}
		}
	}
}

/* 0x0021f6d0: the sound of the places of the map is set: two pairs of
numbers and 0x30 bytes, kept in what the game has of the console's sound,
and then unknown_0021e3b0 with what is at 0x2AB4 of it */
void __attribute__((stdcall)) sound_console_set_environment(
	const long *second,
	const long *first,
	const void *unknown)
{
	byte *console= unknown_0051ebe4;
	long *numbers= (long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET);

	numbers[0]= first[0];
	numbers[1]= first[1];
	numbers[2]= second[0];
	numbers[3]= second[1];
	memcpy(&numbers[4], unknown, UNKNOWN_0051EBE4_ENVIRONMENT_UNKNOWN_SIZE);
	unknown_0021e3b0(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET));
}

/* 0x00221900: some seconds go by for the classes of sounds: each has three
reals of its own, where it goes, where it is and the seconds that it has
left to get there */
void sound_classes_update(
	real seconds)
{
	if (seconds>0.0f)
	{
		real *gain= sound_class_gains;
		long index;

		for (index= 0; index<NUMBER_OF_SOUND_CLASSES; index++, gain+= SOUND_CLASS_GAIN_REALS)
		{
			if (gain[2]>seconds)
			{
				gain[1]= (gain[0]-gain[1])*(seconds/gain[2])+gain[1];
				gain[2]= gain[2]-seconds;
			}
			else
			{
				*(long *)&gain[1]= *(const long *)&gain[0];
				gain[2]= 0.0f;
			}
		}
	}
}

/* 0x00126df0: a sound that plays fades out and another fades in, in some
seconds and in a way: either can be NONE. The name is a guess */
void __attribute__((stdcall)) sound_sources_cross_fade(
	long out_source_index,
	long in_source_index,
	short kind,
	real seconds)
{
	if (out_source_index!=NONE)
	{
		byte *source= sound_source_get(out_source_index);

		if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>7)&1)
		{
			*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(out_source_index);
		}
		else
		{
			*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= SOUND_MINIMUM_DECIBELS;
		}
		*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= kind;
		*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= NONE;
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= (long)(seconds*1000.0f);
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
	}
	if (in_source_index!=NONE)
	{
		byte *source= sound_source_get(in_source_index);

		*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(in_source_index);
		*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= kind;
		*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= (long)(seconds*1000.0f);
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
	}
}

/* 0x00127e20: how far a sound is from a listener, of what the game says of
it, by its type: 0 for the type 0. For the type 1, which has a place, the
square of the distance; or, if its place is a column (the byte at 2), that
of the distance on the ground, and the most that a real can be for a
listener that is not at the height of the column. For the rest, the length
of its three reals */
real sound_source_unknown_00127e20(
	const struct sound_source *source,
	long listener_index)
{
	const byte *bytes= (const byte *)source;
	const real *place= (const real *)(bytes+SOUND_SOURCE_POSITION_OFFSET);

	switch ((signed char)(bytes[SOUND_SOURCE_TYPE_OFFSET]<<4)>>4)
	{
	case 0:
		return 0.0f;
	case _sound_source_datum_type_positioned:
	{
		const struct sound_listener *listener=
			(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+listener_index;
		real height= listener->position.z-place[2];
		real limited;
		real y;
		real x;

		if (!(char)bytes[SOUND_SOURCE_POSITION_KIND_OFFSET])
		{
			y= listener->position.y-place[1];
			x= listener->position.x-place[0];

			return height*height+y*y+x*x;
		}
		limited= 0.0f;
		if (!(0.0f>height))
		{
			limited= height>*(const real *)(bytes+SOUND_SOURCE_HEIGHT_OFFSET) ? *(const real *)(bytes+SOUND_SOURCE_HEIGHT_OFFSET) :
				height;
		}
		if (limited!=height)
		{
			return SOUND_REAL_MAXIMUM;
		}
		y= place[1]-listener->position.y;
		x= place[0]-listener->position.x;

		return x*x+y*y;
	}
	}

	return (real)sqrtl((long double)place[0]*(long double)place[0]+(long double)place[1]*(long double)place[1]+
		(long double)place[2]*(long double)place[2]);
}

/* 0x00129f20: a channel takes a piece of the samples of a recording to play
it: its voice of the console's sound is given it, and it is asked for. It is
the first of the channel, if it had none, or the one that comes after; of
the one that came after, if there was one, the channel lets go. The name is
a guess */
void __attribute__((stdcall)) sound_channel_queue_chunk(
	short channel_index,
	const struct sound_permutation *permutation,
	short chunk)
{
	struct sound_channel *channel= &sound_channels[channel_index];
	const struct sound_chunk *queued= sound_permutation_chunk(permutation, chunk);

	if (channel->unknown20)
	{
		sound_chunk_release(sound_permutation_chunk((const struct sound_permutation *)(uintptr_t)channel->unknown20,
			channel->unknown1A));
		channel->unknown1A= NONE;
	}
	unknown_002ae500(sound_channel_voice(channel), queued);
	unknown_00218850(NONE, queued, _sound_cache_request_unknown4);
	if (channel->unknown1C)
	{
		channel->unknown1A= chunk;
		channel->unknown20= (long)(uintptr_t)permutation;
	}
	else
	{
		channel->unknown18= chunk;
		channel->unknown1C= (long)(uintptr_t)permutation;
		channel->unknown10= 0.0f;
	}
}

/* 0x0012ac20: how much of a sound is heard at a distance, from 0 to 1: all
of it up to the distance that it is heard at its loudest, and from there
less, by the distance and by how far the distance is on the way to that
from which it is not heard. The name is a guess */
real __attribute__((stdcall)) sound_source_distance_gain(
	long sound_index,
	const struct sound_source *source,
	real distance)
{
	real minimum_distance= sound_get_minimum_distance(source, sound_index);
	real fraction;

	if (!(minimum_distance>0.001f))
	{
		minimum_distance= 0.001f;
	}
	fraction= sound_interpolation_fraction(sound_get_maximum_distance(source, sound_index), minimum_distance, distance, FALSE);

	return minimum_distance/(minimum_distance>distance ? minimum_distance : distance)*fraction;
}

/* 0x00219cc0: each sound playback controller goes on to its next random
number */
void sound_playback_controllers_update(
	void)
{
	long controller_index;

	for (controller_index= data_next_index(unknown_0051ebd8, NONE);
		controller_index!=NONE;
		controller_index= data_next_index(unknown_0051ebd8, controller_index))
	{
		unsigned long *seed= (unsigned long *)((byte *)sound_playback_controller_get(controller_index)+
			UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET);

		*seed= *seed*1664525UL+1013904223UL;
	}
}

/* 0x00219d90: when the sound is disabled, and when it is enabled: each
sound playback controller with something and a count of 0 at 3 whose flag
there is as the function is told (set, to enable; not set, to disable) is
given to sound_console_unknown_0021f430, and to disable its flag is set */
void __attribute__((stdcall)) sound_playback_controllers_unknown_00219d90(
	boolean enable)
{
	long controller_index;

	for (controller_index= data_next_index(unknown_0051ebd8, NONE);
		controller_index!=NONE;
		controller_index= data_next_index(unknown_0051ebd8, controller_index))
	{
		byte *controller= sound_playback_controller_get(controller_index);
		byte count= controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET];

		if (((count&UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG)!=0)==(enable!=0) &&
			*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE &&
			!(count&UNKNOWN_0051EBD8_DATUM_COUNT_MASK))
		{
			sound_console_unknown_0021f430(controller_index);
			if (!enable)
			{
				controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]|= UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG;
			}
		}
	}
}

/* 0x0021a100: when the structure bsp changes: where in it the looping
sounds with a place are */
void looping_sounds_initialize_for_new_structure_bsp(
	void)
{
	struct data_array *data= looping_sounds;
	long looping_sound_index;

	for (looping_sound_index= data_next_index(data, NONE);
		looping_sound_index!=NONE;
		looping_sound_index= data_next_index(data, looping_sound_index))
	{
		byte *looping_sound= (byte *)data->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;

		if ((looping_sound[LOOPING_SOUND_DATUM_SOURCE_OFFSET+SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)==
			_sound_source_datum_type_positioned)
		{
			scenario_location_from_point((struct scenario_location *)(looping_sound+LOOPING_SOUND_DATUM_LOCATION_OFFSET),
				(const real_point3d *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+SOUND_SOURCE_POSITION_OFFSET));
		}
	}
}

/* 0x0021a1e0: all the looping sounds are let go */
void looping_sounds_delete_all(
	void)
{
	struct data_array *data= looping_sounds;

	if (data)
	{
		long looping_sound_index;

		for (looping_sound_index= data_next_index(data, NONE);
			looping_sound_index!=NONE;
			looping_sound_index= data_next_index(data, looping_sound_index))
		{
			looping_sound_delete(looping_sound_index);
		}
	}
}

/* 0x0021d4d0: all the sound effects are let go */
void sound_effects_delete_all(
	void)
{
	struct data_array *data= sound_effects;
	long effect_index;

	for (effect_index= data_next_index(data, NONE); effect_index!=NONE; effect_index= data_next_index(data, effect_index))
	{
		sound_effect_delete(effect_index);
	}
}

/* 0x0021b870: the sound that plays, of those of a track of a looping sound,
that is of a sound, and of a pitch range of it if one is given. The sounds
of a track are in a ring: each says which is next. NONE if there is none */
long __attribute__((stdcall)) looping_sound_find_source(
	long looping_sound_index,
	short track,
	long sound_index,
	long pitch_range)
{
	const byte *looping_sound= (const byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	long first= *(const long *)(looping_sound+track*LOOPING_SOUND_TRACK_SIZE+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET);
	long source_index= first;
	long count;

	for (count= 0; ; count++)
	{
		const byte *source;

		if (count>0 && source_index!=NONE)
		{
			source_index= *(const long *)(sound_source_get(source_index)+SOUND_SOURCE_DATUM_NEXT_OFFSET);
			if (source_index==first)
			{
				source_index= NONE;
			}
		}
		if (source_index==NONE)
		{
			return NONE;
		}
		source= sound_source_get(source_index);
		if (*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)==sound_index &&
			(pitch_range==NONE || (char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]==pitch_range))
		{
			return source_index;
		}
	}
}

/* 0x0021bc80: a sound that plays goes into the ring of a track of a looping
sound, after its first one; nothing if the track has none */
void looping_sound_add_source(
	long looping_sound_index,
	short track,
	long source_index)
{
	const byte *looping_sound= (const byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	const long *first= (const long *)(looping_sound+track*LOOPING_SOUND_TRACK_SIZE+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET);
	byte *source= sound_source_get(source_index);

	if (*first!=NONE)
	{
		byte *first_source= sound_source_get(*first);

		*(long *)(source+SOUND_SOURCE_DATUM_NEXT_OFFSET)= *(const long *)(first_source+SOUND_SOURCE_DATUM_NEXT_OFFSET);
		*(long *)(source+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET)= *first;
		*(long *)(first_source+SOUND_SOURCE_DATUM_NEXT_OFFSET)= source_index;
		*(long *)(sound_source_get(*(const long *)(source+SOUND_SOURCE_DATUM_NEXT_OFFSET))+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET)=
			source_index;
	}
}

/* 0x0021d390: what is done with each sound effect each time: those with the
bit 0 of their flags are let go, and of the rest, those that
sound_effect_unknown_0021d5a0 says are given to unknown_0021d630 with a 1 */
void sound_effects_update(
	void)
{
	struct data_array *data= sound_effects;
	long effect_index;

	for (effect_index= data_next_index(data, NONE); effect_index!=NONE; effect_index= data_next_index(data, effect_index))
	{
		if (sound_effect_get(effect_index)[SOUND_EFFECT_DATUM_FLAGS_OFFSET]&FLAG(_sound_effect_datum_unknown0_bit))
		{
			sound_effect_delete(effect_index);
		}
		else if (sound_effect_unknown_0021d5a0(effect_index))
		{
			unknown_0021d630(effect_index, 1);
		}
	}
}

/* 0x0021f360: what the game keeps of the sound is filled from what it has
of the console's sound: which voices are of each kind, which slots have no
voice that plays, and six numbers */
void __attribute__((stdcall)) sound_console_get_state(
	byte *state)
{
	const byte *console= unknown_0051ebe4;
	long index;

	memset(state, 0, SOUND_CONSOLE_STATE_SIZE);
	memset(state+GAME_SOUND_GLOBALS_SLOTS_VALID_OFFSET, 0xFF,
		(unsigned long)((*(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_OFFSET)+31)>>5)*4);
	for (index= 0; index<*(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET); index++)
	{
		const byte *voice= console+UNKNOWN_0051EBE4_VOICE_OFFSET+index*UNKNOWN_0051EBE4_VOICE_SIZE;
		long kind= (signed char)(voice[VOICE_STATE_OFFSET]<<5)>>5;

		((unsigned long *)(state+kind*SOUND_CONSOLE_STATE_KIND_SIZE))[index>>5]|= 1UL<<(index&31);
		if (voice[VOICE_SLOT_OFFSET]!=0xFF && (voice[VOICE_STATE_OFFSET]&VOICE_STATE_MASK))
		{
			long slot= (char)voice[VOICE_SLOT_OFFSET];

			((unsigned long *)(state+GAME_SOUND_GLOBALS_SLOTS_VALID_OFFSET))[slot>>5]&= ~(1UL<<(slot&31));
		}
	}
	for (index= 0; index<3; index++)
	{
		*(long *)(state+SOUND_CONSOLE_STATE_COUNTS_OFFSET+index*4)=
			*(const short *)(console+UNKNOWN_0051EBE4_COUNTS_OFFSET+index*2);
		*(long *)(state+SOUND_CONSOLE_STATE_COUNTS_OFFSET+(3+index)*4)=
			*(const short *)(console+UNKNOWN_0051EBE4_COUNTS_OFFSET+(3+index)*2);
	}
}

/* 0x0021f430: each voice of the console's sound that plays and is of a
sound playback controller is given to unknown_0037b822 with a 2; if there
was any, unknown_0037b657 is given what is at 0x2AB0 */
void __attribute__((stdcall)) sound_console_unknown_0021f430(
	long controller_index)
{
	byte *console= unknown_0051ebe4;
	boolean found= FALSE;
	long index;

	for (index= 0; index<*(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET); index++)
	{
		const byte *voice= console+UNKNOWN_0051EBE4_VOICE_OFFSET+index*UNKNOWN_0051EBE4_VOICE_SIZE;

		if (*(const long *)(voice+VOICE_CONTROLLER_INDEX_OFFSET)==controller_index &&
			(voice[VOICE_STATE_OFFSET]&VOICE_STATE_MASK))
		{
			unknown_0037b822(*(void *const *)(voice+VOICE_UNKNOWN24_OFFSET), 2);
			console= unknown_0051ebe4;
			found= TRUE;
		}
	}
	if (found)
	{
		unknown_0037b657(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET));
	}
}

/* 0x0021be20: the sounds that play of a ring that are of the kind 2 fade
out, in some seconds and in a way, as the first of sound_sources_cross_fade
does. The ring is that of a sound: nothing for NONE */
void __attribute__((stdcall)) sound_source_ring_fade_out(
	long first_source_index,
	short kind,
	real seconds)
{
	long source_index= first_source_index;
	long count;

	if (first_source_index==NONE)
	{
		return;
	}
	for (count= 0; ; count++)
	{
		if (count>0 && source_index!=NONE)
		{
			source_index= *(const long *)(sound_source_get(source_index)+SOUND_SOURCE_DATUM_NEXT_OFFSET);
			if (source_index==first_source_index)
			{
				source_index= NONE;
			}
		}
		if (source_index==NONE)
		{
			return;
		}
		if (sound_source_get(source_index)[SOUND_SOURCE_DATUM_KIND_OFFSET]==_sound_source_datum_kind_unknown2)
		{
			sound_sources_cross_fade(source_index, NONE, kind, seconds);
		}
	}
}

/* 0x0021bf00: what the game says of a looping sound, as of a sound: copied
out, if the looping sound has no sound playback controller or the flag at 3
of the one that it has is set. If the two numbers at 4 and 8 of the looping
sound are the same but for two bits, the type of the copy is 0. FALSE if
there is no such looping sound, or the flag is not set. Its second and
third arguments are not used */
boolean __attribute__((stdcall)) looping_sound_get_source(
	long looping_sound_index,
	long unused,
	long unused2,
	struct sound_source *source)
{
	const byte *looping_sound;
	long controller_index;

	(void)unused;
	(void)unused2;
	if (looping_sound_index==NONE)
	{
		return FALSE;
	}
	looping_sound= datum_try_and_get(looping_sounds, looping_sound_index);
	if (!looping_sound)
	{
		return FALSE;
	}
	controller_index= *(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET);
	if (controller_index!=NONE &&
		!(((const byte *)sound_playback_controller_get(controller_index))[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]&
		UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG))
	{
		return FALSE;
	}
	memcpy(source, looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET, LOOPING_SOUND_DATUM_SOURCE_SIZE);
	if (*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN8_OFFSET)==
		(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)|0x6000))
	{
		((byte *)source)[SOUND_SOURCE_TYPE_OFFSET]&= ~SOUND_SOURCE_DATUM_TYPE_MASK;
	}

	return TRUE;
}

/* 0x0021cf80: the looping sound that has two numbers at 4 and at 8; NONE if
there is none */
long __attribute__((stdcall)) looping_sound_find(
	long unknown4,
	long unknown8)
{
	struct data_array *data= looping_sounds;
	long looping_sound_index;

	for (looping_sound_index= data_next_index(data, NONE);
		looping_sound_index!=NONE;
		looping_sound_index= data_next_index(data, looping_sound_index))
	{
		const byte *looping_sound= (const byte *)data->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;

		if (*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)==unknown4 &&
			*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN8_OFFSET)==unknown8)
		{
			break;
		}
	}

	return looping_sound_index;
}

/* 0x0021dde0: when the structure bsp changes: where in it the point of each
sound voice is */
void sound_voices_initialize_for_new_structure_bsp(
	void)
{
	struct data_array *data= sound_voices;
	long voice_index;

	for (voice_index= data_next_index(data, NONE); voice_index!=NONE; voice_index= data_next_index(data, voice_index))
	{
		byte *voice= (byte *)data->data+(voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE;

		scenario_location_from_point((struct scenario_location *)(voice+SOUND_VOICE_DATUM_LOCATION_OFFSET),
			(const real_point3d *)(voice+SOUND_VOICE_DATUM_UNKNOWN48_OFFSET));
	}
}

/* 0x0021f8a0: a voice of the console's sound starts with what something
says: its sound playback controller, and its slot, whose stream it is given.
What the functions of the console that it calls do is not known */
void __attribute__((stdcall)) sound_console_voice_start(
	const byte *unknown,
	long voice_index)
{
	byte *voice= unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_OFFSET+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE;
	long controller_index;
	short slot;

	voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown4_bit);
	controller_index= *(const long *)(unknown+SOUND_VOICE_START_CONTROLLER_INDEX_OFFSET);
	*(long *)(voice+VOICE_CONTROLLER_INDEX_OFFSET)= controller_index;
	*(long *)(voice+VOICE_UNKNOWN2C_OFFSET)= 0;
	unknown_0037b822(*(void **)(voice+VOICE_UNKNOWN24_OFFSET), controller_index!=NONE);
	unknown_002ae7a0(voice, 0.0f, 0.1f);
	slot= *(const short *)(unknown+SOUND_VOICE_START_SLOT_OFFSET);
	if (slot!=NONE)
	{
		unknown_0037c705(*(void **)(voice+VOICE_UNKNOWN24_OFFSET),
			*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_SLOT_STREAM_OFFSET+slot*UNKNOWN_0051EBE4_SLOT_SIZE));
		voice[VOICE_SLOT_OFFSET]= unknown[SOUND_VOICE_START_SLOT_OFFSET];
	}
	else
	{
		unknown_0037c705(*(void **)(voice+VOICE_UNKNOWN24_OFFSET), NULL);
		voice[VOICE_SLOT_OFFSET]= 0xFF;
	}
	unknown_002201f0(voice_index, 1);
	voice[VOICE_FLAGS_OFFSET]&= ~FLAG(_voice_unknown0_bit);
	unknown_0021fa80(voice_index, 0, 0);
	voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown0_bit);
}

/* 0x00221da0: TRUE with no listener. For a sound with a place whose first
flag is not set: TRUE if its cluster or that of the listener is NONE; FALSE
if they are in the same leaf or cluster; and if not, whether what
unknown_00249d60 says of the two clusters, times a real, is 256 or more.
FALSE for the rest. What it means is not known */
boolean __attribute__((stdcall)) sound_source_unknown_00221da0(
	long listener_index,
	const struct sound_source *source,
	real unknown)
{
	const byte *bytes= (const byte *)source;
	const struct sound_listener *listener;
	short cluster_index;

	if (listener_index==NONE)
	{
		return TRUE;
	}
	if ((bytes[0]&1) || (bytes[SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)!=_sound_source_datum_type_positioned)
	{
		return FALSE;
	}
	listener= (const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+listener_index;
	cluster_index= *(const short *)(bytes+SOUND_SOURCE_LOCATION_OFFSET+4);
	if (cluster_index==NONE || listener->cluster_index==NONE)
	{
		return TRUE;
	}
	if (listener->leaf_index==*(const long *)(bytes+SOUND_SOURCE_LOCATION_OFFSET) || listener->cluster_index==cluster_index)
	{
		return FALSE;
	}

	return !(256.0f>unknown_00249d60(listener->cluster_index, cluster_index, global_structure_bsp)*unknown);
}

/* 0x0021dd30: what a tag says of a sound effect: the elements of a block of
what unknown_0018d090 gives for the tag and a number; whether the first has
anything at 8. FALSE if there is no such block. What unknown_0018d170 gives
is only looked at when the number is NONE, and it changes nothing */
boolean __attribute__((stdcall)) sound_effect_definition_get(
	long tag_index,
	long unknown,
	const byte **definition)
{
	const byte *first= unknown_0018d090(tag_index, unknown);
	const byte *second= unknown_0018d170(tag_index);
	const byte *elements;

	if (!(first && *(const long *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)))
	{
		if (unknown!=NONE || !second || !*(const long *)(second+SOUND_EFFECT_COLLECTION_COUNT_OFFSET))
		{
			return FALSE;
		}
	}
	if (*(const long *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)<=0)
	{
		return FALSE;
	}
	elements= *(const byte *const *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET+4);
	if (!elements)
	{
		return FALSE;
	}
	*definition= elements;

	return *(const long *)(elements+SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET)>0;
}

/* 0x0021d2c0: a new sound effect, of what a number says of it, with a real
and a number of 16 bits. NONE if there is nothing for it, or no room */
long __attribute__((stdcall)) sound_effect_new(
	long unknown,
	real unknownC,
	short unknown6)
{
	const byte *definition= NULL;
	long effect_index= NONE;
	byte *effect;

	if (!sound_effect_definition_get(NONE, unknown, &definition))
	{
		return NONE;
	}
	if (*(const long *)(definition+SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET))
	{
		effect_index= datum_new(sound_effects);
		if (effect_index==NONE)
		{
			return NONE;
		}
		effect= sound_effect_get(effect_index);
		*(const byte **)(effect+SOUND_EFFECT_DATUM_DEFINITION_OFFSET)= definition;
		*(long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)= NONE;
	}
	if (effect_index==NONE)
	{
		return NONE;
	}
	effect= sound_effect_get(effect_index);
	effect[SOUND_EFFECT_DATUM_KIND_OFFSET]= 0;
	*(real *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET)= unknownC;
	*(short *)(effect+SOUND_EFFECT_DATUM_UNKNOWN6_OFFSET)= unknown6;
	unknown_0021d630(effect_index, 0);

	return effect_index;
}

/* 0x0021db80: what the game says of the sound of a sound effect, by the
kind of the effect. Of the kind 0, which has no sound: no type, and the
number of the effect. Of the kind 1: what its sound that plays says, with
the distances that it is heard at, if its flag 8 asks for them. Of the kind
2: what unknown_0018cbc0 gives */
void sound_effect_get_source(
	long effect_index,
	struct sound_source *source)
{
	const byte *effect= sound_effect_get(effect_index);
	byte *bytes= (byte *)source;

	switch ((char)effect[SOUND_EFFECT_DATUM_KIND_OFFSET])
	{
	case 0:
		bytes[SOUND_SOURCE_TYPE_OFFSET]= 0;
		*(long *)(bytes+4)= *(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET);
		*(long *)(bytes+8)= 0;
		break;
	case 1:
	{
		const byte *playing= sound_source_get(*(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET));
		const struct sound_source *said= (const struct sound_source *)(playing+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET);

		if (source!=said)
		{
			memcpy(source, said, LOOPING_SOUND_DATUM_SOURCE_SIZE);
			if (bytes[1]&1)
			{
				real distance= sound_get_minimum_distance(said, *(const long *)(playing+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));

				bytes[0]|= 4;
				source->minimum_distance= distance;
				distance= sound_get_maximum_distance(said, *(const long *)(playing+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
				*(word *)bytes|= 0x208;
				source->maximum_distance= distance;
			}
		}
		break;
	}
	case 2:
		unknown_0018cbc0(*(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET), source);
		break;
	}
}

/* 0x002186b0: the cache of the sounds is let go: the array that counts who
is at each piece of the samples, and the cache itself, which what made it
frees */
void sound_cache_dispose(
	void)
{
	struct lruv_cache *cache;

	data_dispose(unknown_00502104);
	cache= sound_lruv_cache;
	cache->allocation->functions->deallocate(cache->allocation, cache);
	sound_cache_base= 0;
}

/* 0x00219070: which recording of a pitch range of a sound is played now.
The one that was set aside to be next (the byte at 5 of the pitch range), if
there is one; if not, what unknown_00219110 chooses, with the bits of those
that were played, the one that was played last and the seed of the game's
random numbers. It is kept as the last one */
short __attribute__((stdcall)) sound_choose_permutation(
	const struct sound_definition *sound,
	short pitch_range_offset)
{
	struct sound_pitch_range *pitch_range=
		(struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset);
	long permutation;

	if ((short)sound_pitch_range_get_unknown5(pitch_range, sound)!=NONE)
	{
		permutation= sound_pitch_range_get_unknown5(pitch_range, sound);
		sound_pitch_range_set_unknown5(pitch_range, sound, NONE);
	}
	else
	{
		unsigned long played= sound_pitch_range_get_played_bits(pitch_range, sound);

		permutation= unknown_00219110(sound, pitch_range_offset, &played, sound_pitch_range_get_unknown4(pitch_range, sound),
			&global_random_seed_reference[1], 0, 0);
		sound_pitch_range_set_played_bits(pitch_range, sound, played);
	}
	sound_pitch_range_set_unknown4(pitch_range, sound, (char)permutation);

	return (short)permutation;
}

/* 0x0021b910: the sound that plays of a track of a looping sound that is of
a sound and a pitch range; if there is none, unknown_0021b940 makes it */
long __attribute__((stdcall)) looping_sound_get_or_new_source(
	long sound_index,
	short track,
	long pitch_range,
	long looping_sound_index,
	long unknown,
	long unknown2)
{
	long source_index= looping_sound_find_source(looping_sound_index, track, sound_index, pitch_range);

	if (source_index==NONE)
	{
		source_index= unknown_0021b940(looping_sound_index, sound_index, pitch_range, track, unknown, unknown2);
	}

	return source_index;
}

/* 0x00218a10: the samples of a piece of a recording start to be read from
the map into the cache of the sounds: a block of the cache is made for
them, and the element of unknown_00502104 that goes with the block says of
which piece it is and whether the read is done. Nothing if there is no room.
The second argument is kept at 8 of the element */
void __attribute__((stdcall)) sound_cache_chunk_load(
	struct sound_chunk *chunk,
	long unknown)
{
	byte *bytes= (byte *)chunk;
	long block_index= lruv_block_new(sound_lruv_cache, *(const long *)(bytes+SOUND_CHUNK_SIZE_OFFSET)&SOUND_CHUNK_SIZE_MASK, 0);
	const struct lruv_cache *cache;
	long address;
	byte *datum;
	long size;

	if (block_index==NONE)
	{
		return;
	}
	cache= sound_lruv_cache;
	address= (long)((unsigned long)((const struct lruv_cache_block *)cache->blocks->data)[block_index&0xFFFF].first_page_index<<
		(cache->page_size_bits&31))+sound_cache_base;
	datum_new_at_index(unknown_00502104, block_index);
	datum= (byte *)unknown_00502104->data+(block_index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;
	*(long *)(bytes+SOUND_CHUNK_CACHE_INDEX_OFFSET)= block_index;
	*(long *)(datum+UNKNOWN_00502104_DATUM_UNKNOWN8_OFFSET)= unknown;
	*(struct sound_chunk **)(datum+UNKNOWN_00502104_DATUM_CHUNK_OFFSET)= chunk;
	datum[UNKNOWN_00502104_DATUM_COUNT_OFFSET]= 0;
	datum[UNKNOWN_00502104_DATUM_USES_OFFSET]= 0;
	datum[UNKNOWN_00502104_DATUM_UNKNOWN3_OFFSET]= 0;
	size= *(const long *)(bytes+SOUND_CHUNK_SIZE_OFFSET)&SOUND_CHUNK_SIZE_MASK;
	if (size&(SOUND_CACHE_READ_UNIT-1))
	{
		size= (size|(SOUND_CACHE_READ_UNIT-1))+1;
	}
	cache_file_read_async(*(const uint32_t *)bytes, size, (void *)(uintptr_t)address, NULL,
		datum+UNKNOWN_00502104_DATUM_DONE_OFFSET, _sound_cache_read_category, _sound_cache_read_priority);
}

/* 0x00218e50: a byte, as a real from 0 to 1, of what the sounds of a map
have more of a sound (its number at 0xE says which): that of a recording of
a pitch range, at a place in the bytes of the recording. 0 if the recording
is not of the pitch range, or has no bytes. What the bytes are is not known */
real __attribute__((stdcall)) sound_unknown_00218e50(
	long sound_index,
	short permutation,
	short pitch_range_offset,
	short index)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const byte *sound= (const byte *)sound_definition_get(sound_index);
	const struct sound_pitch_range *pitch_range=
		(const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address+
		(*(const short *)(sound+8)+pitch_range_offset);
	long clamped;
	const byte *extra;
	const byte *entry;
	long count;
	long clamped_index;

	if (permutation<0)
	{
		clamped= 0;
	}
	else
	{
		clamped= pitch_range->permutation_count-1;
		if (permutation<=clamped)
		{
			clamped= permutation;
		}
	}
	if (clamped!=permutation)
	{
		return 0.0f;
	}
	extra= *(const byte *const *)(*(const byte *const *)((const byte *)gestalt+SOUND_GESTALT_EXTRA_INFOS_OFFSET+4)+
		*(const short *)(sound+SOUND_DEFINITION_EXTRA_INFO_INDEX_OFFSET)*SOUND_EXTRA_INFO_SIZE+4);
	entry= *(const byte *const *)(extra+0xC)+
		(char)((const byte *)(uintptr_t)gestalt->permutations.address)[
			(pitch_range->first_permutation_index+permutation)*(long)sizeof(struct sound_permutation)+SOUND_PERMUTATION_UNKNOWN5_OFFSET]*
		SOUND_EXTRA_INFO_ENTRY_SIZE;
	count= *(const long *)(entry+4);
	if (count<=0)
	{
		return 0.0f;
	}
	if (index<0)
	{
		clamped_index= 0;
	}
	else
	{
		clamped_index= count-1;
		if (!(index>clamped_index))
		{
			clamped_index= index;
		}
	}

	return (real)(long)(*(const byte *const *)(extra+4))[(short)clamped_index+*(const long *)entry]*(1.0f/255.0f);
}

/* whether a track of a looping sound keeps for itself the bits of the
recordings that were played: with either of two flags, and a sound playback
controller */
static inline __attribute__((always_inline)) boolean looping_sound_track_keeps_played_bits(
	const byte *flags,
	const byte *looping_sound)
{
	return (*flags&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS) &&
		*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET)!=NONE;
}

/* 0x0021cb90: the bits of the recordings that were played of a track of a
looping sound: those that the track keeps, which can be cleared as they are
read, or those of a pitch range of a sound */
unsigned long __attribute__((stdcall)) looping_sound_track_get_played_bits(
	long track,
	const struct sound_definition *sound,
	byte *looping_sound,
	const byte *flags,
	short pitch_range_offset,
	boolean clear)
{
	if (looping_sound_track_keeps_played_bits(flags, looping_sound))
	{
		unsigned long *played= (unsigned long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_PLAYED_OFFSET+
			track*LOOPING_SOUND_TRACK_SIZE);
		unsigned long bits= *played;

		if (clear)
		{
			*played= 0;
		}

		return bits;
	}

	return sound_pitch_range_get_played_bits(
		(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset), sound);
}

/* 0x0021cbe0: the bits of the recordings that were played of a track of a
looping sound are set: those that the track keeps, or those of a pitch range
of a sound, which is told too which was played last */
void __attribute__((stdcall)) looping_sound_track_set_played_bits(
	long track,
	unsigned long bits,
	byte *looping_sound,
	const struct sound_definition *sound,
	const byte *flags,
	short pitch_range_offset,
	char last_permutation)
{
	struct sound_pitch_range *pitch_range;

	if (looping_sound_track_keeps_played_bits(flags, looping_sound))
	{
		*(unsigned long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_PLAYED_OFFSET+track*LOOPING_SOUND_TRACK_SIZE)= bits;
		return;
	}
	pitch_range= (struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset);
	sound_pitch_range_set_played_bits(pitch_range, sound, bits);
	sound_pitch_range_set_unknown4(pitch_range, sound, last_permutation);
}
