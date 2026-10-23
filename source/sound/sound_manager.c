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
#include "memory/physical_memory.h"
#include "objects/objects.h"
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

	/* what sound_cache_chunk_request is asked, and what it answers */
	_sound_cache_request_unknown2= 2,
	_sound_cache_result_unknown0_bit= 0,
	_sound_cache_result_unknown1_bit= 1,
	_sound_cache_result_held_bit= 2,
	_sound_cache_request_wait_bit= 0,
	_sound_cache_request_load_bit= 1,
	_sound_cache_request_hold_bit= 2,

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
	/* three gains of the sound of the console */
	UNKNOWN_0051EBE4_GAINS_OFFSET= 0x2ABC,
	SOUND_MINIMUM_MILLIBELS= -6400,
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
	/* four gains at the end of a looping sound, the flags its number at 8 has
	with its tag when it answers how its sounds are heard, and where the four
	decibels go in what is asked, with their flag */
	LOOPING_SOUND_DATUM_GAINS_OFFSET= 0xC4,
	NUMBER_OF_LOOPING_SOUND_GAINS= 4,
	LOOPING_SOUND_UNKNOWN8_OWN_FLAGS= 0x6000,
	SOUND_HEARD_DECIBELS_OFFSET= 0x28,
	_sound_heard_decibels_bit= 4,
	/* how many sounds of a track have a channel, and where the bytes for who
	is told of a sound of a track say which track */
	LOOPING_SOUND_TRACK_CHANNELS_OFFSET= 0xE,
	LOOPING_SOUND_TOLD_TRACK_OFFSET= 0x4,
	LOOPING_SOUND_TOLD_COUNTED_OFFSET= 0x6,
	LOOPING_SOUND_TRACK_PITCH_OFFSET= 0x10,
	LOOPING_SOUND_DEFINITION_TRACK_GAIN_OFFSET= 0x8,
	LOOPING_SOUND_DEFINITION_TRACK_UNKNOWN3C_OFFSET= 0x3C,
	LOOPING_SOUND_DEFINITION_MAXIMUM_DISTANCE_OFFSET= 0x10,
	LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET= 0x4,
	LOOPING_SOUND_DEFINITION_TRACK_FADE_IN_OFFSET= 0xC,
	LOOPING_SOUND_DEFINITION_TRACK_FADE_OUT_OFFSET= 0x10,
	LOOPING_SOUND_DEFINITION_TRACK_ALTERNATE_CROSS_FADE_OFFSET= 0x50,
	LOOPING_SOUND_DEFINITION_TRACK_ALTERNATE_FADE_OUT_OFFSET= 0x54,
	MAXIMUM_SOUND_PITCH_RANGES= 9,
	_sound_source_deleted_out_of_pitch_range= 8,
	SOUND_SOURCE_DATUM_TRACK_SEED_OFFSET= 0x64,
	SOUND_SOURCE_DATUM_TRACK_PLAYED_OFFSET= 0x68,
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
	/* more of a sound voice */
	SOUND_VOICE_DATUM_CLASS_OFFSET= 0x4,
	SOUND_VOICE_DATUM_UNKNOWN8_OFFSET= 0x8,
	SOUND_VOICE_DATUM_CALLBACK_DATA_OFFSET= 0xC,
	SOUND_VOICE_DATUM_CALLBACKS_OFFSET= 0x10,
	SOUND_VOICE_DATUM_UNKNOWN14_OFFSET= 0x14,
	/* in a recording: how often it is passed over when one is chosen */
	SOUND_PERMUTATION_SKIP_FRACTION_OFFSET= 0x2,
	/* two flags of what the game says of a sound: with either, the sound
	is heard however far it is */
	_sound_source_unknown8_bit= 8,
	_sound_source_unknown11_bit= 11,
	/* of a sound that plays: something that says which goes before, and
	its listener; of a class of sounds: how long a sound of it plays before
	its channel can be taken, and which class goes before */
	SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET= 0xA0,
	SOUND_SOURCE_DATUM_LISTENER_OFFSET= 0xA1,
	SOUND_CLASS_UNKNOWN4_OFFSET= 0x4,
	SOUND_CLASS_PRIORITY_OFFSET= 0xC,
	/* why a sound is let go: another took its channel */
	_sound_source_deleted_channel_taken= 9,
	/* more of why a sound is let go, or gets no channel */
	_sound_source_deleted_too_many_instances= 5,
	_sound_source_deleted_too_many_sounds= 6,
	_sound_source_deleted_replaced= 13,
	/* in what the game gives to start a sound: what it says of the sound,
	and in that, a scale from 0 to 1 and some decibels */
	SOUND_REQUEST_SOURCE_OFFSET= 0xC,
	SOUND_SOURCE_SCALE_OFFSET= 0x4,
	SOUND_SOURCE_DECIBELS_OFFSET= 0x8,
	/* why a sound does not start */
	_sound_start_refused_skipped= 1,
	_sound_start_refused_not_heard= 4,
	_sound_start_refused_unknown5= 5,
	/* a promotion, of those that the sounds of a map share: its rules, a
	time of each, the rule that it is at, when it was last looked at, and up
	to when the sound does not play; and of a rule */
	SOUND_PROMOTION_TIMER_COUNT_OFFSET= 0x8,
	SOUND_PROMOTION_RULE_INDEX_OFFSET= 0x10,
	SOUND_PROMOTION_LAST_TIME_OFFSET= 0x14,
	SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET= 0x18,
	SOUND_PROMOTION_RULE_COUNT_OFFSET= 0x2,
	SOUND_PROMOTION_RULE_SECONDS_OFFSET= 0x4,
	SOUND_PROMOTION_RULE_THRESHOLD_OFFSET= 0x8,
	SOUND_PROMOTION_RULE_STEP_OFFSET= 0xC,
	/* what the game gives to start a sound: its flags say which of the rest
	there is */
	_sound_request_unknown1_bit= 1,
	_sound_request_callback_data_bit= 3,
	_sound_request_controller_bit= 4,
	_sound_request_callbacks_bit= 5,
	_sound_request_unknown6_bit= 6,
	_sound_request_gain_bit= 7,
	_sound_request_unknown8_bit= 8,
	_sound_request_unknown9_bit= 9,
	_sound_request_permutation_bit= 10,
	SOUND_REQUEST_UNKNOWN4_OFFSET= 0x4,
	SOUND_REQUEST_UNKNOWN8_OFFSET= 0x8,
	SOUND_REQUEST_CALLBACK_DATA_OFFSET= 0x50,
	SOUND_REQUEST_CONTROLLER_OWNER_OFFSET= 0x54,
	SOUND_REQUEST_CALLBACKS_OFFSET= 0x58,
	SOUND_REQUEST_CALLBACK_BYTES_OFFSET= 0x5C,
	SOUND_REQUEST_CALLBACK_BYTES_SIZE= 0x30,
	SOUND_REQUEST_CALLBACK_SIZE_OFFSET= 0x8C,
	SOUND_REQUEST_UNKNOWN8E_OFFSET= 0x8E,
	SOUND_REQUEST_UNKNOWN90_OFFSET= 0x90,
	SOUND_REQUEST_GAIN_OFFSET= 0x94,
	SOUND_REQUEST_UNKNOWN98_OFFSET= 0x98,
	SOUND_REQUEST_PITCH_RANGE_OFFSET= 0x9C,
	SOUND_REQUEST_PERMUTATION_OFFSET= 0x9E,
	/* more of a sound that plays: its pitch and its gain, at random, and
	the seed that they left */
	SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET= 0x6,
	SOUND_SOURCE_DATUM_UNKNOWN8_OFFSET= 0x8,
	SOUND_SOURCE_DATUM_PITCH_OFFSET= 0x90,
	SOUND_SOURCE_DATUM_GAIN_OFFSET= 0x94,
	SOUND_SOURCE_DATUM_SEED_OFFSET= 0x98,
	SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET= 0xA2,
	SOUND_SOURCE_DATUM_UNKNOWNA4_OFFSET= 0xA4,
	SOUND_SOURCE_DELAY_MINIMUM= 100,
	_sound_definition_unknown3_bit= 3,
	_sound_class_internal_unknown2_bit= 2,
	/* in a tag that sound_request_unknown_002226b0 reads: a block */
	SOUND_ENVIRONMENT_TAG_BLOCK_OFFSET= 0x14,
	/* in the globals of the sound: the thousandths of a second that are
	left before the sound goes down when the game is paused */
	GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET= 0x1E8,
	/* in the globals of the game: the kind of game, whether one is going on
	and whether it is paused */
	MAIN_GAME_GLOBALS_MODE_OFFSET= 0x8,
	MAIN_GAME_GLOBALS_ACTIVE_OFFSET= 0x1120,
	MAIN_GAME_GLOBALS_PAUSED_OFFSET= 0x1121,
	/* of a sound that plays: its decibels, of what the game says of it; and
	how many bytes what sound_channel_compute_parameters works out of a channel is */
	SOUND_SOURCE_DATUM_DECIBELS_OFFSET= 0x20,
	SOUND_CHANNEL_PARAMETERS_SIZE= 0x38,
	_sound_source_deleted_faded_out= 4,
	/* a flag of a sound that plays, at 6 of it: it plays with a sound voice */
	_sound_source_datum_voice_bit= 1,
	_sound_source_deleted_no_voice= 7,
	/* why a sound that plays is let go: its samples cannot be asked for.
	A flag of it, and a number of its class */
	_sound_source_deleted_no_data= 14,
	/* more of why: its promotion keeps it from playing, and what
	sound_source_unknown_00127fc0 says; two flags more of a sound that
	plays, one of what the game says of it, and in how long a sound that
	no one hears fades out */
	_sound_source_deleted_promotion= 15,
	_sound_source_deleted_unknown3= 3,
	_sound_source_datum_unknown6_bit= 6,
	_sound_source_datum_unknown9_bit= 9,
	_sound_source_unknown0_bit= 0,
	SOUND_UNHEARD_FADE_OUT_TIME= 2000,
	/* what sound_unknown_00222930 fills, and where it takes it from: a
	block of names of a tag, and a block of an element of what
	unknown_0018d090 gives */
	SOUND_VOICE_ENVIRONMENT_NUMBER_OFFSET= 0x2,
	SOUND_VOICE_ENVIRONMENT_UNKNOWN4_OFFSET= 0x4,
	SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET= 0x8,
	SOUND_VOICE_ENVIRONMENT_UNKNOWNC_OFFSET= 0xC,
	SOUND_ENVIRONMENT_TAG_NAMES_OFFSET= 0xC,
	SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET= 0x20,
	/* of the list that sound_unknown_002225a0 makes, and of what it makes
	it from */
	SOUND_EFFECT_ELEMENT_SIZE= 0x18,
	SOUND_EFFECT_OVERRIDE_SIZE= 0x1C,
	SOUND_EFFECT_OVERRIDES_MASK_OFFSET= 0x10,
	SOUND_EFFECT_ENTRY_SIZE= 0xC,
	SOUND_EFFECT_ENTRY_BYTES_OFFSET= 0xA,
	SOUND_EFFECT_NAME_FOUR_TIMES= 0x11000138,
	SOUND_EFFECT_NAME_KIND10= 0x0D00012E,
	/* of a sound of the definition of a sound effect */
	SOUND_EFFECT_SOUND_SIZE= 0x10,
	SOUND_EFFECT_SOUND_GAIN_OFFSET= 0x8,
	SOUND_EFFECT_SOUND_FLAGS_OFFSET= 0xC,
	_sound_effect_sound_unknown0_bit= 0,
	_sound_effect_sound_unknown1_bit= 1,
	_sound_effect_sound_unknown3_bit= 3,
	_sound_effect_sound_unknown4_bit= 4,
	_sound_effect_sound_unknown5_bit= 5,
	UNKNOWN_004ED28C_DATUM_SIZE= 0x18,
	SOUND_REQUEST_SIZE= 0xA0,
	NUMBER_OF_SOUND_CONSOLE_FUNCTION_STREAMS= 2,
	SOUND_CONSOLE_FUNCTION_STREAM_SIZE= 1000,
	SOUND_EFFECT_NAME_KIND4= 0x1300012D,
	/* of what an element has in its place: the functions (of 0x10 bytes: two
	shorts that say which reals of the sound it is of, its data and a real),
	the reals, a byte for each number that it has (bit 0: a function; the
	rest: which) and the bits of the numbers that it has */
	SOUND_EFFECT_OVERRIDE_FUNCTIONS_OFFSET= 0x4,
	SOUND_EFFECT_OVERRIDE_REALS_OFFSET= 0xC,
	SOUND_EFFECT_OVERRIDE_WHICH_OFFSET= 0x14,
	SOUND_EFFECT_OVERRIDE_MASK_OFFSET= 0x18,
	SOUND_EFFECT_FUNCTION_SIZE= 0x10,
	SOUND_EFFECT_FUNCTION_DATA_OFFSET= 0x4,
	SOUND_EFFECT_FUNCTION_SCALE_OFFSET= 0xC,
	_sound_effect_input_scaled= 1,
	_sound_effect_input_directional= 3,
	SOUND_EFFECT_INPUTS_VECTOR_INDEX= 4,
	/* three blocks more of what unknown_0018d090 gives, and what
	sound_unknown_00222770 fills */
	SOUND_EFFECT_COLLECTION_UNKNOWN14_OFFSET= 0x14,
	SOUND_EFFECT_COLLECTION_UNKNOWN1C_OFFSET= 0x1C,
	SOUND_EFFECT_COLLECTION_UNKNOWN24_OFFSET= 0x24,
	SOUND_VOICE_EFFECTS_UNKNOWN8_OFFSET= 0x8,
	SOUND_VOICE_EFFECTS_UNKNOWNC_OFFSET= 0xC,
	SOUND_VOICE_EFFECTS_UNKNOWN10_OFFSET= 0x10,
	SOUND_VOICE_EFFECTS_UNKNOWN14_OFFSET= 0x14,
	SOUND_VOICE_EFFECTS_UNKNOWN18_OFFSET= 0x18,
	SOUND_VOICE_EFFECTS_LIST_OFFSET= 0x1C,
	SOUND_VOICE_EFFECTS_LIST_SIZE_OFFSET= 0x41C,
	SOUND_EFFECT_LIST_SIZE= 0x400,
	/* in what the game gives to start a sound of a sound effect, at the end
	of the bytes for who is told: the effect, and who was to be told */
	SOUND_REQUEST_EFFECT_INDEX_OFFSET= 0x84,
	SOUND_REQUEST_EFFECT_CALLBACKS_OFFSET= 0x88,
	SOUND_EFFECT_CONTROLLER_OWNER_FLAG= 0x4000,
	/* what a voice of the console's sound takes of the sound of a channel */
	SOUND_VOICE_PARAMETERS_UNKNOWN4_OFFSET= 0x4,
	SOUND_VOICE_PARAMETERS_CLASS_OFFSET= 0x10,
	SOUND_VOICE_PARAMETERS_UNKNOWN12_OFFSET= 0x12,
	SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET= 0x14,
	SOUND_VOICE_PARAMETERS_UNKNOWN18_OFFSET= 0x18,
	SOUND_VOICE_PARAMETERS_UNKNOWN1C_OFFSET= 0x1C,
	SOUND_VOICE_PARAMETERS_UNKNOWN20_OFFSET= 0x20,
	SOUND_VOICE_PARAMETERS_SLOT_OFFSET= 0x24,
	SOUND_VOICE_PARAMETERS_UNKNOWN28_OFFSET= 0x28,
	SOUND_VOICE_PARAMETERS_FLAGS_OFFSET= 0x30,
	SOUND_VOICE_PARAMETERS_UNKNOWN38_OFFSET= 0x38,
	SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET= 0x48,
	SOUND_VOICE_PARAMETERS_ENVIRONMENT_OFFSET= 0x468,
	SOUND_VOICE_PARAMETERS_SIZE= 0x478,
	SOUND_VOICE_PARAMETERS_PITCH_OFFSET= 0x8,
	SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET= 0xC,
	SOUND_VOICE_PARAMETERS_UNKNOWN34_OFFSET= 0x34,
	/* in a recording: its gain, in decibels; in a class of sounds and in
	what sound_channel_compute_parameters works out: how much of the Doppler effect a
	sound has; a flag of a sound that plays */
	SOUND_PERMUTATION_GAIN_OFFSET= 0x4,
	SOUND_CLASS_UNKNOWN48_OFFSET= 0x48,
	SOUND_CHANNEL_PARAMETERS_UNKNOWN14_OFFSET= 0x14,
	/* more of what sound_channel_compute_parameters works out; the two
	flags of a channel, at 0xA and 0xB of it; in how long a channel goes
	from the one to the other; and in a sound that plays, of what the game
	says of it: where it points and how fast it moves */
	SOUND_CHANNEL_PARAMETERS_UNKNOWN4_OFFSET= 0x4,
	SOUND_CHANNEL_PARAMETERS_UNKNOWN8_OFFSET= 0x8,
	SOUND_CHANNEL_PARAMETERS_UNKNOWNC_OFFSET= 0xC,
	SOUND_CHANNEL_PARAMETERS_GAIN_OFFSET= 0x18,
	SOUND_CHANNEL_PARAMETERS_POSITION_OFFSET= 0x1C,
	_sound_channel_direct_bit= 0,
	_sound_channel_slot_bit= 1,
	SOUND_CHANNEL_FADE_TIME= 1000,
	SOUND_SOURCE_DATUM_FORWARD_OFFSET= 0x30,
	SOUND_SOURCE_DATUM_VELOCITY_OFFSET= 0x34,
	SOUND_CLASS_UNKNOWN14_OFFSET= 0x14,
	/* in a class of sounds: four reals for each state of the sound of the
	game */
	SOUND_CLASS_STATES_OFFSET= 0x28,
	SOUND_CLASS_STATE_SIZE= 0x10,
	GAME_SOUND_GLOBALS_UNKNOWN_STATES_OFFSET= 0x1A8,
	UNKNOWN_0051EBE4_ENVIRONMENT_SIZE= 0x40,
	UNKNOWN_0051EBE4_UNKNOWN1_OFFSET= 0x1,
	/* the kinds of samples of a sound, and how the console calls them */
	_sound_samples_compressed_mono= 0,
	_sound_samples_compressed_stereo= 1,
	_sound_samples_uncompressed= 2,
	SOUND_CONSOLE_FORMAT_UNCOMPRESSED= 1,
	SOUND_CONSOLE_FORMAT_COMPRESSED= 0x69,
	SOUND_SAMPLES_PER_SECOND= 44100,
	_sound_unknown13_bit= 13,
	SOUND_COMPRESSED_SAMPLES_PER_BLOCK= 64,
	SOUND_COMPRESSED_BLOCK_SIZE= 36,
	/* more of a voice of the console's sound */
	_voice_unknown2_bit= 2,
	_voice_state_playing= 1,
	UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET= 0x6,
	MAXIMUM_SOUND_PLAYBACK_CONTROLLERS= 64,
	/* the cache of the sound */
	MAXIMUM_CACHED_SOUND_CHUNKS= 512,
	SOUND_CACHE_PAGE_COUNT= 192,
	SOUND_CACHE_PAGE_SIZE_BITS= 14,
	SOUND_CACHE_BLOCKS_SIZE= 0x208C,
	SOUND_CACHE_LRUV_SIZE= 0x30FC,
	/* in milliseconds */
	SOUND_CACHE_CLOSE_WAIT= 5000,
	_sound_class_internal_unknown10_bit= 10,
	/* of a sound of a looping sound that plays now and then */
	LOOPING_SOUND_DETAIL_SOUND_INDEX_OFFSET= 0x8,
	LOOPING_SOUND_DETAIL_YAW_BOUNDS_OFFSET= 0x1C,
	LOOPING_SOUND_DETAIL_PITCH_BOUNDS_OFFSET= 0x24,
	LOOPING_SOUND_DETAIL_DISTANCE_BOUNDS_OFFSET= 0x2C,
	/* in what the game says of a sound: which way it points (a vector in 32
	bits) and how fast it moves; a byte of the tag of
	a sound; and how many players the console has */
	SOUND_SOURCE_FORWARD_OFFSET= 0x18,
	SOUND_SOURCE_VELOCITY_OFFSET= 0x1C,
	SOUND_DEFINITION_UNKNOWN4_OFFSET= 0x4,
	PLAYERS_GLOBALS_LOCAL_PLAYER_COUNT_OFFSET= 0x8,
	/* of what the tags have of a sound effect, and of what unknown_0018d090
	gives */
	SOUND_EFFECT_COLLECTION_FLAGS_OFFSET= 0x8,
	SOUND_CONSOLE_SLOT_BUFFER_FLAGS= 0x2010,
	SOUND_CONSOLE_SLOT_MIX_BUFFER_FLAGS= 0x100000,
	MAXIMUM_SOUND_CONSOLE_SLOTS= 64,
	/* what sound_console_set_listener takes, and where the console's sound
	keeps it */
	SOUND_CONSOLE_LISTENER_FACING_OFFSET= 0xC,
	SOUND_CONSOLE_LISTENER_PLACE_COUNT_OFFSET= 0x30,
	SOUND_CONSOLE_PLACE_SIZE= 0x18,
	SOUND_CONSOLE_PLACE_NUMBERS_SIZE= 0x10,
	SOUND_CONSOLE_PLACE_AMOUNT_OFFSET= 0x14,
	NUMBER_OF_SOUND_CONSOLE_PLACES= 2,
	NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS= 4,
	UNKNOWN_0051EBE4_SIZE= 0x2AD8,
	UNKNOWN_0051EBE4_UNKNOWN2AA0_OFFSET= 0x2AA0,
	SOUND_CONSOLE_LISTENER_SIZE= 0x38,
	/* of the effects of a voice, and of a voice */
	SOUND_VOICE_EFFECTS_FRACTION_OFFSET= 0x4,
	SOUND_VOICE_EFFECTS_FILTER_OFFSET= 0x10,
	VOICE_FRACTION_OFFSET= 0x4,
	VOICE_FLAGS3_OFFSET= 0x3,
	VOICE_DECIBELS_OFFSET= 0xC,
	VOICE_UNKNOWN10_OFFSET= 0x10,
	VOICE_UNKNOWN14_OFFSET= 0x14,
	VOICE_UNKNOWN18_OFFSET= 0x18,
	/* what the console gives of where a stream is heard */
	SOUND_CONSOLE_STREAM_MIXBINS_SIZE= 0x5C,
	VOICE_STREAM_OFFSET= 0x24,
	UNKNOWN_0051EBE4_PLACE_STREAMS_OFFSET= 0x2A70,
	SOUND_ENVIRONMENT_SIZE= 0x48,
	UNKNOWN_0051EBE4_LISTENER_POSITION_OFFSET= 0x2918,
	UNKNOWN_0051EBE4_LISTENER_FACING_OFFSET= 0x2924,
	UNKNOWN_0051EBE4_PLACE_ENVIRONMENTS_OFFSET= 0x2950,
	UNKNOWN_0051EBE4_PLACE_AMOUNTS_OFFSET= 0x29E0,
	UNKNOWN_0051EBE4_PLACE_NUMBERS_OFFSET= 0x29E8,
	SOUND_SILENCE_SIZE= 1000,
	SOUND_UNKNOWN_00222250_MAXIMUM= 4,
	/* more of an entry of the list that sound_unknown_002225a0 makes, and of
	what the game knows of an effect of the processor of the console's sound:
	the first thing of unknown_00510c90 says where they are */
	SOUND_EFFECT_ENTRY_FLAGS_OFFSET= 0x4,
	_sound_effect_entry_whole_bit= 0,
	_sound_effect_entry_skip_bit= 1,
	SOUND_EFFECT_ENTRY_OFFSET_OFFSET= 0x8,
	SOUND_CONSOLE_EFFECT_SIZE= 0x20,
	SOUND_CONSOLE_EFFECT_STATE_OFFSET= 0x10,
	SOUND_CONSOLE_EFFECT_STATE_SIZE_OFFSET= 0x14,
	UNKNOWN_00510C90_EFFECTS_OFFSET= 0x22,
	/* more of a looping sound, of its tag and of a sound of it that plays now
	and then */
	LOOPING_SOUND_DATUM_UNKNOWN2_OFFSET= 0x2,
	LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET= 0x50,
	LOOPING_SOUND_DATUM_DETAIL_TIMES_OFFSET= 0x58,
	LOOPING_SOUND_DATUM_TRACK_SEED_OFFSET= 0x90,
	LOOPING_SOUND_DEFINITION_TRACKS_OFFSET= 0x1C,
	LOOPING_SOUND_DEFINITION_DETAILS_OFFSET= 0x24,
	LOOPING_SOUND_DETAIL_SIZE= 0x34,
	LOOPING_SOUND_DEFINITION_TRACK_SIZE= 0x58,
	LOOPING_SOUND_DEFINITION_TRACK_SOUND_INDEX_OFFSET= 0x20,
	/* in a sound that plays of a looping sound: which track it is of */
	SOUND_SOURCE_DATUM_TRACK_OFFSET= 0x60,
	LOOPING_SOUND_DETAIL_PERIOD_BOUNDS_OFFSET= 0xC,
	LOOPING_SOUND_DETAIL_FLAGS_OFFSET= 0x18,
	_looping_sound_detail_unknown2_bit= 2,
	UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET= 0x2A08,
	UNKNOWN_0051EBE4_UNKNOWN2AD4_OFFSET= 0x2AD4,
	SOUND_CONSOLE_FORMAT_BASE_SIZE= 0x12,
	VOICE_UNKNOWN2_OFFSET= 0x2,
	VOICE_UNKNOWN30_OFFSET= 0x30,
	NUMBER_OF_SOUND_UNKNOWN_STATES= 2,
	_sound_source_datum_unknown3_bit= 3,
	_sound_source_datum_unknown4_bit= 4,
	SOUND_CLASS_UNKNOWNE_OFFSET= 0xE,
	/* more of a sound that plays, of a listener, of a class of sounds and of
	the slots of the console's sound */
	_sound_source_unknown12_bit= 12,
	/* a flag of what the game says of a sound: what is not heard of it
	from where its listener is, is what counts */
	_sound_source_inverted_gain_bit= 9,
	SOUND_SOURCE_DATUM_SCALE_OFFSET= 0x1C,
	SOUND_LISTENER_UNKNOWN7_OFFSET= 0x7,
	SOUND_LISTENER_MATRIX_OFFSET= 0x8,
	/* from the place of a sound, in what the game says of it: how fast it
	moves */
	SOUND_POSITION_VELOCITY_OFFSET= 0x10,
	SOUND_CLASS_UNKNOWN54_OFFSET= 0x54,
	SOUND_CLASS_UNKNOWN58_OFFSET= 0x58,
	UNKNOWN_0051EBE4_SLOTS_OFFSET= 0x1A18,
	UNKNOWN_0051EBE4_SLOT_FLAGS_OFFSET= 0x4,
	_sound_console_slot_unknown0_bit= 0,
	_sound_console_slot_unknown1_bit= 1,
	MAXIMUM_SOUND_QUERIES= 72,
	/* of the structure bsp: how many clusters it has, and the count and the
	address of its openings for the sound, which starts with how many there
	are and has what each says at 0x30 */
	MAXIMUM_STRUCTURE_BSP_CLUSTERS= 512,
	STRUCTURE_BSP_CLUSTER_COUNT_OFFSET= 0x9C,
	STRUCTURE_BSP_PORTALS_OFFSET= 0x60,
	STRUCTURE_BSP_UNKNOWN68_OFFSET= 0x68,
	STRUCTURE_BSP_UNKNOWN68_SIZE= 0x18,
	STRUCTURE_BSP_CLUSTER_UNKNOWN70_OFFSET= 0x70,
	/* of what unknown_004e9c8c has for each local player, of a player, of an
	object and of the tag of a unit */
	SOUND_OBSERVER_SIZE= 0x358,
	SOUND_OBSERVER_LOCATION_OFFSET= 0xC,
	SOUND_OBSERVER_VELOCITY_OFFSET= 0x14,
	SOUND_OBSERVER_FORWARD_OFFSET= 0x20,
	SOUND_PLAYER_DATUM_SIZE= 0x21C,
	SOUND_PLAYER_DATUM_UNIT_INDEX_OFFSET= 0x2C,
	SOUND_OBJECT_PARENT_INDEX_OFFSET= 0x14,
	SOUND_UNIT_FORWARD_OFFSET= 0x180,
	SOUND_UNIT_SEAT_INDEX_OFFSET= 0x1FC,
	SOUND_UNIT_DEFINITION_SEATS_OFFSET= 0x1CC,
	SOUND_UNIT_SEAT_SIZE= 0xB0,
	SOUND_UNIT_SEAT_LISTENER_FRACTION_OFFSET= 0x40,
	STRUCTURE_BSP_CLUSTER_SIZE= 0xB0,
	STRUCTURE_BSP_CLUSTER_UNKNOWN7A_OFFSET= 0x7A,
	UNKNOWN_004ED280_SIZE= 32,
	STRUCTURE_BSP_PORTAL_SIZE= 0x24,
	STRUCTURE_BSP_SOUND_OPENINGS_OFFSET= 0x224,
	STRUCTURE_BSP_SOUND_OPENINGS_SAID_OFFSET= 0x30,
	_sound_source_datum_unknown2_bit= 2,
	_sound_class_internal_unknown6_bit= 6,
	_sound_definition_unknown5_bit= 5,
	/* what the one who is told of a sound is told when all are stopped */
	_sound_source_deleted_unknown10= 10,

	_game_mode_main_menu= 3
};

#define SOUND_PI 3.14159265358979323846f

/* the most that a real can be */
#define SOUND_REAL_MAXIMUM 3.4028234663852886e+38f

/* how fast sound goes, in world units each second: 340 metres */
#define SOUND_SPEED 111.54855f

/* the cents of an interval: 1200 for each time that the pitch doubles */
#define SOUND_CENTS_PER_NATURAL_LOGARITHM 1731.2340f

/* the square of the least that a sound is away from its listener for its
cone to count */
#define SOUND_MINIMUM_OFFSET_SQUARED (0.001f*0.001f)

/* the least that a sound is heard, in decibels */
#define SOUND_MINIMUM_DECIBELS -64.0f

/* ---------- structures */

/* who is told of what happens to a sound that plays. Only two of its
functions are known */
struct sound_source_callbacks
{
	boolean (__attribute__((stdcall)) *unknown0)(long data, long sound_index, void *unknown5C, void *unknown18);
	void (__attribute__((stdcall)) *got_channel)(long data, long sound_index, long source_index, void *unknown5C);
	void (__attribute__((stdcall)) *playing)(long data, void *unknown5C, long sound_index, long pitch_range,
		long permutation, real position);
	boolean (__attribute__((stdcall)) *heard)(long data, long sound_index, void *unknown5C, byte *parameters);
	void (__attribute__((stdcall)) *deleted)(long data, long source_index, long reason);
	void (__attribute__((stdcall)) *stopped)(long data, long source_index);
	boolean (__attribute__((stdcall)) *matches)(const void *unknown5C, const void *unknown14);
	boolean (__attribute__((stdcall)) *same)(long data, const void *unknown5C, long other_data, const void *other_unknown5C);
};

/* what sound_source_find_same fills: the channels that play a sound, and
those that play it for the same thing, with how many of each there can be */
struct sound_same_channels
{
	short sound_count;
	short sound_channels[16];
	short sound_limit;
	boolean started_now;
	byte pad;
	short instance_count;
	short instance_channels[16];
	short instance_limit;
};

typedef char verify_sound_same_channels_size[
	sizeof(struct sound_same_channels) == 0x4A ? 1 : -1];

/* what sound_console_slot_set_parameters takes for a slot of the console's sound */
struct sound_slot_parameters
{
	unsigned long flags;
	real_point3d position;
	real unknown10;
	real unknown14;
	real unknown18;
	real unknown1C;
	real unknown20;
};

typedef char verify_sound_slot_parameters_size[
	sizeof(struct sound_slot_parameters) == 0x24 ? 1 : -1];

/* a slot of the console's sound: a place that a sound is heard from */
struct sound_console_slot
{
	byte unknown0[4];
	byte flags;
	byte unknown5[3];
	/* what it was last given */
	real_point3d position;
	byte unknown14[0x18];
	real unknown2C;
	real unknown30;
	/* of the console */
	void *stream;
	void *unknown38;
};

typedef char verify_sound_console_slot_size[
	sizeof(struct sound_console_slot) == 0x3C ? 1 : -1];

/* what the console takes of how a sound is heard in a room: hundredths of
a decibel, and three reals */
struct sound_console_i3dl2_source
{
	long direct;
	long direct_high_frequencies;
	long room;
	long room_high_frequencies;
	real room_rolloff;
	long obstruction_high_frequencies;
	real obstruction_low_frequency_ratio;
	long occlusion_high_frequencies;
	real occlusion_low_frequency_ratio;
};

/* what the console takes of the filter of a sound */
struct sound_console_filter
{
	long mode;
	long unknown4;
	long coefficients[4];
};

typedef char verify_sound_console_i3dl2_source_size[
	sizeof(struct sound_console_i3dl2_source) == 0x24 ? 1 : -1];
typedef char verify_sound_console_filter_size[
	sizeof(struct sound_console_filter) == 0x18 ? 1 : -1];

/* one of the two things of the same kind that the globals of the sound keep
at 0x1A8, each of which comes in and goes out: what it is of (NONE for
none), how much of it there is, from 0 to 1, in how many seconds it comes
and goes, and four reals that follow what it is given: the first two as
angles do. What they are of is not known: the game gives them from
0x0018bb80 */
struct sound_unknown_state
{
	real duration;
	real amount;
	long index;
	real unknownC;
	real unknown10;
	real unknown14;
	real unknown18;
};

typedef char verify_sound_unknown_state_size[
	sizeof(struct sound_unknown_state) == 0x1C ? 1 : -1];

/* what the game keeps at 0x2AB4 of the console's sound: something with seven
functions, which take it in ecx and take their arguments off the stack. The
first is given the sound of the places of the map */
struct sound_console_environment_handler;

struct sound_console_environment_handler_functions
{
	void (__attribute__((thiscall)) *set)(struct sound_console_environment_handler *handler, const void *environment,
		const void *unknown, long unknown2);
	void (__attribute__((thiscall)) *dispose)(struct sound_console_environment_handler *handler);
	void (__attribute__((thiscall)) *unknown8)(struct sound_console_environment_handler *handler, const void *unknown,
		long unknown2);
	/* these four fill the outputs where a voice is heard */
	void (__attribute__((thiscall)) *unknownC)(struct sound_console_environment_handler *handler, const void *places,
		long unknown, long unknown2, long *mixbins);
	void (__attribute__((thiscall)) *unknown10)(struct sound_console_environment_handler *handler, long unknown,
		long unknown2, real unknown3, long *mixbins);
	void (__attribute__((thiscall)) *unknown14)(struct sound_console_environment_handler *handler, const byte *parameters,
		long unknown, long unknown2, long *mixbins);
	void (__attribute__((thiscall)) *unknown18)(struct sound_console_environment_handler *handler, const void *places,
		long unknown, long unknown2, long *mixbins);
};

struct sound_console_environment_handler
{
	const struct sound_console_environment_handler_functions *functions;
};

/* the two things of the console that a voice of the console's sound has, at
0x24 and 0x30: of their functions, those that the game calls. Those of the
first take it on the stack; those of the second, in ecx */
struct sound_console_stream_functions
{
	void *unknown0;
	void (__attribute__((stdcall)) *unknown4)(void *stream);
	void *unknown8;
	void (__attribute__((stdcall)) *unknownC)(void *stream, long *status);
	void *unknown10[2];
	void (__attribute__((stdcall)) *unknown18)(void *stream);
};

struct sound_console_voice_object_functions
{
	void *unknown0[6];
	void (__attribute__((thiscall)) *unknown18)(void *object, byte *voice, long unknown, long unknown2);
	void (__attribute__((thiscall)) *unknown1C)(void *object);
};

/* how the samples of a sound are, as the console takes it: the last two
numbers are only of the compressed ones */
struct sound_console_format
{
	word format;
	word channels;
	unsigned long samples_per_second;
	unsigned long bytes_per_second;
	word block_size;
	word bits_per_sample;
	word extra_size;
	word samples_per_block;
};

typedef char verify_sound_console_format_size[
	sizeof(struct sound_console_format) == 0x14 ? 1 : -1];

/* where a sound of the console is to be heard: a number of pairs, each of
where and how loud */
struct sound_console_mixbins
{
	long count;
	const long *pairs;
};

/* what the console takes to make what plays the sound of a slot, or any
sound that is not a stream */
struct sound_console_buffer_description
{
	unsigned long size;
	unsigned long flags;
	unsigned long buffer_size;
	const struct sound_console_format *format;
	const struct sound_console_mixbins *mixbins;
	unsigned long unknown14;
};

typedef char verify_sound_console_buffer_description_size[
	sizeof(struct sound_console_buffer_description) == 0x18 ? 1 : -1];

/* what the console takes to make a stream of sound */
struct sound_console_stream_description
{
	unsigned long flags;
	unsigned long maximum_packets;
	const struct sound_console_format *format;
	void (__attribute__((stdcall)) *callback)(long context, long unknown, long unknown2);
	long context;
	const void *unknown14;
};

typedef char verify_sound_console_stream_description_size[
	sizeof(struct sound_console_stream_description) == 0x18 ? 1 : -1];

/* how a place sounds, as the game has it (the reals of it: four loudnesses
in decibels, at 8, 0xC, 0x1C and 0x24, and eight more), and as the effect of
the console that makes it takes it: how many numbers, and the numbers, the
loudnesses in hundredths of a decibel */
struct sound_console_reverb
{
	long count;
	long room;
	long room_high_frequencies;
	real room_rolloff;
	real decay_time;
	real decay_high_frequency_ratio;
	long reflections;
	real reflections_delay;
	long reverb;
	real reverb_delay;
	real diffusion;
	real density;
	real high_frequency_reference;
};

typedef char verify_sound_console_reverb_size[
	sizeof(struct sound_console_reverb) == 0x34 ? 1 : -1];

/* a sound of which it is to be worked out how it is heard where it is */
struct sound_query
{
	long listener_index;
	/* where the line from the listener to the sound hits the level, where the
	one back does, and how far along each that is */
	real_point3d near_point;
	real_point3d far_point;
	real near_fraction;
	real far_fraction;
	real_point3d position;
	/* what is worked out */
	real results[3];
	long cluster_index;
	long leaf_index;
	boolean unknown44;
	boolean unknown45;
	boolean unknown46;
	boolean unknown47;
};

typedef char verify_sound_query_size[
	sizeof(struct sound_query) == 0x48 ? 1 : -1];

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

/* 0x004e636d: whether looping_sound_check_distances looks at anything */
boolean unknown_004e636d;

/* 0x004ed28c: an array of something else, which the sound effects of the
kind 2 are of */
struct data_array *unknown_004ed28c;

/* 0x0047f088: the handler of the sound of the places that the console's
sound starts with */
struct sound_console_environment_handler unknown_0047f088;

/* 0x004e9c8c: where each local player sees the game from, most likely: 0x358
bytes for each. Only what the listeners take of it is known */
byte unknown_004e9c8c[MAXIMUM_LOCAL_PLAYERS*0x358];

/* 0x004ed280: of something else; it has 32 bytes for each structure bsp */
byte *unknown_004ed280;

/* 0x004ed288: of something else; two numbers of it start again with the sound */
byte *unknown_004ed288;

/* 0x00502108: where the memory of the cache of the sounds starts */
long sound_cache_base;

/* 0x00502118: four reals for each class of sounds */
real *sound_class_gains;

/* 0x00470090: the names of the classes of sounds; those with no name are not
used */
const char *sound_class_names[NUMBER_OF_SOUND_CLASSES]=
{
	"projectile_impact",
	"projectile_detonation",
	"projectile_flyby",
	"",
	"weapon_fire",
	"weapon_ready",
	"weapon_reload",
	"weapon_empty",
	"weapon_charge",
	"weapon_overheat",
	"weapon_idle",
	"weapon_melee",
	"weapon_animation",
	"object_impacts",
	"particle_impacts",
	"",
	"",
	"",
	"unit_footsteps",
	"unit_dialog",
	"unit_animation",
	"",
	"vehicle_collision",
	"vehicle_engine",
	"vehicle_animation",
	"",
	"device_door",
	"",
	"device_machinery",
	"device_stationary",
	"",
	"",
	"music",
	"ambient_nature",
	"ambient_machinery",
	"",
	"huge_ass",
	"object_looping",
	"cinematic_music",
	"",
	"",
	"",
	"",
	"",
	"",
	"cortana_mission",
	"cortana_cinematic",
	"mission_dialog",
	"cinematic_dialog",
	"scripted_cinematic_foley",
	"game_event",
	"ui",
	"test",
	"multilingual_test"
};

/* 0x0055e780: while it is set, no sound shares a sound voice */
boolean unknown_0055e780;

/* 0x00547f14: how much later a sound starts for each unit that it is far */
real unknown_00547f14;

/* 0x00510c90: of something else; a number of 16 bits is at 6 of it */
byte *unknown_00510c90;

/* 0x00468788: where a point is that the sounds with no place are asked at */
const real_point3d *unknown_00468788;

/* 0x0047f0e0: 0x10000 in the main menu and 0x4000 in a game */
long unknown_0047f0e0;

/* 0x0047f0e4: what makes sounds of the kind 2 be told so by
sound_definition_unknown_00126bd0 */
long unknown_0047f0e4;

/* 0x00470084 and 0x00470088: a frequency, 8000, and some decibels, 0, for
the filter of the slots of the console's sound */
/* 0x00470060: 0x24 bytes for the handler of the sound of the places of the
map. What they are is not known */
byte unknown_00470060[0x24];

/* 0x0047005c: set when the console's sound takes its gains */
boolean unknown_0047005c= TRUE;

/* 0x00468834: a gain that is added to how loud a place sounds: 1 */
real unknown_00468834= 1.0f;

/* 0x00386ad8: a number that the console's sound follows. What it is of is not
known */
long unknown_00386ad8;

/* 0x0047f0d0 and 0x0047f0f0: two things of the game with functions of their
own, for the samples that are not compressed and for those that are. What
they do is not known. The first is 0x20 bytes too: its numbers at 0x10 and
0x14 are unknown_0047f0e0 and unknown_0047f0e4 */
byte unknown_0047f0d0[0x10];
byte unknown_0047f0f0[0x20];

long unknown_00470084= 8000;
real unknown_00470088= 0.0f;

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
void sound_cache_initialize(void);
void sound_cache_close(void);

/* 0x002189a0 and 0x002189e0 */
boolean __attribute__((stdcall)) sound_cache_block_locked(long block_index);
void __attribute__((stdcall)) sound_cache_block_deleted(long block_index);
void sound_cache_dispose(void);

/* 0x00219b40, 0x0021a0c0, 0x0021d490, 0x0021dda0, 0x0021e4b0: set up the
sound playback controllers, the looping sounds, the sound effects, the sound
voices and something more. FALSE if they cannot */
boolean sound_playback_controllers_initialize(void);
boolean looping_sounds_initialize(void);
boolean sound_effects_initialize(void);
boolean sound_voices_initialize(const struct sound_configuration *configuration);
boolean __attribute__((stdcall)) sound_console_initialize(const struct sound_configuration *configuration);
/* 0x0037d797: the console makes its sound; 0x0037b5ae, 0x0037d506, 0x0037d5cd,
0x0037b5ca, 0x0037b637 and 0x0037d52a: it is asked and told things of it;
0x00379f5b; 0x00191300: the processor of the sound is given its program;
0x0012c040 and 0x00191420 */
long unknown_0037d797(void *unknown, void **sound, void *unknown2);
long unknown_0037b5ae(void *sound, void *unknown);
long unknown_0037d506(void *sound, real unknown, long apply);
long unknown_0037d5cd(void *sound, real unknown, long apply);
long unknown_0037b5ca(void *sound, unsigned long *speakers);
void unknown_0037b637(void *sound, long output, long unknown);
void unknown_0037d52a(void *sound, real unknown, long apply);
void unknown_00379f5b(void);
boolean unknown_00191300(void *sound);
void xmem_static_buffer_allocate(void);
void unknown_00191420(void *sound);

/* 0x0021eae0: takes down what sound_console_initialize set up, most likely */
void sound_console_dispose(void);

/* 0x00379f45 and 0x00379f2a: of the console: something of its sound is let
go; 0x002ae170 */
void unknown_00379f45(void *unknown);
void unknown_00379f2a(void *unknown);
void unknown_002ae170(void *unknown);

/* 0x003314b0 */
unsigned long GetTickCount(void);

/* of the C library of the console */
long time(long *timer);
int rand(void);

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
void __attribute__((stdcall)) sound_listeners_update(boolean tell_console);
/* 0x00155760: something of a listener, 0 or 1 for those that are heard from;
0x000caf60: where the head of a unit is, most likely */
long unknown_00155760(long listener_index);
void object_get_marker95_position(long unit_index, real_point3d *point);
void sound_playback_controllers_update(void);
void __attribute__((stdcall)) looping_sounds_update(boolean update_sources);
void sound_sources_update(void);
void sound_sources_get_channels(void);
void __attribute__((stdcall)) sound_channels_build_queries(byte *first, byte *second, struct sound_update_listeners *listeners);
long players_first_local_player(void);
void __attribute__((stdcall)) sound_listener_compute_queries(long listener_index,
	struct sound_update_listeners *listeners);

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
real sound_class_get_decibels(short sound_class);

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

void sound_console_update(void);

/* 0x001915f0; 0x0037b844: the console does the work of its sound */
void unknown_001915f0(void);
void unknown_0037b844(void);
void sound_update_time(void);
void sound_channels_update_sources(void);
void sound_console_commit(void);
/* 0x0037d141: the console does all that it was told of its sound; 0x00191550 */
void unknown_0037d141(void *sound);
void unknown_00191550(void);
void sound_cache_idle(void);

/* 0x0021f6d0: sets the sound of the places of the map, from three parts of
a tag */

/* 0x00218850: asks for a piece of the samples of a sound. Bits come back */
unsigned long __attribute__((stdcall)) sound_cache_chunk_request(long sound_index, const struct sound_chunk *chunk,
	long flags);

/* 0x001268e0: of a sound that plays, when its samples are not there */

/* 0x0012a810, 0x00219650 and 0x002197b0 */
real sound_source_fade_decibels(long source_index);
real __attribute__((stdcall)) sound_decibels_interpolate(long kind, real from, real to, real fraction);
real __attribute__((stdcall)) sound_decibels_fade_in(long kind, real decibels, real fraction);

/* 0x00219f80 */
real __attribute__((stdcall)) sound_pitch_range_fade_decibels(const byte *flags,
	const struct sound_pitch_range *pitch_range, real pitch);



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
void sound_console_voice_stop(long voice_index, long unknown);

/* 0x002ae8e0 and 0x002ae6f0: of a voice of the console's sound */
void unknown_002ae8e0(byte *voice);
void unknown_002ae6f0(byte *voice);

/* 0x00220730; 0x002ae820: of a voice of the console's sound */
void __attribute__((stdcall)) sound_console_voice_stream_callback(long voice_index, long unknown, long unknown2);
void unknown_002ae820(byte *voice);

/* 0x0021f4e0; 0x002ae660: makes the stream of a voice of the console's sound */
struct sound_console_stream_description;
boolean __attribute__((stdcall)) sound_console_voice_create_stream(long kind, long voice_index);
boolean unknown_002ae660(byte *voice, void *object, struct sound_console_stream_description *description, long kind);
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
void sound_console_voices_stop(void);
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

/* 0x00219430, 0x00219910, 0x00219960 and 0x00219a30 */
long __attribute__((stdcall)) sound_permutation_get_unknown6(const struct sound_permutation *permutation,
	const struct sound_definition *sound);
void *sound_playback_controller_try_and_get(long controller_index);
long sound_playback_controller_find(long owner);
long sound_playback_controller_acquire(long owner);

/* 0x001462b0: a number from the clocks of the console; 0x00219ed0: of a
sound playback controller */
long random_seed_from_time(void);
void __attribute__((stdcall)) sound_playback_controller_unknown_00219ed0(boolean unknown, byte *controller,
	const struct sound_source *source);

/* 0x0014de10: the local player after one; 0x0012a9d0: the Doppler effect
of a place for a listener; 0x0021d630: of a sound effect */
long players_next_local_player(long player_index);
long double sound_listener_doppler_pitch(long listener_index, const real_point3d *position);
void sound_effect_start_sounds(long effect_index, long phase);
/* 0x0021a250: a looping sound starts, or goes on, or ends */
boolean __attribute__((stdcall)) looping_sound_start(long definition_index, long unknown2, long owner,
	long controller_owner, const struct sound_source *source, long state, unsigned long flags, real seconds);
/* 0x00192d70: how loud something is for how far it is; 0x00192ce0: four
parts of a direction */
real __attribute__((stdcall)) sound_speakers_distance_gain(real minimum_distance, real maximum_distance, real distance);
void __attribute__((stdcall)) sound_speakers_get_gains4(const real_vector3d *direction, real *parts, long unknown);

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
boolean __attribute__((stdcall)) sound_start_test(const byte *request, long sound_index, long *listener_index,
	long *reason);
long __attribute__((stdcall)) sound_promotion_update(long sound_index, long *rule_index);
long __attribute__((stdcall)) sound_source_create(long sound_index, long listener_index, byte *request,
	long rule_index);

/* 0x002226b0, 0x00222930 and 0x001914f0 */
void sound_request_unknown_002226b0(byte *request, long sound_index);
void sound_unknown_00222930(long sound_index, long unknown, byte *result);

/* 0x002225a0; 0x002222c0: writes an entry of a list for the console's sound */
void __attribute__((stdcall)) sound_unknown_002225a0(const byte *overrides, const long *block, long unknown,
	long *list, long *size);
void sound_effect_entry_build(long *entry, const long *element, const byte *override, const real *inputs,
	long number);
/* 0x00191660: which effects of the console an element is for; 0x0013b390:
what a function of a tag is of two reals */
long sound_unknown_00191660(long name, long number);
real unknown_0013b390(const byte *function, real input, real range);

/* 0x0021dc60; 0x0021d9f0, 0x0021da30, 0x0021da50, 0x0021da70 and 0x0021db30:
what a sound effect is told of its sounds */
void sound_effect_prepare_request(long effect_index, byte *request);
boolean __attribute__((stdcall)) sound_effect_source_unknown0(long data, long sound_index, void *unknown5C, void *unknown18);
/* 0x0021d970 */
boolean __attribute__((stdcall)) sound_effect_sound_unknown0(long data, long sound_index, void *unknown5C, void *unknown18);
void __attribute__((stdcall)) sound_effect_source_got_channel(long data, long sound_index, long source_index, void *unknown5C);
void __attribute__((stdcall)) sound_effect_source_playing(long data, void *unknown5C, long sound_index, long pitch_range,
	long permutation, real position);
boolean __attribute__((stdcall)) sound_effect_source_heard(long data, long sound_index, void *unknown5C, byte *parameters);
void __attribute__((stdcall)) sound_effect_source_stopped(long data, long source_index);

/* 0x002228d0 and 0x002221d0 */
void __attribute__((stdcall)) sound_unknown_002228d0(long unknown, byte *result, long unknown2);
long __attribute__((stdcall)) sound_real_to_fixed24(real value);

/* 0x00222770 and 0x0012a1b0 */
void __attribute__((stdcall)) sound_unknown_00222770(long sound_index, long unknown, long unknown2,
	byte *result);
void __attribute__((stdcall)) sound_channel_build_voice_parameters(short channel_index, const byte *parameters,
	byte *result, const byte *source);

long sound_unknown_001914f0(long name);
void __attribute__((stdcall)) sound_channel_update_slot(short channel_index, const real *unknown);

/* 0x00220e00; 0x0037c668: the console is told where a sound is; 0x002211e0:
of a slot of the console's sound */
void __attribute__((stdcall)) sound_console_slot_set_parameters(const struct sound_slot_parameters *parameters,
	long slot_index);
void unknown_0037c668(void *stream, real x, real y, real z, long apply);
void __attribute__((stdcall)) sound_console_slot_update_room(long slot_index, real decibels);

/* 0x0037c6e5 and 0x0037b68b: the console is told how a sound is heard in a
room, and its filter */
struct sound_console_i3dl2_source;
struct sound_console_filter;
void unknown_0037c6e5(void *stream, const struct sound_console_i3dl2_source *source, long apply);
void unknown_0037b68b(void *stream, const struct sound_console_filter *filter);
struct sound_console_environment_handler;
void sound_console_set_environment_handler(struct sound_console_environment_handler *handler);

/* 0x00126660 */
struct sound_unknown_state;
void sound_unknown_states_set(const struct sound_unknown_state *wanted);

/* 0x00125f70, 0x001268e0, 0x00129380, 0x0021f6d0 and 0x00221900 */
long __attribute__((stdcall)) sound_source_new(long sound_index, long *failure, byte *request);
void sound_source_hold_chunk(byte *source);
void sound_channels_unknown_00129380(const long *voice_values, const long *slot_values);
void __attribute__((stdcall)) sound_console_set_environment(const long *second, const long *first, const void *unknown);
void sound_classes_update(real seconds);

/* 0x00220a70; 0x0037d4be, 0x0037c5e4, 0x0037c620, 0x0037c644, 0x0037c6c1,
0x0037c69d and 0x0037c600: of the console */
struct sound_console_buffer_description;
struct sound_console_mixbins;
boolean sound_console_slot_create_stream(long slot_index);
long unknown_0037d4be(void *sound, const struct sound_console_buffer_description *description, void **stream,
	void *unknown);
void unknown_0037c5e4(void *stream, const struct sound_console_mixbins *mixbins);
void unknown_0037c620(void *stream, real unknown, long apply);
void unknown_0037c644(void *stream, real unknown, long apply);
void unknown_0037c6c1(void *stream, real unknown, long apply);
void unknown_0037c69d(void *stream, real unknown, long apply);
void unknown_0037c600(void *stream, long unknown, long apply);

/* 0x002215d0 and 0x002216f0; 0x0037c70a, 0x0037b81d and 0x0037b6c3: of the
console */
void __attribute__((stdcall)) sound_console_stream_set_mixbins(const long *old, const long *new, void *stream);
void __attribute__((stdcall)) sound_console_slot_stream_set_mixbins(const long *old, const long *new, void *stream);
void unknown_0037c70a(void *stream, const struct sound_console_mixbins *mixbins);
void unknown_0037b81d(void *stream, const struct sound_console_mixbins *volumes);
void unknown_0037b6c3(void *stream, const struct sound_console_mixbins *volumes);

/* 0x00221490; 0x0037d7de, 0x0037cc4a, 0x0037b6a7, 0x0037b66f and 0x0037b6df:
of the console */
void __attribute__((stdcall)) sound_console_create_silence(void **silence);
long unknown_0037d7de(const struct sound_console_buffer_description *description, void **buffer);
void unknown_0037cc4a(void *buffer, void *samples, unsigned long size);
void unknown_0037b6a7(void *buffer, long unknown);
void unknown_0037b66f(void *buffer, long unknown);
void unknown_0037b6df(void *buffer, long unknown, long unknown2, long unknown3);

/* 0x00222250; 0x00192b90: something of a direction */
real sound_unknown_00222250(const real_vector3d *vector, long index);
real unknown_00192b90(long index, const real_vector3d *direction, long unknown);

/* 0x0021f960; 0x0037ba6f and 0x0037b60d: of the console */
void __attribute__((stdcall)) sound_console_apply_effect_list(long size, const byte *list);
void unknown_0037ba6f(unsigned long effect_index, const byte *data, long unknown);
void unknown_0037b60d(void *sound, unsigned long effect_index, unsigned long offset, const byte *data,
	unsigned long size, long unknown);

/* 0x0021ece0 */
void __attribute__((stdcall)) sound_console_set_reverb(const real *environment, long effect_index);

/* 0x0021aeb0 */
long __attribute__((stdcall)) looping_sound_new(long definition_index, short unknown2, long unknown8, long owner);

/* 0x00220fd0 */
void sound_console_set_gains(const real *gains);

/* 0x00220b90 */
short __attribute__((stdcall)) sound_console_slot_new(unsigned long flags, long unknown);

/* 0x0021c190 */
boolean __attribute__((stdcall)) looping_sound_source_next_state(byte *looping_sound, byte *state,
	long source_index, unsigned long *seed);

/* 0x00220790; 0x0037d598 and 0x0037d54e: the console is told where who hears
is, and which way it faces */
void __attribute__((stdcall)) sound_console_set_listener(const byte *listener);
void unknown_0037d598(void *sound, real x, real y, real z, long apply);
void unknown_0037d54e(void *sound, real front_x, real front_y, real front_z, real top_x, real top_y, real top_z,
	long apply);

/* 0x00249d10, 0x00249c90 and 0x0018cac0: the openings of a cluster, the
clusters of the two sides of an opening and what an opening says, as a real;
0x00020670 calls 0x0031bc70, which does nothing, with each of the two sides;
0x00221e30 works out a query */
struct sound_query;
void unknown_00249d10(const byte *structure_bsp, unsigned long *openings, long cluster_index);
void unknown_00249c90(unsigned long *first, const byte *structure_bsp, long opening_index, unsigned long *second);
real sound_unknown_0018cac0(long said);
void __attribute__((stdcall)) array_construct_elements(void *array, unsigned long size, long count,
	void *(__attribute__((thiscall)) *constructor)(void *element));
void *__attribute__((thiscall)) unknown_0031bc70(void *element);
void __attribute__((stdcall)) sound_query_compute(struct sound_query *query, const real *heard,
	const struct sound_listener *listener);
/* 0x001de630: whether a line hits the level, and where */
boolean unknown_001de630(void *result, long unknown, void *collision_bsp, real maximum, long flags,
	const byte *unknown2, const real_point3d *origin, const real_vector3d *vector);

/* 0x00222150; 0x0018cfd0: the portal of a cluster that is nearest a place,
and how far it is; 0x0014b240: how far a place is from a portal */
void __attribute__((stdcall)) sound_query_portal_results(long listener_cluster_index,
	const real_point3d *listener_position, real *results, const real *heard, const real_point3d *position,
	long cluster_index);
long unknown_0018cfd0(long cluster_index, const real_point3d *position, real *distance);
real unknown_0014b240(const byte *structure_bsp, const byte *portal, const real_point3d *position);

/* 0x0021ac30 */
void __attribute__((stdcall)) looping_sound_track_source_got_channel(long data, long sound_index,
	long source_index, void *unknown5C);

/* 0x0021acc0 */
boolean __attribute__((stdcall)) looping_sound_track_source_heard(long data, long sound_index,
	void *unknown5C, byte *parameters);

/* 0x0021adb0 */
void __attribute__((stdcall)) looping_sound_track_source_deleted(long data, long source_index,
	long reason);

/* 0x0021d060 */
void looping_sound_check_distances(long definition_index, const struct sound_source *source);

/* 0x0021d110 */
long __attribute__((stdcall)) sound_effect_source_new(byte *request, long sound_index);

/* 0x0021bfb0, 0x0021c140 and 0x0021cc60 */
boolean __attribute__((stdcall)) looping_sound_source_unknown0(long data, long sound_index, void *unknown5C,
	void *unknown18);
boolean __attribute__((stdcall)) looping_sound_source_matches(const void *unknown5C, const void *unknown14);
boolean __attribute__((stdcall)) looping_sound_detail_request(const byte *detail, long looping_sound_index,
	byte *request);

/* 0x0021ce60 */
void looping_sound_detail_random_position(const byte *detail, real_vector3d *position);

/* 0x0021e410 */
struct sound_console_format;
void sound_console_get_format(long kind, struct sound_console_format *format, void **unknown);

/* 0x00221980 */
void __attribute__((stdcall)) sound_classes_set_decibels(const char *name, real decibels, real seconds);

/* 0x00126df0, 0x00127e20, 0x00129f20 and 0x0012ac20 */
void __attribute__((stdcall)) sound_sources_cross_fade(long out_source_index, long in_source_index, short kind,
	real seconds);
real sound_source_unknown_00127e20(const struct sound_source *source, long listener_index);
void __attribute__((stdcall)) sound_channel_queue_chunk(short channel_index, const struct sound_permutation *permutation,
	short chunk);
real __attribute__((stdcall)) sound_source_distance_gain(long sound_index, const struct sound_source *source,
	real distance);

/* 0x0012ad30 */
real __attribute__((stdcall)) sound_source_spatial_gain(const real_vector3d *offset,
	const struct sound_source *source, long sound_index, const real_vector3d *forward);

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
void __attribute__((stdcall)) sound_source_ring_fade_out_in_seconds(real seconds, long first_source_index);
boolean __attribute__((stdcall)) looping_sound_get_source(long looping_sound_index, long unused, long unused2,
	struct sound_source *source);
long __attribute__((stdcall)) looping_sound_find(long unknown4, long unknown8);

/* 0x002ae7a0, 0x0037c705, 0x002201f0 and 0x0021fa80: of the console's
sound; 0x00249d60: something of two clusters of a structure bsp */
void unknown_002ae7a0(void *voice, real unknown, real unknown2);
void unknown_0037c705(void *unknown, void *stream);
void __attribute__((stdcall)) sound_console_voice_apply_effects(long voice_index, const byte *effects, boolean all);
/* 0x0037b813 and 0x0037b809: the console is told of the filter of a stream,
and of two other things of it */
struct sound_console_effect_description;
void unknown_0037b813(void *stream, const struct sound_console_effect_description *description);
void unknown_0037b809(void *stream, const struct sound_console_effect_description *description);
void __attribute__((stdcall)) sound_console_voice_set_parameters(const byte *parameters, long voice_index,
	const byte *effects, boolean keep_pitch);
/* 0x0037b83f and 0x0037b7e3: the console gives where a stream is heard;
0x0037b7ff and 0x0037b804: it is told how loud a stream is, and how high;
0x0037b71b: something stops */
void unknown_0037b83f(void *stream, long *mixbins);
void unknown_0037b7e3(void *stream, long *mixbins);
void unknown_0037b7ff(void *stream, long millibels);
void unknown_0037b804(void *stream, long pitch);
void unknown_0037b71b(void *buffer, long unknown, long unknown2, long unknown3);
real unknown_00249d60(long cluster_index, long other_cluster_index, const void *structure_bsp);

/* 0x0021f8a0 and 0x00221da0 */
void __attribute__((stdcall)) sound_console_voice_start(const byte *parameters, const byte *effects, long voice_index);
boolean __attribute__((stdcall)) sound_source_unknown_00221da0(long listener_index, const struct sound_source *source,
	real unknown);

/* 0x0018d090 and 0x0018d170: what a tag has of the sound effects;
0x0018cbc0: what the game says of the sound of something */
const byte *unknown_0018d090(long tag_index, long unknown);
const byte *sound_get_gestalt_unknown30(long tag_index);
void unknown_0018cbc0(long unknown, struct sound_source *source);

/* 0x0021d2c0, 0x0021db80 and 0x0021dd30 */
long __attribute__((stdcall)) sound_effect_new(long unknown, real unknownC, short unknown6);
void sound_effect_get_source(long effect_index, struct sound_source *source);
boolean __attribute__((stdcall)) sound_effect_definition_get(long tag_index, long unknown, const byte **definition);

/* 0x00219110; 0x0021b940: makes the sound that plays of a track of a
looping sound */
long __attribute__((stdcall)) sound_pitch_range_choose_permutation(const struct sound_definition *sound,
	short pitch_range_offset, unsigned long *played, long last_permutation, unsigned long *seed, boolean *all_played,
	boolean unknown);
long __attribute__((stdcall)) looping_sound_source_new(long looping_sound_index, long sound_index, long pitch_range,
	long track, long kind, long seed_address);
/* 0x000a5750: says yes */
boolean __attribute__((stdcall)) unknown_000a5750(const void *unknown5C, const void *unknown14);

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

/* 0x0021e0b0 */
long sound_voice_matches_source(const byte *voice, const byte *source);

/* 0x00218f50 */
short sound_choose_pitch_range(short pitch_range_offset, const struct sound_definition *sound, real pitch);

/* 0x00127d00 and 0x0012a6d0 */
long __attribute__((stdcall)) sound_source_get_listener(const struct sound_source *source, real maximum_distance,
	real *distance);
real __attribute__((stdcall)) sound_unknown_0012a6d0(short kind, real time, real duration);

/* 0x00128a60 and 0x00128b90 */
short __attribute__((stdcall)) sound_source_choose_channel(long source_index, short channel_count,
	const short *channel_indices);
long __attribute__((stdcall)) sound_source_goes_before(long other_source_index, long source_index, real distance);

/* 0x00128920 */
short __attribute__((stdcall)) sound_source_find_channel(long source_index, long *reason);

/* 0x00128500 */
void __attribute__((stdcall)) sound_source_find_same(struct sound_same_channels *same, long source_index);

/* 0x00128700 */
short __attribute__((stdcall)) sound_source_get_channel(long source_index, long *reason);

/* 0x00129aa0: works out how the sound of a channel is heard; 0x0021c400
and 0x00128ca0: give it to the voice of the channel */
void __attribute__((stdcall)) sound_channel_compute_parameters(short channel_index, real *decibels, byte *parameters);
void __attribute__((stdcall)) looping_sound_channel_update_voice(short channel_index, real decibels,
	const byte *parameters);
void __attribute__((stdcall)) sound_channel_update_voice(short channel_index, real decibels, const byte *parameters);

/* 0x0021f720: of the console's sound, with what sound_unknown_00222930 fills */
void __attribute__((stdcall)) sound_console_stream_set_function(const byte *environment);
/* 0x0037b7b3: the console gives where to write some bytes of a stream, and
how many; 0x0037b797: the console is told something of a stream */
void unknown_0037b7b3(void *stream, unsigned long offset, unsigned long size, byte **bytes, unsigned long *bytes_size,
	byte **more_bytes, unsigned long *more_bytes_size, unsigned long flags);
void unknown_0037b797(void *stream, long unknown);

/* 0x0037b75b: of the console: what a stream of sound does; 0x0021de90 */
void unknown_0037b75b(void *stream, long *status);
long sound_voice_get_for_source(const byte *source);

/* 0x001295e0 */
boolean __attribute__((stdcall)) sound_channel_get_voice(short channel_index);

/* 0x001286b0, 0x00127fc0 and 0x0012abe0 */
void __attribute__((stdcall)) sound_source_set_sound(long source_index, long sound_index, char pitch_range,
	char permutation, short chunk);
boolean sound_source_unknown_00127fc0(long source_index);
real_point3d *sound_source_get_position(const struct sound_source *source, long listener_index, real_point3d *position);

/* 0x00126360, 0x00126400, 0x00127320 and 0x00127390 */
void sound_source_stop(long source_index);
void sound_stop_all(void);
void __attribute__((stdcall)) sound_source_delete(long source_index, long reason);
void sound_source_free(long source_index);

/* 0x0044a1c0: who is told of the sounds of a sound effect: the effect
itself */
const struct sound_source_callbacks sound_effect_callbacks=
{
	sound_effect_source_unknown0,
	sound_effect_source_got_channel,
	sound_effect_source_playing,
	sound_effect_source_heard,
	sound_effect_source_deleted,
	sound_effect_source_stopped,
	NULL,
	NULL
};

/* 0x0044a1e0: who is told of the sounds that a sound effect starts by
itself: only the first thing, which the effect answers */
const struct sound_source_callbacks sound_effect_sound_callbacks=
{
	sound_effect_sound_unknown0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/* 0x0044a200 and 0x0044a20c: what a looping sound of a sound effect is
started with, by what the effect is at (0 to 2), and by a byte of the
element of unknown_004ed28c of an effect of the kind 2 */
const long sound_effect_looping_sound_states[3]= { 0, 1, 2 };
const long sound_effect_looping_sound_unknown_states[4]= { 0, 1, 2, 2 };

/* 0x0044a110: who is told of the sounds of the tracks of a looping sound:
the looping sound itself. Any two of them are the same for
unknown_000a5750, which only says yes */
const struct sound_source_callbacks looping_sound_track_callbacks=
{
	(boolean (__attribute__((stdcall)) *)(long, long, void *, void *))looping_sound_get_source,
	looping_sound_track_source_got_channel,
	NULL,
	looping_sound_track_source_heard,
	looping_sound_track_source_deleted,
	NULL,
	unknown_000a5750,
	NULL
};

/* 0x0044a250: the effect of the processor of the console's sound that each
of the two places that are heard has */
const long sound_console_place_effects[NUMBER_OF_SOUND_CONSOLE_PLACES]= { 9, 8 };

/* 0x0044a130: who is told of the sounds of a looping sound that play now and
then: the looping sound itself, which says where they are and which are the
same */
const struct sound_source_callbacks looping_sound_callbacks=
{
	looping_sound_source_unknown0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	looping_sound_source_matches,
	NULL
};

/* 0x0044a2a0: how a place with no sound of its own sounds: silence. As the
tags have it: 8 bytes, and the twelve reals that sound_console_set_reverb
takes; the rest is not known */
const unsigned long sound_environment_default[SOUND_ENVIRONMENT_SIZE/4]=
{
	0, 0x8000,
	0xC2C80000, 0xC2C80000, 0, 0x3F800000, 0x3F800000, 0xC2C80000, 0, 0xC2C80000, 0, 0x3F800000, 0x3F800000, 0x459C4000,
	0, 0, 0, 0
};

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
		sound_cache_initialize();

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
			if (sound_playback_controllers_initialize() && looping_sounds_initialize() && sound_effects_initialize() &&
				sound_voices_initialize(&sound_configuration) && sound_console_initialize(&sound_configuration))
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
		sound_console_dispose();
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
				long leaf_index= bsp3d_test_point(global_bsp3d, 0, &listener->matrix.position);

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
				sound_console_update();
				sound_update_time();
				sound_channels_update();
				sound_channels_update_sources();
				sound_console_commit();
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
	unsigned long result= sound_cache_chunk_request(sound_index,
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
				sound_cache_chunk_request(sound_index, sound_get_chunk(sound->first_pitch_range_index, 0, 0),
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
		sound_console_voice_stop(channel->unknownC, offset);
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
		if (!((sound_cache_chunk_request(NONE,
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

	return gain_decibels + (sound_class_get_decibels(sound->sound_class) + decibels);
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
		sound_console_update();
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
				sound_update_time();
				globals= game_sound_globals;
			}
			sound_classes_update(*(real *)(globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET));
			sound_listeners_update(TRUE);
			sound_playback_controllers_update();
			looping_sounds_update(game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET]);
			sound_channels_update();
			sound_sources_update();
			sound_sources_get_channels();
			memset(first, 0, sizeof(first));
			memset(second, 0, sizeof(second));
			listeners.count= 0;
			sound_channels_build_queries(first, second, &listeners);
			for (player_index= players_first_local_player(); player_index!=NONE;
				player_index= sound_next_local_player(player_index))
			{
				sound_listener_compute_queries(player_index, &listeners);
			}
			sound_channels_unknown_00129380((const long *)second, (const long *)first);
			sound_channels_update_sources();
			if (game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7E_OFFSET])
			{
				game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET]++;
			}
		}
		sound_console_commit();
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
is a guess. The game leaves in eax the place that it was given there, and
sound_channel_update_slot goes on to use it: it is given back */
real_point3d *sound_source_get_position(
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
		result[2]= *(const long *)&listeners[listener_index].matrix.position.z;
		break;
	}
	}

	return position;
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
			sound_console_voices_stop();
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

		sound_playback_controller_initialize(controller, owner, random_seed_from_time(), unknown);
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
real of each local player (what sound_listener_doppler_pitch says of it and the
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

		for (player_index= players_first_local_player(); player_index!=NONE; player_index= players_next_local_player(player_index))
		{
			*(real *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET+player_index*4)=
				(real)sound_listener_doppler_pitch(player_index, (const real_point3d *)((const byte *)source+SOUND_SOURCE_POSITION_OFFSET));
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
sound_effect_start_sounds is given it with a 2, and then its bit 0 is set */
void sound_effect_unknown_0021d360(
	long effect_index)
{
	byte *effect= sound_effect_get(effect_index);

	effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= SOUND_EFFECT_DATUM_UNKNOWN_FLAGS;
	sound_effect_start_sounds(effect_index, 2);
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
	sound_effect_start_sounds(effect_index, 2);
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

	return *(const long *)(definition+looping_sound_unknown_0044a188[(long)next]+4)!=NONE;
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

		if (looping_sound_unknown_0044a16c[(long)current]==next || looping_sound_unknown_0044a150[(long)current]==next)
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

/* 0x00125f70: a sound starts to play, with what the game gives to start
it: sound_start_test says whether it is to, and which listener hears it;
sound_promotion_update, whether its promotion keeps it from playing; and
sound_source_create makes it. Gives the sound that plays; NONE if it does not,
and then why, if it is asked: what sound_start_test says, 2 if its promotion
keeps it, and 3 if it could not be made. The name is a guess */
long __attribute__((stdcall)) sound_source_new(
	long sound_index,
	long *failure,
	byte *request)
{
	long source_index= NONE;

	if (sound_available_inline())
	{
		long first;
		long second= (long)(uintptr_t)request;

		if (sound_start_test(request, sound_index, &first, failure))
		{
			if (sound_promotion_update(sound_index, &second))
			{
				if (failure)
				{
					*failure= _sound_source_new_failure_unknown2;
				}
			}
			else
			{
				source_index= sound_source_create(sound_index, first, request, second);
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
		sound_cache_chunk_request(NONE,
			sound_get_chunk(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET],
				(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET],
				*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)),
			_sound_cache_request_unknown4);
	}
}

/* 0x00129380: each channel that plays and is not stopped for a while is
given, by sound_channel_update_slot, a number of one of two tables: that of its slot,
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
				sound_channel_update_slot((short)channel_index, (const real *)(uintptr_t)slot_values[channel->unknownE]);
			}
			else if (channel->unknownC!=NONE)
			{
				sound_channel_update_slot((short)channel_index, (const real *)(uintptr_t)voice_values[channel->unknownC]);
			}
		}
	}
}

/* 0x0021f6d0: the sound of the places of the map is set: two pairs of
numbers and 0x30 bytes, kept in what the game has of the console's sound,
and then sound_console_set_environment_handler with what is at 0x2AB4 of it */
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
	sound_console_set_environment_handler(
		*(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET));
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
		real height= listener->matrix.position.z-place[2];
		real limited;
		real y;
		real x;

		if (!(char)bytes[SOUND_SOURCE_POSITION_KIND_OFFSET])
		{
			y= listener->matrix.position.y-place[1];
			x= listener->matrix.position.x-place[0];

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
		y= place[1]-listener->matrix.position.y;
		x= place[0]-listener->matrix.position.x;

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
	sound_cache_chunk_request(NONE, queued, _sound_cache_request_unknown4);
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
sound_effect_unknown_0021d5a0 says are given to sound_effect_start_sounds with a 1 */
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
			sound_effect_start_sounds(effect_index, 1);
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

/* 0x0021f8a0: a voice of the console's sound starts with what
sound_channel_build_voice_parameters fills: its sound playback controller,
and its slot, whose stream it is given. `effects` is what is at 0x48 of it:
the game has it in ebx, and sound_console_voice_apply_effects takes it there. What the
functions of the console that it calls do is not known */
void __attribute__((stdcall)) sound_console_voice_start(
	const byte *parameters,
	const byte *effects,
	long voice_index)
{
	byte *voice= unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_OFFSET+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE;
	long controller_index;
	short slot;

	voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown4_bit);
	controller_index= *(const long *)(parameters+SOUND_VOICE_START_CONTROLLER_INDEX_OFFSET);
	*(long *)(voice+VOICE_CONTROLLER_INDEX_OFFSET)= controller_index;
	*(long *)(voice+VOICE_UNKNOWN2C_OFFSET)= 0;
	unknown_0037b822(*(void **)(voice+VOICE_UNKNOWN24_OFFSET), controller_index!=NONE);
	unknown_002ae7a0(voice, 0.0f, 0.1f);
	slot= *(const short *)(parameters+SOUND_VOICE_START_SLOT_OFFSET);
	if (slot!=NONE)
	{
		unknown_0037c705(*(void **)(voice+VOICE_UNKNOWN24_OFFSET),
			*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_SLOT_STREAM_OFFSET+slot*UNKNOWN_0051EBE4_SLOT_SIZE));
		voice[VOICE_SLOT_OFFSET]= parameters[SOUND_VOICE_START_SLOT_OFFSET];
	}
	else
	{
		unknown_0037c705(*(void **)(voice+VOICE_UNKNOWN24_OFFSET), NULL);
		voice[VOICE_SLOT_OFFSET]= 0xFF;
	}
	sound_console_voice_apply_effects(voice_index, effects, 1);
	voice[VOICE_FLAGS_OFFSET]&= ~FLAG(_voice_unknown0_bit);
	sound_console_voice_set_parameters(parameters, voice_index, NULL, 0);
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
anything at 8. FALSE if there is no such block. What sound_get_gestalt_unknown30 gives
is only looked at when the number is NONE, and it changes nothing */
boolean __attribute__((stdcall)) sound_effect_definition_get(
	long tag_index,
	long unknown,
	const byte **definition)
{
	const byte *first= unknown_0018d090(tag_index, unknown);
	const byte *second= sound_get_gestalt_unknown30(tag_index);
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
	sound_effect_start_sounds(effect_index, 0);

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
there is one; if not, what sound_pitch_range_choose_permutation chooses, with the bits of those
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

		permutation= sound_pitch_range_choose_permutation(sound, pitch_range_offset, &played, sound_pitch_range_get_unknown4(pitch_range, sound),
			&global_random_seed_reference[1], NULL, FALSE);
		sound_pitch_range_set_played_bits(pitch_range, sound, played);
	}
	sound_pitch_range_set_unknown4(pitch_range, sound, (char)permutation);

	return (short)permutation;
}

/* 0x0021b910: the sound that plays of a track of a looping sound that is of
a sound and a pitch range; if there is none, looping_sound_source_new makes it */
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
		source_index= looping_sound_source_new(looping_sound_index, sound_index, pitch_range, track, unknown, unknown2);
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

/* whether two reals are closer than 0.0001, with the precision of the x87 */
static inline __attribute__((always_inline)) boolean sound_reals_match(
	real a,
	real b)
{
	long double difference= (long double)a-(long double)b;

	if (difference<0.0L)
	{
		difference= -difference;
	}

	return 0.0001f>difference;
}

/* 0x0021e0b0: whether a sound that plays can share a sound voice: the same
who is told of it, the same flag, classes that match, and then, with someone
to tell, the same thing to tell with and what its seventh function says of
the two; with no one, the same place in the structure bsp and the same six
reals. Never while unknown_0055e780 is set */
long sound_voice_matches_source(
	const byte *voice,
	const byte *source)
{
	const struct sound_source_callbacks *callbacks= *(const struct sound_source_callbacks *const *)(voice+SOUND_VOICE_DATUM_CALLBACKS_OFFSET);

	if (callbacks!=*(const struct sound_source_callbacks *const *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET) ||
		voice[SOUND_VOICE_DATUM_UNKNOWN8_OFFSET]!=(source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]&1) ||
		!(byte)sound_classes_match((char)voice[SOUND_VOICE_DATUM_CLASS_OFFSET],
			sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class))
	{
		return FALSE;
	}
	if (callbacks)
	{
		if (*(const long *)(voice+SOUND_VOICE_DATUM_CALLBACK_DATA_OFFSET)!=
			*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET) ||
			!callbacks->matches ||
			!callbacks->matches(source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, voice+SOUND_VOICE_DATUM_UNKNOWN14_OFFSET))
		{
			return FALSE;
		}
	}
	else
	{
		static const byte offsets[]= { 0x0, 0x4, 0x8, 0x10, 0x14, 0x18 };
		long index;

		if (*(const short *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET+4)!=*(const short *)(voice+SOUND_VOICE_DATUM_LOCATION_OFFSET+4) ||
			*(const long *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET)!=*(const long *)(voice+SOUND_VOICE_DATUM_LOCATION_OFFSET))
		{
			return FALSE;
		}
		for (index= 0; index<(long)sizeof(offsets); index++)
		{
			if (!sound_reals_match(*(const real *)(source+SOUND_SOURCE_DATUM_POSITION_OFFSET+offsets[index]),
				*(const real *)(voice+SOUND_VOICE_DATUM_UNKNOWN48_OFFSET+offsets[index])))
			{
				return FALSE;
			}
		}
	}

	return !unknown_0055e780;
}

/* the parameters of a pitch range, of those that the sounds of a map share:
five numbers of 16 bits, pitches in hundredths of a semitone */
static inline __attribute__((always_inline)) const short *sound_pitch_range_parameters(
	const struct sound_pitch_range *pitch_range)
{
	return (const short *)((const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown8[2].address+
		*(const short *)(pitch_range->unknown0+SOUND_PITCH_RANGE_PARAMETERS_INDEX_OFFSET)*SOUND_PITCH_RANGE_PARAMETERS_SIZE);
}

/* whether a pitch is one of those that a pitch range is played at: from its
fourth number to its fifth */
static inline __attribute__((always_inline)) boolean sound_pitch_range_has_pitch(
	const short *parameters,
	real pitch)
{
	real clamped= (real)parameters[3];

	if (!(clamped>pitch))
	{
		clamped= (real)parameters[4];
		if (!(pitch>clamped))
		{
			clamped= pitch;
		}
	}

	return clamped==pitch;
}

/* 0x00218f50: the pitch range of a sound for a pitch. The one that is
asked for, if it has the pitch and recordings; if not, the first with
recordings that has the pitch; and if none has it, the one whose second and
third numbers are closest to the pitch. NONE if no pitch range has
recordings */
short sound_choose_pitch_range(
	short pitch_range_offset,
	const struct sound_definition *sound,
	real pitch)
{
	const struct sound_pitch_range *pitch_ranges=
		(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address;
	long count= (char)sound->pitch_range_count;
	long closest= NONE;
	real closest_distance= SOUND_REAL_MAXIMUM;
	short index;

	if (pitch_range_offset!=NONE && pitch_range_offset<count)
	{
		const struct sound_pitch_range *pitch_range= &pitch_ranges[sound->first_pitch_range_index+pitch_range_offset];

		if (sound_pitch_range_has_pitch(sound_pitch_range_parameters(pitch_range), pitch) && pitch_range->permutation_count>0)
		{
			return pitch_range_offset;
		}
	}
	for (index= 0; index<count; index++)
	{
		const struct sound_pitch_range *pitch_range= &pitch_ranges[sound->first_pitch_range_index+index];

		if (pitch_range->permutation_count>0)
		{
			const short *parameters= sound_pitch_range_parameters(pitch_range);
			real distance;

			if (sound_pitch_range_has_pitch(parameters, pitch))
			{
				return index;
			}
			if (pitch>(real)parameters[2])
			{
				distance= pitch-(real)parameters[2];
			}
			else
			{
				distance= (real)parameters[1]-pitch;
			}
			if (closest_distance>distance)
			{
				closest= index;
				closest_distance= distance;
			}
		}
	}

	return (short)closest;
}

/* 0x00219110: a recording of a pitch range of a sound, at random, that was
not played since all were: the bits of those that were played are kept, and
when all have been, they start again from the one that was played last.
Each recording says how often it is passed over (the word at 2 of it, of
65535; or the rest to 1, for those from a half on, if it is asked so); one
that is passed over stays as played, and another is looked for, up to twice
as many times as there are recordings. It can be told whether all have been
played now */
long __attribute__((stdcall)) sound_pitch_range_choose_permutation(
	const struct sound_definition *sound,
	short pitch_range_offset,
	unsigned long *played,
	long last_permutation,
	unsigned long *seed,
	boolean *all_played,
	boolean unknown)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const struct sound_pitch_range *pitch_range=
		(const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+pitch_range_offset);
	long count= pitch_range->permutation_count;
	long tries= 0;
	unsigned long permutation;

	if (all_played)
	{
		*all_played= FALSE;
	}
	for (;;)
	{
		unsigned long bit;
		real skip;

		*seed= *seed*1664525UL+1013904223UL;
		permutation= ((*seed>>16)*(unsigned long)count)>>16;
		if (!(~*played&((1UL<<(pitch_range->permutation_count&31))-1)))
		{
			*played= 0;
			if (pitch_range->permutation_count>1 && (short)last_permutation!=NONE)
			{
				*played= 1UL<<(last_permutation&31);
			}
			last_permutation= NONE;
		}
		bit= 1UL<<((short)permutation&31);
		if (*played&bit)
		{
			continue;
		}
		*played|= bit;
		if (all_played && !(~*played&((1UL<<(count&31))-1)))
		{
			*all_played= TRUE;
		}
		skip= (real)(long)*(const word *)((const byte *)(uintptr_t)gestalt->permutations.address+
			(pitch_range->first_permutation_index+(short)permutation)*(long)sizeof(struct sound_permutation)+
			SOUND_PERMUTATION_SKIP_FRACTION_OFFSET)*(1.0f/65535.0f);
		if (unknown && !(skip<0.5f))
		{
			skip= 1.0f-skip;
		}
		*seed= *seed*1664525UL+1013904223UL;
		if ((long double)(*seed>>16)*(long double)(1.0f/65535.0f)>=(long double)skip)
		{
			break;
		}
		tries++;
		if (tries==count+count)
		{
			break;
		}
	}

	return (long)permutation;
}

/* 0x00127d00: the listener that hears a sound, of what the game says of
it, and how far it is. For a sound with no place: the first local player,
at no distance. For one with a place: the closest listener, by
sound_source_unknown_00127e20, unless sound_source_unknown_00221da0 says no
of it; then, if neither of two flags of the sound is set, there is none
beyond a distance that the function is given; and if either is set and
there is none, the first local player. NONE if no one hears it */
long __attribute__((stdcall)) sound_source_get_listener(
	const struct sound_source *source,
	real maximum_distance,
	real *distance)
{
	const byte *bytes= (const byte *)source;
	long listener_index= NONE;
	real result;

	if ((bytes[SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)!=_sound_source_datum_type_positioned)
	{
		listener_index= sound_next_local_player(NONE);
		result= 0.0f;
	}
	else
	{
		const struct sound_listener *listeners=
			(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET);
		real closest= SOUND_REAL_MAXIMUM;
		long index;

		for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
		{
			if (listeners[index].valid)
			{
				real square= sound_source_unknown_00127e20(source, index);

				if (closest>square)
				{
					listener_index= index;
					closest= square;
				}
			}
		}
		result= (real)sqrtl(closest);
		if (sound_source_unknown_00221da0(listener_index, source, (real)(sqrtl(closest)/(long double)maximum_distance)))
		{
			listener_index= NONE;
		}
		if (!(source->flags&(FLAG(_sound_source_unknown8_bit)|FLAG(_sound_source_unknown11_bit))))
		{
			if (closest>maximum_distance*maximum_distance)
			{
				listener_index= NONE;
				result= SOUND_REAL_MAXIMUM;
			}
		}
		else if (listener_index==NONE)
		{
			listener_index= sound_next_local_player(NONE);
		}
	}
	if (distance)
	{
		*distance= result;
	}

	return listener_index;
}

/* 0x0012a6d0: how loud something that comes up from silence is, in
decibels, at a time of how long it takes: the part of it that has gone by,
from 0 to 1 (and from 1 to 0 if it takes less than nothing), along the gains
or with a square root, by its kind. For any other kind, how long it takes */
real __attribute__((stdcall)) sound_unknown_0012a6d0(
	short kind,
	real time,
	real duration)
{
	real fraction= (real)fabsl((long double)time/(long double)duration);

	if (fraction>1.0f)
	{
		fraction= 1.0f;
	}
	if (0.0f>duration)
	{
		fraction= 1.0f-fraction;
	}
	switch (kind)
	{
	case 0:
		return sound_decibels_interpolate(_sound_interpolation_gain, SOUND_MINIMUM_DECIBELS, 0.0f, fraction);
	case 1:
		return sound_decibels_interpolate(_sound_interpolation_gain_square_root, SOUND_MINIMUM_DECIBELS, 0.0f, fraction);
	}

	return duration;
}

/* the class of the sound of a sound that plays */
static inline __attribute__((always_inline)) const byte *sound_source_get_class(
	const byte *source)
{
	return (const byte *)sound_class_get(
		sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class);
}

/* 0x00128a60: of some channels, the one that a sound that is to play takes
from the sound that it plays: the one whose sound has played the longest, of
those that have played as long as the class of the new sound says at least;
or one whose sound has less of what is at 0xA0 than the new one. The last
one that can be taken is the one; NONE if none can */
short __attribute__((stdcall)) sound_source_choose_channel(
	long source_index,
	short channel_count,
	const short *channel_indices)
{
	const byte *source= sound_source_get(source_index);
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	long longest= 0;
	short chosen= NONE;
	long time;
	long index;

	if (channel_count<=0)
	{
		return NONE;
	}
	time= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
	for (index= 0; index<(word)channel_count; index++)
	{
		short channel_index= channel_indices[index];
		const byte *playing= sound_source_get(sound_channels[channel_index].unknown0);
		long played= time-*(const long *)(playing+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET);

		if ((played>=*(const long *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN4_OFFSET) &&
			played>longest) ||
			(char)source[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET]>(char)playing[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET])
		{
			chosen= channel_index;
			longest= played;
		}
	}

	return chosen;
}

/* 0x00128b90: whether a sound that plays goes before another: if its class
has more at 0xC; with as much, and both of the same sound, if it has more at
0xA0; and with as much of that, or of different sounds, if the other is
farther from its listener than a distance, as sound_source_unknown_00127e20
measures it */
long __attribute__((stdcall)) sound_source_goes_before(
	long other_source_index,
	long source_index,
	real distance)
{
	const byte *other= sound_source_get(other_source_index);
	const byte *source= sound_source_get(source_index);
	long priority= *(const short *)(sound_source_get_class(source)+SOUND_CLASS_PRIORITY_OFFSET);
	long other_priority= *(const short *)(sound_source_get_class(other)+SOUND_CLASS_PRIORITY_OFFSET);

	if (priority>other_priority)
	{
		return TRUE;
	}
	if (priority!=other_priority)
	{
		return FALSE;
	}
	if (*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)==*(const long *)(other+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))
	{
		char unknown= (char)source[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET];
		char other_unknown= (char)other[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET];

		if (unknown>other_unknown)
		{
			return TRUE;
		}
		if (unknown!=other_unknown)
		{
			return FALSE;
		}
	}

	return sound_source_unknown_00127e20((const struct sound_source *)(other+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET),
		(char)other[SOUND_SOURCE_DATUM_LISTENER_OFFSET])>distance;
}

/* how far a sound that plays is from its listener, as
sound_source_unknown_00127e20 measures it */
static inline __attribute__((always_inline)) real sound_source_listener_distance(
	const byte *source)
{
	return sound_source_unknown_00127e20((const struct sound_source *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET),
		(char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]);
}

/* 0x00128920: a channel for a sound that is to play, of those of the type
of its sound: the first that plays nothing; if all play, the one whose sound
the new one goes before, and of those, the one whose sound all the rest go
before. NONE if there is none. It says too why a channel that plays was
taken: 9; 0 if none was */
short __attribute__((stdcall)) sound_source_find_channel(
	long source_index,
	long *reason)
{
	const byte *source= sound_source_get(source_index);
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	real distance= sound_source_listener_distance(source);
	real taken_distance= 0.0f;
	long taken_source_index= NONE;
	short taken= NONE;
	short channel_index;

	*reason= 0;
	for (channel_index= 0;
		channel_index<*(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		const struct sound_channel *channel= &sound_channels[channel_index];

		if (sound->unknown3[1]!=(byte)channel->type)
		{
			continue;
		}
		if (channel->unknown0==NONE)
		{
			return channel_index;
		}
		if ((byte)sound_source_goes_before(channel->unknown0, source_index, distance) &&
			(taken==NONE || (byte)sound_source_goes_before(channel->unknown0, taken_source_index, taken_distance)))
		{
			*reason= _sound_source_deleted_channel_taken;
			taken_source_index= channel->unknown0;
			taken= channel_index;
			taken_distance= sound_source_listener_distance(sound_source_get(channel->unknown0));
		}
	}

	return taken;
}

/* 0x00128500: the channels that play the sound of a sound that is to play,
and of those, the ones that play it for the same thing: with the same who is
told, and the same to tell with or what its eighth function says of the two.
With each list goes how many of them the class of the sound lets there be.
It is said too whether one of the second list started to play this very
time */
void __attribute__((stdcall)) sound_source_find_same(
	struct sound_same_channels *same,
	long source_index)
{
	const byte *source= sound_source_get(source_index);
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	short channel_index;

	same->started_now= FALSE;
	same->sound_count= 0;
	same->instance_count= 0;
	same->sound_limit= ((const short *)sound_class_get(sound->sound_class))[0];
	same->instance_limit= ((const short *)sound_class_get(sound->sound_class))[1];
	for (channel_index= 0;
		channel_index<*(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		const struct sound_channel *channel= &sound_channels[channel_index];
		const struct sound_source_callbacks *callbacks;
		const byte *playing;
		long data;
		long playing_data;

		if (channel->unknown0==NONE || channel->unknown0==source_index)
		{
			continue;
		}
		playing= sound_source_get(channel->unknown0);
		if (sound->unknown3[1]!=(byte)channel->type ||
			*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)!=*(const long *)(playing+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))
		{
			continue;
		}
		same->sound_channels[same->sound_count]= channel_index;
		same->sound_count++;
		data= *(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET);
		playing_data= *(const long *)(playing+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET);
		callbacks= *(const struct sound_source_callbacks *const *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);
		if (data==NONE || playing_data==NONE ||
			callbacks!=*(const struct sound_source_callbacks *const *)(playing+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET))
		{
			continue;
		}
		if (data!=playing_data)
		{
			if (!callbacks->same || !callbacks->same(data, source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, playing_data,
				playing+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET))
			{
				continue;
			}
		}
		same->instance_channels[same->instance_count]= channel_index;
		same->instance_count++;
		if (*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)==
			*(const long *)(playing+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET))
		{
			same->started_now= TRUE;
		}
	}
}

/* whether the class of the sound of a sound that plays is one of those of
which the same thing plays only one sound at a time */
static inline __attribute__((always_inline)) boolean sound_definition_class_is_exclusive(
	const struct sound_definition *sound)
{
	return (sound_class_get(sound->sound_class)->flags>>_sound_class_unknown1_bit)&1;
}

/* 0x00128700: the channel that a sound that is to play gets; NONE if it
gets none, and why. The one that it has, if it has one. A sound of a class
of which the same thing plays only one at a time takes the place of the one
that the thing plays: in the same channel, if it is of its type (reason 13);
if not, the other sound and its channel are let go, and a channel is looked
for. Any other sound gets none if one like it started this very time (5);
with as many like it as its class lets there be, or as many of its sound, it
takes the channel of one of them (5 and 6); and if not, a channel is looked
for. The name is a guess */
short __attribute__((stdcall)) sound_source_get_channel(
	long source_index,
	long *reason)
{
	byte *source= sound_source_get(source_index);
	const struct sound_definition *sound;
	struct sound_same_channels same;

	*reason= 0;
	if (*(const short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)!=NONE)
	{
		return *(const short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET);
	}
	sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	if (sound_definition_class_is_exclusive(sound) && *(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET)!=NONE)
	{
		short count= *(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		long replaced_source_index= NONE;
		short replaced_channel_index= NONE;
		short channel_index;

		for (channel_index= 0; channel_index<count; channel_index++)
		{
			const struct sound_channel *channel= &sound_channels[channel_index];
			long playing_index= channel->unknown0;

			if (playing_index!=NONE)
			{
				const byte *playing= sound_source_get(playing_index);

				if (*(const long *)(playing+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET)==
					*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET) &&
					sound_definition_class_is_exclusive(
						sound_definition_get(*(const long *)(playing+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))))
				{
					if (sound->unknown3[1]==(byte)channel->type)
					{
						source[SOUND_SOURCE_DATUM_TYPE_OFFSET]= (source[SOUND_SOURCE_DATUM_TYPE_OFFSET]&~SOUND_SOURCE_DATUM_TYPE_MASK)|
							(playing[SOUND_SOURCE_DATUM_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK);
						*reason= _sound_source_deleted_replaced;
						goto replace;
					}
					replaced_channel_index= channel_index;
					replaced_source_index= playing_index;
				}
			}
		}
		channel_index= sound_source_find_channel(source_index, reason);
		if (channel_index==NONE)
		{
			return NONE;
		}
replace:
		if (replaced_source_index!=NONE)
		{
			sound_source_delete(replaced_source_index, _sound_source_deleted_replaced);
			sound_channel_delete(replaced_channel_index);
		}

		return channel_index;
	}
	sound_source_find_same(&same, source_index);
	if (same.started_now)
	{
		*reason= _sound_source_deleted_too_many_instances;
		return NONE;
	}
	if (same.instance_count>=same.instance_limit)
	{
		*reason= _sound_source_deleted_too_many_instances;
		return sound_source_choose_channel(source_index, same.instance_count, same.instance_channels);
	}
	if (same.sound_count>=same.sound_limit)
	{
		*reason= _sound_source_deleted_too_many_sounds;
		return sound_source_choose_channel(source_index, same.sound_count, same.sound_channels);
	}

	return sound_source_find_channel(source_index, reason);
}

/* 0x00126c30: whether a sound is to start to play, with what the game says
of it (which is 0xC into what the function is given), and if not, why. Not
with no sound, or for a sound of a kind that is not played (5). Not, at
random, as often as its scale and how it is played say that it is skipped
(1). And not if no listener hears it, or it would be heard at -64 decibels
or less and neither of two flags says that it plays all the same (4). It
says too which listener hears it */
boolean __attribute__((stdcall)) sound_start_test(
	const byte *request,
	long sound_index,
	long *listener_index,
	long *reason)
{
	const struct sound_source *source= (const struct sound_source *)(request+SOUND_REQUEST_SOURCE_OFFSET);
	boolean result= FALSE;
	long why= _sound_start_refused_unknown5;

	if (sound_available_inline())
	{
		const struct sound_definition *sound= sound_definition_get(sound_index);

		if ((sound->unknown3[2]==1 && sound->unknown3[1]!=2) || (sound->unknown3[1]==2 && unknown_0047f0e4))
		{
			unsigned long *seed= &global_random_seed_reference[1];
			const real *scale= sound_definition_scale_decibels(sound);
			long double random;
			long double skip;

			*seed= *seed*1664525UL+1013904223UL;
			random= (long double)(*seed>>16)*(long double)(1.0f/65535.0f);
			why= _sound_start_refused_skipped;
			skip= ((long double)scale[3]+((long double)scale[4]-(long double)scale[3])*
				(long double)*(const real *)((const byte *)source+SOUND_SOURCE_SCALE_OFFSET))*
				(long double)*(const real *)sound_definition_get_playback(sound)->unknown8;
			if (random>skip)
			{
				why= _sound_start_refused_not_heard;
				if (sound_definition_unknown_00126bd0(sound_index))
				{
					real distance= sound_get_maximum_distance(source, sound_index);
					long listener= sound_source_get_listener(source, distance, &distance);

					*listener_index= listener;
					if (listener!=NONE)
					{
						real gain= sound_source_distance_gain(sound_index, source, distance);
						real decibels= sound_definition_decibels(sound,
							*(const real *)((const byte *)source+SOUND_SOURCE_DECIBELS_OFFSET),
							*(const real *)((const byte *)source+SOUND_SOURCE_SCALE_OFFSET));
						real gain_decibels= sound_gain_to_decibels(gain);
						real total= sound_definition_get_playback(sound)->gain_base+(decibels+gain_decibels);

						if ((source->flags&(FLAG(_sound_source_unknown8_bit)|FLAG(_sound_source_unknown11_bit))) ||
							total>SOUND_MINIMUM_DECIBELS)
						{
							why= 0;
							result= TRUE;
						}
					}
				}
			}
		}
	}
	if (reason)
	{
		*reason= why;
	}

	return result;
}

/* 0x00126ec0: the promotion of a sound, when the sound is to start: which
of its rules it is at now, and whether the sound is not to play for a while.
Each rule has a time of its own, which goes down as time goes by and up by
what the rule says each time the sound starts; a rule that has a count and
whose time is up to what it says goes on to the next one. A rule that is
reached keeps the sound from playing again, at that rule or a lower one, for
the seconds that it says. NONE, and FALSE, for a sound with no promotion */
long __attribute__((stdcall)) sound_promotion_update(
	long sound_index,
	long *rule_index)
{
	char promotion_index= ((const char *)sound_definition_get(sound_index))[SOUND_DEFINITION_PROMOTION_INDEX_OFFSET];
	uintptr_t address= 0;
	byte *promotion;
	const byte *rules;
	long *timers;
	long time;
	long elapsed;
	long suppressed;
	long rule;
	long index;

	if (promotion_index!=NONE)
	{
		address= (uintptr_t)*(const long *)((const byte *)global_sound_cache_file_gestalt+SOUND_GESTALT_PROMOTIONS_OFFSET+4)+
			(uintptr_t)(promotion_index*SOUND_PROMOTION_SIZE);
	}
	if (!address)
	{
		*rule_index= NONE;
		return FALSE;
	}
	promotion= (byte *)address;
	time= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
	if (*(const long *)(promotion+SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET)<=time)
	{
		*(long *)(promotion+SOUND_PROMOTION_RULE_INDEX_OFFSET)= NONE;
		*(long *)(promotion+SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET)= 0;
	}
	elapsed= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)-
		*(const long *)(promotion+SOUND_PROMOTION_LAST_TIME_OFFSET);
	for (index= 0; index<*(const long *)(promotion+SOUND_PROMOTION_TIMER_COUNT_OFFSET); index++)
	{
		long left;

		timers= *(long *const *)(promotion+SOUND_PROMOTION_TIMER_COUNT_OFFSET+4);
		left= timers[index]-elapsed;
		timers[index]= left<0 ? 0 : left;
	}
	for (rule= 0; rule<*(const long *)promotion; rule++)
	{
		long *timer= *(long *const *)(promotion+SOUND_PROMOTION_TIMER_COUNT_OFFSET+4)+rule;
		const byte *entry= *(const byte *const *)(promotion+4)+rule*SOUND_PROMOTION_RULE_SIZE;

		*timer+= *(const long *)(entry+SOUND_PROMOTION_RULE_STEP_OFFSET);
		if (*(const short *)(entry+SOUND_PROMOTION_RULE_COUNT_OFFSET)<=0 ||
			*timer<*(const long *)(entry+SOUND_PROMOTION_RULE_THRESHOLD_OFFSET) ||
			rule>=*(const long *)promotion-1)
		{
			break;
		}
		*timer= 0;
	}
	*rule_index= rule;
	suppressed= rule<=*(const long *)(promotion+SOUND_PROMOTION_RULE_INDEX_OFFSET) &&
		*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)<
		*(const long *)(promotion+SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET);
	if (rule>=*(const long *)(promotion+SOUND_PROMOTION_RULE_INDEX_OFFSET) && !suppressed)
	{
		rules= *(const byte *const *)(promotion+4);
		*(long *)(promotion+SOUND_PROMOTION_RULE_INDEX_OFFSET)= rule;
		*(long *)(promotion+SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET)=
			(long)(*(const real *)(rules+rule*SOUND_PROMOTION_RULE_SIZE+SOUND_PROMOTION_RULE_SECONDS_OFFSET)*1000.0f+
			(real)*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET));
	}
	*(long *)(promotion+SOUND_PROMOTION_LAST_TIME_OFFSET)= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);

	return suppressed;
}

/* 0x00126000: makes the sound that plays, of a sound, for a listener, with
what the game gives to start it and the rule of its promotion. NONE if it
would be heard at -64 decibels or less, or there is no room. It is given a
gain and a pitch at random; what the game says of it; its sound playback
controller, who is told of it and what with, if the game gives them; and
the pitch range and the recording that it plays: those that the game gives,
or the pitch range of the rule of its promotion, or the one for its pitch,
and a recording that is chosen. A sound that is far (more than 100, by its
distance and unknown_00547f14) and has neither of two flags starts that much
later. Its samples are asked for */
long __attribute__((stdcall)) sound_source_create(
	long sound_index,
	long listener_index,
	byte *request,
	long rule_index)
{
	const struct sound_definition *sound= sound_definition_get(sound_index);
	const struct sound_source *said= (const struct sound_source *)(request+SOUND_REQUEST_SOURCE_OFFSET);
	real gain= sound_definition_random_gain(sound);
	real decibels;
	long source_index;
	byte *source;
	long delay;
	unsigned long *seed;
	unsigned long flags;

	gain= ((signed char)request[0]<0 ? *(const real *)(request+SOUND_REQUEST_GAIN_OFFSET) : 0.0f)+gain;
	decibels= sound_definition_decibels(sound, *(const real *)((const byte *)said+SOUND_SOURCE_DECIBELS_OFFSET),
		*(const real *)((const byte *)said+SOUND_SOURCE_SCALE_OFFSET));
	if (!(decibels+gain>SOUND_MINIMUM_DECIBELS))
	{
		return NONE;
	}
	sound_request_unknown_002226b0(request, sound_index);
	source_index= datum_new(sound_sources);
	if (source_index==NONE)
	{
		return NONE;
	}
	source= sound_source_get(source_index);
	delay= (long)(long long)(sqrtl((long double)sound_source_unknown_00127e20(said, listener_index))*
		(long double)unknown_00547f14);
	source[SOUND_SOURCE_DATUM_KIND_OFFSET]= 0;
	*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)= NONE;
	source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]= (byte)listener_index;
	*(word *)(source+SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET)= *(const word *)(request+SOUND_REQUEST_UNKNOWN4_OFFSET);
	seed= &global_random_seed_reference[1];
	*(real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET)= sound_definition_random_pitch(sound, seed);
	*(real *)(source+SOUND_SOURCE_DATUM_GAIN_OFFSET)= gain;
	*(unsigned long *)(source+SOUND_SOURCE_DATUM_SEED_OFFSET)= *seed;
	*(word *)(source+SOUND_SOURCE_DATUM_FLAGS_OFFSET)=
		request[0]&FLAG(_sound_request_unknown1_bit) ? *(const word *)(request+SOUND_REQUEST_UNKNOWN8_OFFSET) : 0;
	*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET)=
		request[0]&FLAG(_sound_request_callback_data_bit) ? *(const long *)(request+SOUND_REQUEST_CALLBACK_DATA_OFFSET) : NONE;
	memcpy(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET, said, LOOPING_SOUND_DATUM_SOURCE_SIZE);
	if (sound->flags&FLAG(_sound_definition_unknown3_bit))
	{
		source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]|= 1;
	}
	if ((sound_class_get(sound->sound_class)->internal_flags>>_sound_class_internal_unknown2_bit)&1)
	{
		source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]|= 0x80;
	}
	*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET)=
		request[0]&FLAG(_sound_request_unknown6_bit) ? (short)(char)*(const short *)(request+SOUND_REQUEST_UNKNOWN8E_OFFSET) : NONE;
	flags= *(const unsigned long *)request;
	source[SOUND_SOURCE_DATUM_UNKNOWNA4_OFFSET]=
		flags&FLAG(_sound_request_unknown9_bit) ? request[SOUND_REQUEST_UNKNOWN90_OFFSET] : 0xFF;
	flags= *(const unsigned long *)request;
	*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8_OFFSET)=
		flags&FLAG(_sound_request_unknown8_bit) ? *(const long *)(request+SOUND_REQUEST_UNKNOWN98_OFFSET) : NONE;
	if (request[0]&FLAG(_sound_request_controller_bit))
	{
		long controller_index= sound_playback_controller_acquire(*(const long *)(request+SOUND_REQUEST_CONTROLLER_OWNER_OFFSET));

		*(long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET)= controller_index;
		if (controller_index!=NONE)
		{
			sound_playback_controller_add_count(sound_playback_controller_get(controller_index), 1);
		}
	}
	else
	{
		*(long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET)= NONE;
	}
	source[SOUND_SOURCE_DATUM_UNKNOWN3_OFFSET]= 0xFF;
	if (request[0]&FLAG(_sound_request_callbacks_bit))
	{
		word size= *(const word *)(request+SOUND_REQUEST_CALLBACK_SIZE_OFFSET);

		*(const void **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET)= *(const void *const *)(request+SOUND_REQUEST_CALLBACKS_OFFSET);
		memcpy(source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, request+SOUND_REQUEST_CALLBACK_BYTES_OFFSET,
			size>SOUND_REQUEST_CALLBACK_BYTES_SIZE ? SOUND_REQUEST_CALLBACK_BYTES_SIZE : size);
	}
	else
	{
		*(const void **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET)= NULL;
	}
	flags= *(const unsigned long *)request;
	if (flags&FLAG(_sound_request_permutation_bit))
	{
		sound_source_set_sound(source_index, sound_index, (char)*(const word *)(request+SOUND_REQUEST_PITCH_RANGE_OFFSET),
			(char)*(const word *)(request+SOUND_REQUEST_PERMUTATION_OFFSET), 0);
	}
	else
	{
		long pitch_range;
		short permutation;

		if (rule_index!=NONE)
		{
			pitch_range= sound_definition_promotion_rule(rule_index, sound);
		}
		else
		{
			const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
			real lower= (real)bends[0];
			real pitch= ((real)bends[1]-lower)*(*(const real *)((const byte *)said+SOUND_SOURCE_SCALE_OFFSET));

			pitch+= *(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET);
			pitch+= lower;
			pitch_range= sound_choose_pitch_range(NONE, sound, pitch);
		}
		permutation= sound_choose_permutation(sound, (short)pitch_range);
		sound_source_set_sound(source_index, sound_index, (char)pitch_range, (char)permutation, 0);
	}
	source[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET]= (byte)rule_index;
	*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= 0;
	*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= 0;
	if (delay>SOUND_SOURCE_DELAY_MINIMUM &&
		!(*(const word *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET)&(FLAG(_sound_source_unknown8_bit)|FLAG(_sound_source_unknown11_bit))))
	{
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)=
			*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)+delay;
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown0_bit);
	}
	else
	{
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)=
			*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
	}
	sound_source_request_data(source);

	return source_index;
}

/* 0x001914f0: a number for each of six names (the numbers of six texts of
the game, as the tags give them); NONE for any other. It is of another
file, most likely; what the names are is not known */
long sound_unknown_001914f0(
	long name)
{
	switch (name)
	{
	case 0x1300012D:
		return 13;
	case 0x14000135:
		return 14;
	case 0x1600013C:
		return 27;
	case 0x1600013D:
		return 28;
	case 0x1600013E:
		return 29;
	case 0x1600013F:
		return 30;
	}

	return NONE;
}

/* 0x002226b0: what the game gives to start a sound takes two things more
from the tags, for the sound: what unknown_0018d090 or sound_get_gestalt_unknown30 has
for it says a tag, and the first element of a block of that tag has a name.
For one of the names, a number of unknown_00510c90 is taken, if it has one;
and for the names that sound_unknown_001914f0 knows, its number */
void sound_request_unknown_002226b0(
	byte *request,
	long sound_index)
{
	const byte *first= unknown_0018d090(sound_index,
		*(const unsigned long *)request&FLAG(_sound_request_unknown8_bit) ? *(const long *)(request+SOUND_REQUEST_UNKNOWN98_OFFSET) :
		NONE);
	const byte *second= sound_get_gestalt_unknown30(sound_index);
	const byte *chosen= NULL;
	const byte *elements;
	const byte *tag;
	const byte *block;
	long tag_index;
	long number;

	if (second && *(const long *)(second+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)>0)
	{
		chosen= second;
	}
	if (first && *(const long *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)>0)
	{
		chosen= first;
	}
	if (!chosen)
	{
		return;
	}
	elements= *(const long *)(chosen+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)>0 ?
		*(const byte *const *)(chosen+SOUND_EFFECT_COLLECTION_COUNT_OFFSET+4) : NULL;
	tag_index= *(const long *)(elements+4);
	if (tag_index==NONE)
	{
		return;
	}
	tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (*(const long *)(tag+SOUND_ENVIRONMENT_TAG_BLOCK_OFFSET)<=0)
	{
		return;
	}
	block= *(const byte *const *)(tag+SOUND_ENVIRONMENT_TAG_BLOCK_OFFSET+4);
	if (*(const long *)(block+8)==0x1300012D)
	{
		short unknown= *(const short *)(unknown_00510c90+6);

		if (unknown!=NONE)
		{
			*(short *)(request+SOUND_REQUEST_UNKNOWN8E_OFFSET)= unknown;
			*(unsigned long *)request|= FLAG(_sound_request_unknown6_bit);
		}
	}
	number= sound_unknown_001914f0(*(const long *)(block+8));
	if ((short)number!=NONE)
	{
		request[SOUND_REQUEST_UNKNOWN90_OFFSET]= (byte)number;
		*(unsigned long *)request|= FLAG(_sound_request_unknown9_bit);
	}
}

/* how much a real that goes towards another moves in a step: the step, but
no more than what is left, and back by as much if it is less than nothing */
static inline __attribute__((always_inline)) real sound_step_towards(
	real left,
	real step)
{
	real back= 0.0f-step;

	if (back>left)
	{
		return back;
	}

	return left>step ? step : left;
}

/* 0x001269f0: the time of the sound goes on, by the clock of the console:
its thousandths of a second, the seconds of this time, and the two reals
that count seconds for sound_set_unknown1f8. And the real at 0x1E4 follows
the game, in a campaign that is going on: it goes down to 0 while the game
is paused, after a wait and as fast as two numbers of the sound of the
places of the map say, and back up to 1 when it goes on, as fast as a
third says, unless something is to be done by the main loop. It is 1 for
any other game */
void sound_update_time(
	void)
{
	unsigned long now= GetTickCount();
	byte *globals= game_sound_globals;
	long elapsed= (long)(now-*(const unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET));
	real seconds= (real)elapsed*0.001f;
	const byte *game= (const byte *)main_game_globals;

	*(unsigned long *)(globals+GAME_SOUND_GLOBALS_START_TIME_OFFSET)= now;
	*(long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)+= elapsed;
	*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)+= seconds;
	*(real *)(globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET)= seconds;
	*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET)+= seconds;
	if (!game || !game[MAIN_GAME_GLOBALS_ACTIVE_OFFSET] || *(const long *)(game+MAIN_GAME_GLOBALS_MODE_OFFSET)!=_game_mode_campaign)
	{
		*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET)= 1.0f;
		return;
	}
	if (game[MAIN_GAME_GLOBALS_PAUSED_OFFSET])
	{
		long wait;
		real duration;

		if (*(const long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET)==NONE)
		{
			*(long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET)=
				(long)(*(const real *)(globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET)*1000.0f);
		}
		wait= *(const long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET);
		if (wait>0)
		{
			wait-= elapsed;
			*(long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET)= wait<=0 ? 0 : wait;
		}
		if (*(const long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET))
		{
			return;
		}
		duration= 0.001f;
		if (!(duration>*(const real *)(globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET+4)))
		{
			duration= *(const real *)(globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET+4);
		}
		*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET)=
			sound_step_towards(0.0f-*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET), seconds/duration)+
			*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET);
	}
	else
	{
		real duration;

		if (main_action_pending())
		{
			return;
		}
		duration= 0.001f;
		if (!(duration>*(const real *)(globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET+8)))
		{
			duration= *(const real *)(globals+GAME_SOUND_GLOBALS_ENVIRONMENT_OFFSET+8);
		}
		*(real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET)=
			*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET)+
			sound_step_towards(1.0f-*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET),
				*(const real *)(globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET)/duration);
		*(long *)(globals+GAME_SOUND_GLOBALS_PAUSE_WAIT_OFFSET)= NONE;
	}
}

/* 0x001293f0: what is done each time with the sound of each channel that
plays and is not stopped for a while. A sound that has faded out and is
heard at -64 decibels or less is let go (reason 4); one that has faded in
stops fading. For a channel with a voice, sound_channel_compute_parameters works out how the
sound is heard, to which the decibels of the real that follows the pause of
the game are added, and that is given to looping_sound_channel_update_voice (for the sounds of
the kinds 1 to 4) or to sound_channel_update_voice; and who is told of it is
told where it plays, if the sound is not marked. Before all, what the game
keeps of the console's sound is filled */
void sound_channels_update_sources(
	void)
{
	byte *globals= game_sound_globals;
	real master_decibels= sound_gain_to_decibels(*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1E4_OFFSET));
	short channel_index;

	sound_console_get_state(globals);
	for (channel_index= 0;
		channel_index<*(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		struct sound_channel *channel= &sound_channels[channel_index];
		const struct sound_source_callbacks *callbacks;
		byte parameters[SOUND_CHANNEL_PARAMETERS_SIZE];
		byte *source;
		byte *playing;
		real decibels;

		if (channel->unknown0==NONE || channel->unknown9)
		{
			continue;
		}
		source= sound_source_get(channel->unknown0);
		decibels= sound_source_fade_decibels(channel->unknown0);
		if (!(SOUND_MINIMUM_DECIBELS<*(const real *)(source+SOUND_SOURCE_DATUM_DECIBELS_OFFSET)+decibels))
		{
			playing= sound_source_get(channel->unknown0);
			if (((playing[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>7)&1) &&
				*(const long *)(playing+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)>*(const long *)(playing+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET))
			{
				sound_source_delete(channel->unknown0, _sound_source_deleted_faded_out);
				channel->unknown0= NONE;
				continue;
			}
		}
		playing= sound_source_get(channel->unknown0);
		if (((playing[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>7)&1) &&
			*(const long *)(playing+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)>*(const long *)(playing+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET) &&
			*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)>=
			*(const long *)(playing+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET))
		{
			playing[SOUND_SOURCE_DATUM_FLAGS_OFFSET]&= ~SOUND_SOURCE_DATUM_FADING_FLAG;
		}
		if (channel->unknownC==NONE)
		{
			continue;
		}
		sound_channel_compute_parameters(channel_index, &decibels, parameters);
		decibels= master_decibels+decibels;
		if (sound_source_kind_test(sound_source_get(channel->unknown0)))
		{
			looping_sound_channel_update_voice(channel_index, decibels, parameters);
		}
		else
		{
			sound_channel_update_voice(channel_index, decibels, parameters);
		}
		if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown5_bit)&1)
		{
			continue;
		}
		callbacks= *(const struct sound_source_callbacks *const *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);
		if (callbacks && callbacks->playing)
		{
			callbacks->playing(*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET),
				source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET),
				(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], channel->unknown1C, channel->unknown10);
		}
	}
}

/* 0x001295e0: a channel gets a voice of the console's sound to play its
sound with: the first of those of its type that is free and not taken. With
it goes a slot: the one that the sound says (the byte at 0xA2 of it), if its
stream is not busy; or, for a sound with the flag 2 at 6, that of the
sound voice that sound_voice_get_for_source gives. A channel that has a voice keeps it,
unless its sound has that flag and the channel has no slot. If it gets none,
the sound and the channel are let go (reason 7), and FALSE comes back */
boolean __attribute__((stdcall)) sound_channel_get_voice(
	short channel_index)
{
	struct sound_channel *channel= &sound_channels[channel_index];
	const byte *source= sound_source_get(channel->unknown0);
	byte *globals;
	long voice;
	long last;

	if (channel->unknownC!=NONE &&
		(!(source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit)) || channel->unknownE!=NONE))
	{
		return TRUE;
	}
	globals= game_sound_globals;
	voice= *(const long *)(globals+SOUND_CONSOLE_STATE_COUNTS_OFFSET+(char)channel->type*4);
	last= *(const long *)(globals+SOUND_CONSOLE_STATE_COUNTS_OFFSET+(3+(char)channel->type)*4);
	for (; voice<=last; voice++)
	{
		unsigned long bit= 1UL<<(voice&31);

		if ((((const unsigned long *)globals)[voice>>5]&bit) &&
			!(((const unsigned long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN60_OFFSET))[voice>>5]&bit))
		{
			break;
		}
	}
	if (voice<=last && (short)voice!=NONE)
	{
		long sound_voice_index= NONE;
		short slot= NONE;
		boolean found= FALSE;

		if (source[SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET]!=0xFF)
		{
			long status;

			unknown_0037b75b(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_SLOT_STREAM_OFFSET+
				(char)source[SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET]*UNKNOWN_0051EBE4_SLOT_SIZE), &status);
			if (!(status&1))
			{
				slot= (char)source[SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET];
				found= slot!=NONE;
			}
		}
		else if (source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit))
		{
			sound_voice_index= sound_voice_get_for_source(source);
			if (sound_voice_index!=NONE)
			{
				slot= *(const short *)((const byte *)sound_voices->data+(sound_voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE+
					SOUND_VOICE_DATUM_SLOT_OFFSET);
				found= slot!=NONE;
			}
		}
		else
		{
			found= TRUE;
		}
		if (found)
		{
			globals= game_sound_globals;
			channel->unknownE= slot;
			channel->unknownC= (short)voice;
			channel->unknown4= sound_voice_index;
			((unsigned long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN60_OFFSET))[(short)voice>>5]|= 1UL<<((short)voice&31);
			((unsigned long *)globals)[channel->unknownC>>5]&= ~(1UL<<(channel->unknownC&31));

			return TRUE;
		}
	}
	sound_source_delete(channel->unknown0, _sound_source_deleted_no_voice);
	sound_channel_delete(channel_index);

	return FALSE;
}

/* 0x0021de90: the sound voice that a sound that plays is heard through, and
it has one more that uses it: one that the sound can share
(sound_voice_matches_source), or a new one, with a slot of the globals of
the sound and what the sound says: its class, its flag, who is told of it
and what with, and its place. NONE if there is no room for one, or no slot */
long sound_voice_get_for_source(
	const byte *source)
{
	struct data_array *data= sound_voices;
	long voice_index;
	byte *voice;
	short slot;

	for (voice_index= data_next_index(data, NONE); voice_index!=NONE; voice_index= data_next_index(data, voice_index))
	{
		if ((byte)sound_voice_matches_source((const byte *)data->data+(voice_index&0xFFFF)*data->size, source))
		{
			break;
		}
	}
	if (voice_index==NONE)
	{
		data= sound_voices;
		if (data->actual_count>=data->maximum_count)
		{
			return NONE;
		}
		slot= sound_slot_new();
		if (slot==NONE)
		{
			return NONE;
		}
		voice_index= datum_new(sound_voices);
		if (voice_index==NONE)
		{
			sound_slot_delete(slot);
			return NONE;
		}
		voice= (byte *)sound_voices->data+(voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE;
		*(short *)(voice+SOUND_VOICE_DATUM_USERS_OFFSET)= 0;
		*(long *)(voice+SOUND_VOICE_DATUM_START_TIME_OFFSET)= NONE;
		*(long *)(voice+SOUND_VOICE_DATUM_CLASS_OFFSET)=
			sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class;
		voice[SOUND_VOICE_DATUM_UNKNOWN8_OFFSET]= source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]&1;
		memcpy(voice+SOUND_VOICE_DATUM_UNKNOWN48_OFFSET, source+SOUND_SOURCE_DATUM_POSITION_OFFSET,
			SOUND_VOICE_DATUM_UNKNOWN48_SIZE);
		memcpy(voice+SOUND_VOICE_DATUM_UNKNOWN14_OFFSET, source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET,
			SOUND_REQUEST_CALLBACK_BYTES_SIZE);
		*(long *)(voice+SOUND_VOICE_DATUM_CALLBACK_DATA_OFFSET)= *(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET);
		*(short *)(voice+SOUND_VOICE_DATUM_SLOT_OFFSET)= slot;
		*(long *)(voice+SOUND_VOICE_DATUM_CALLBACKS_OFFSET)= *(const long *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);
	}
	voice= (byte *)sound_voices->data+(voice_index&0xFFFF)*SOUND_VOICE_DATUM_SIZE;
	(*(short *)(voice+SOUND_VOICE_DATUM_USERS_OFFSET))++;

	return voice_index;
}

/* a real, from 0 to 1 */
static inline __attribute__((always_inline)) real sound_pin_fraction(
	real value)
{
	if (0.0f>value)
	{
		return 0.0f;
	}

	return value>1.0f ? 1.0f : value;
}

/* 0x00129790: what is done with the sound of a channel and three reals
that say how it is heard where it is (or none), each time. A sound of the
type 0 with the flag 12 loses of its real at 0x1C what the third real says,
by its class; with no reals, all of it, if it is not of the structure bsp
that is loaded. A sound of the types 1 and 2 that plays with a sound voice,
in a channel with a slot, gives the slot what sound_console_slot_set_parameters takes: where
the sound is as its listener sees it, two of the reals, by its class, and
three numbers of its class, the first with the decibels of what the second
real leaves. Once each time for each sound voice */
void __attribute__((stdcall)) sound_channel_update_slot(
	short channel_index,
	const real *unknown)
{
	struct sound_channel *channel= &sound_channels[channel_index];
	byte *source= sound_source_get(channel->unknown0);
	const struct sound_definition *sound;
	const struct sound_listener *listener;
	struct sound_slot_parameters parameters;
	real_point3d position;
	boolean first;
	long type;

	channel->unknownB= channel->unknownA;
	channel->unknownA= 0;
	type= (signed char)(source[SOUND_SOURCE_DATUM_TYPE_OFFSET]<<4)>>4;
	if (type==0)
	{
		if (!((*(const word *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET)>>_sound_source_unknown12_bit)&1))
		{
			return;
		}
		if (unknown)
		{
			sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
			*(real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET)=
				(1.0f-sound_pin_fraction(unknown[2]**(const real *)((const byte *)sound_class_get(sound->sound_class)+
				SOUND_CLASS_UNKNOWN50_OFFSET)))**(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET);
		}
		else if (*(const short *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET+6)!=global_structure_bsp_index)
		{
			*(real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET)= 0.0f;
		}
		return;
	}
	if (type<0 || type>2 || !(source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit)))
	{
		return;
	}
	first= TRUE;
	if (channel->unknownE==NONE)
	{
		return;
	}
	if (channel->unknown4!=NONE && !sound_voice_unknown_0021dff0(channel->unknown4, source, &first))
	{
		return;
	}
	listener= (const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+
		(char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET];
	if ((source[SOUND_SOURCE_DATUM_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)==_sound_source_datum_type_positioned)
	{
		sound_source_get_position((const struct sound_source *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET),
			(char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET], &position);
		matrix4x3_inverse_transform_point((const real_matrix4x3 *)((const byte *)listener+SOUND_LISTENER_MATRIX_OFFSET),
			&position, &position);
	}
	else
	{
		memcpy(&position, source+SOUND_SOURCE_DATUM_POSITION_OFFSET, sizeof(position));
	}
	if (first)
	{
		byte *slot= unknown_0051ebe4+UNKNOWN_0051EBE4_SLOTS_OFFSET+channel->unknownE*UNKNOWN_0051EBE4_SLOT_SIZE;

		slot[UNKNOWN_0051EBE4_SLOT_FLAGS_OFFSET]&= ~FLAG(_sound_console_slot_unknown1_bit);
	}
	sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	memcpy(&parameters.position, &position, sizeof(position));
	if (unknown)
	{
		real scale= *(const real *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN50_OFFSET);
		real left= 1.0f-unknown[1]*scale;

		parameters.unknown14= sound_pin_fraction(unknown[0]*scale);
		parameters.unknown10= sound_pin_fraction(unknown[2]*scale);
		left= sound_gain_to_decibels(left);
		parameters.unknown18= left+*(const real *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN10_OFFSET);
		parameters.unknown20= *(const real *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN54_OFFSET)*
			*(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET);
		parameters.unknown1C= *(const real *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN58_OFFSET)*
			*(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET);
	}
	else
	{
		parameters.unknown14= 0.0f;
		parameters.unknown10= 0.0f;
		memcpy(&parameters.unknown18, (const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWN10_OFFSET, sizeof(real));
		parameters.unknown20= 0.0f;
		parameters.unknown1C= 0.0f;
	}
	parameters.flags= (((const byte *)listener)[SOUND_LISTENER_UNKNOWN7_OFFSET]!=0)|2;
	sound_console_slot_set_parameters(&parameters, channel->unknownE);
}

/* 0x00129100: each frame: the channels that play get a voice, and a list is
made of the sounds of which it is to be worked out how they are heard where
they are: at most 72. One for each slot of a sound with a place that has
none of three flags, and one for each sound of the type 0 with the flag 12
that is of the structure bsp that is loaded, which is asked at the place of
unknown_00468788. Each one says the listener, the place, where it is in the
structure bsp and four flags, and has room for three reals: where those of
each are is put in two tables, by slot and by voice, which
sound_channels_unknown_00129380 reads after they are worked out */
void __attribute__((stdcall)) sound_channels_build_queries(
	byte *first,
	byte *second,
	struct sound_update_listeners *listeners)
{
	unsigned long taken[3]= { 0, 0, 0 };
	real **slot_results= (real **)first;
	real **voice_results= (real **)second;
	long channel_index;

	for (channel_index= 0;
		channel_index<*(const short *)(game_sound_globals+GAME_SOUND_GLOBALS_CHANNEL_COUNT_OFFSET);
		channel_index++)
	{
		const struct sound_channel *channel;
		const byte *source;
		struct sound_query *query;
		boolean positioned;
		boolean ambient;
		long type;

		if (listeners->count==MAXIMUM_SOUND_QUERIES)
		{
			break;
		}
		channel= &sound_channels[(short)channel_index];
		if (channel->unknown0==NONE || channel->unknown9 || !sound_channel_get_voice((short)channel_index))
		{
			continue;
		}
		source= sound_source_get(channel->unknown0);
		type= (signed char)(source[SOUND_SOURCE_DATUM_TYPE_OFFSET]<<4)>>4;
		positioned= type==_sound_source_datum_type_positioned &&
			!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown2_bit)&1) &&
			!((source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]>>7)&1) && !(source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]&1);
		ambient= type==0 && !((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown2_bit)&1) &&
			((*(const word *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET)>>_sound_source_unknown12_bit)&1) &&
			*(const short *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET+6)==global_structure_bsp_index;
		if (positioned)
		{
			const struct sound_definition *sound;
			short slot= channel->unknownE;

			if (slot==NONE || (taken[slot>>5]&(1UL<<(slot&31))))
			{
				continue;
			}
			taken[slot>>5]|= 1UL<<(slot&31);
			listeners->count++;
			sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
			query= (struct sound_query *)((byte *)listeners+listeners->count*(long)sizeof(struct sound_query)-
				((long)sizeof(struct sound_query)-(long)sizeof(long)));
			query->listener_index= (char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET];
			memcpy(&query->position, source+SOUND_SOURCE_DATUM_POSITION_OFFSET, sizeof(query->position));
			query->cluster_index= *(const short *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET+4);
			query->leaf_index= *(const long *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET);
			query->unknown44= FALSE;
			query->unknown46= (source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]>>7)&1;
			query->unknown45= FALSE;
			query->unknown47= ((sound_class_get(sound->sound_class)->internal_flags>>_sound_class_internal_unknown6_bit)&1) ||
				(sound->flags&FLAG(_sound_definition_unknown5_bit));
			query->results[0]= 0.0f;
			query->results[2]= 0.0f;
			query->results[1]= 0.0f;
			slot_results[channel->unknownE]= query->results;
		}
		else if (ambient)
		{
			if (channel->unknownC==NONE)
			{
				continue;
			}
			listeners->count++;
			query= (struct sound_query *)((byte *)listeners+listeners->count*(long)sizeof(struct sound_query)-
				((long)sizeof(struct sound_query)-(long)sizeof(long)));
			query->listener_index= (char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET];
			memcpy(&query->position, unknown_00468788, sizeof(query->position));
			query->cluster_index= *(const short *)(source+SOUND_SOURCE_DATUM_LOCATION_OFFSET+4);
			query->leaf_index= NONE;
			query->unknown44= TRUE;
			query->unknown46= TRUE;
			query->unknown45= FALSE;
			query->unknown47= TRUE;
			query->results[0]= 0.0f;
			query->results[2]= 0.0f;
			query->results[1]= 0.0f;
			voice_results[channel->unknownC]= query->results;
		}
	}
}

/* 0x0012a9d0: the Doppler effect of a sound for a listener, in cents: how
much higher it is heard for how fast the sound and the listener come near,
and how much lower for how fast they go apart. From three octaves down to two
up. The place of the sound is followed by how fast it moves; how fast the
listener moves is turned by its matrix. The name is made up */
long double sound_listener_doppler_pitch(
	long listener_index,
	const real_point3d *position)
{
	const struct sound_listener *listener=
		(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+listener_index;
	const real_vector3d *velocity= (const real_vector3d *)((const byte *)position+SOUND_POSITION_VELOCITY_OFFSET);
	real i= position->x-listener->matrix.position.x;
	real j= position->y-listener->matrix.position.y;
	real k= position->z-listener->matrix.position.z;
	long double exact_length= sqrtl((long double)k*(long double)k+(long double)j*(long double)j+(long double)i*(long double)i);
	real length= (real)exact_length;
	real listener_i= listener->velocity.i;
	real listener_j= listener->velocity.j;
	real listener_k= listener->velocity.k;
	real x;
	real y;
	real z;
	real away;
	real speed;
	real ratio;
	real limited;

	if (!((long double)0.0001f>fabsl(exact_length)))
	{
		real scale= 1.0f/length;

		i= scale*i;
		j*= scale;
		k*= scale;
	}
	if (listener->matrix.scale!=1.0f)
	{
		listener_i= listener->matrix.scale*listener_i;
		listener_j= listener->matrix.scale*listener_j;
		listener_k= listener->matrix.scale*listener_k;
	}
	x= listener->matrix.up.i*listener_k + listener->matrix.left.i*listener_j + listener->matrix.forward.i*listener_i;
	y= listener->matrix.up.j*listener_k + listener->matrix.left.j*listener_j + listener->matrix.forward.j*listener_i;
	z= listener->matrix.up.k*listener_k + listener->matrix.left.k*listener_j + listener->matrix.forward.k*listener_i;
	away= 0.0f-(z*k + y*j + x*i);
	speed= velocity->k*k + velocity->j*j + velocity->i*i + SOUND_SPEED;
	if (!(speed>0.001f))
	{
		speed= 0.001f;
	}
	ratio= (real)(((long double)SOUND_SPEED-(long double)away)/(long double)speed);
	if (0.125>(double)ratio)
	{
		return logl((long double)0.125)*(long double)SOUND_CENTS_PER_NATURAL_LOGARITHM;
	}
	limited= 4.0f;
	if (!(ratio>4.0f))
	{
		limited= ratio;
	}

	return logl((long double)limited)*(long double)SOUND_CENTS_PER_NATURAL_LOGARITHM;
}

/* 0x0012ad30: how much of a sound is heard from where its listener is, from
0 to 1: what sound_source_distance_gain says of how far it is, times what
its cone says. `offset` is the way from the listener to the sound, and
`forward` where the sound points. In its inner cone all is heard; out of
the outer one, what the gain of the outer cone says; between the two, by
the angle. A sound with no cone of its own whose tag has the flag 12 has no
cone. With the flag 9, what is not heard is given. The name is made up */
real __attribute__((stdcall)) sound_source_spatial_gain(
	const real_vector3d *offset,
	const struct sound_source *source,
	long sound_index,
	const real_vector3d *forward)
{
	real distance_gain= sound_source_distance_gain(sound_index, source,
		(real)sqrtl((long double)(offset->i*offset->i + offset->j*offset->j + offset->k*offset->k)));
	word flags= source->flags;
	real cone_gain= 1.0f;
	real gain;

	if (((flags>>_sound_source_inner_cone_angle_bit)&1 || (flags>>_sound_source_outer_cone_angle_bit)&1 ||
			!(sound_definition_get(sound_index)->flags&FLAG(_sound_unknown12_bit))) &&
		!((long double)0.0001f>fabsl((long double)distance_gain)) &&
		offset->j*offset->j + offset->k*offset->k + offset->i*offset->i>=SOUND_MINIMUM_OFFSET_SQUARED)
	{
		real_vector3d toward;

		toward.i= 0.0f-offset->i;
		toward.j= 0.0f-offset->j;
		toward.k= 0.0f-offset->k;
		if (normalize3d(&toward)>0.0f)
		{
			real outer_cone_gain= sound_get_outer_cone_gain(source, sound_index);
			real angle= angle_between_vectors3d(&toward, forward);
			real outer_cone_angle= sound_get_outer_cone_angle(source, sound_index);
			real inner_cone_angle= sound_get_inner_cone_angle(source, sound_index);
			real fraction= sound_interpolation_fraction(inner_cone_angle, outer_cone_angle, angle, TRUE);

			cone_gain= (real)((sound_decibels_to_gain(outer_cone_gain)-(long double)1.0f)*(long double)fraction+
				(long double)1.0f);
		}
	}
	gain= cone_gain*distance_gain;
	if ((flags>>_sound_source_inverted_gain_bit)&1)
	{
		gain= 1.0f-gain;
	}

	return gain;
}

/* 0x00220e00: a slot of the console's sound takes where its sound is and
two reals more. The console is told where it is the first time (a slot
with no flag 1, or when the first byte of the sound of the console is not
set) and when it has moved 0.05 or more along an axis. Each of the two
reals, at 0x10 and 0x14 of what it takes, goes to the slot as it is the
first time, when the flag 0 changes, or when its step (at 0x1C and 0x20) is
not over 0; if not, the slot goes towards it by no more than the step. With
nothing new, no more is done; if not, sound_console_slot_update_room is told.
The name is made up */
void __attribute__((stdcall)) sound_console_slot_set_parameters(
	const struct sound_slot_parameters *parameters,
	long slot_index)
{
	byte *console= unknown_0051ebe4;
	struct sound_console_slot *slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET)+slot_index;
	boolean first= !(slot->flags&FLAG(_sound_console_slot_unknown1_bit)) || !console[0];
	byte flag= (byte)parameters->flags&FLAG(_sound_console_slot_unknown0_bit);
	real unknown10= parameters->unknown10;
	real unknown14= parameters->unknown14;

	if (first ||
		!((long double)0.05f>fabsl((long double)parameters->position.x-(long double)slot->position.x)) ||
		!((long double)0.05f>fabsl((long double)parameters->position.y-(long double)slot->position.y)) ||
		!((long double)0.05f>fabsl((long double)parameters->position.z-(long double)slot->position.z)))
	{
		/* the console has the height second */
		unknown_0037c668(slot->stream, parameters->position.x, parameters->position.z, parameters->position.y, 1);
		slot->position= parameters->position;
	}
	if (!first &&
		(long double)0.001f>fabsl((long double)unknown10-(long double)slot->unknown2C) &&
		(long double)0.001f>fabsl((long double)unknown14-(long double)slot->unknown30) &&
		(slot->flags&FLAG(_sound_console_slot_unknown0_bit))==flag)
	{
		slot->flags|= FLAG(_sound_console_slot_unknown1_bit);
		return;
	}

	if (first || (slot->flags&FLAG(_sound_console_slot_unknown0_bit))!=flag || 0.0f>=parameters->unknown20)
	{
		slot->unknown30= unknown14;
	}
	else
	{
		real difference= unknown14-slot->unknown30;
		real limit= parameters->unknown20;
		real step= 0.0f-limit;

		if (!(step>difference))
		{
			step= limit;
			if (!(difference>limit))
			{
				step= difference;
			}
		}
		slot->unknown30= slot->unknown30+step;
	}
	if (first || (slot->flags&FLAG(_sound_console_slot_unknown0_bit))!=flag || 0.0f>=parameters->unknown1C)
	{
		slot->unknown2C= unknown10;
	}
	else
	{
		real limit= parameters->unknown1C;
		real difference= unknown10-slot->unknown2C;
		real step= 0.0f-limit;

		if (!(step>difference))
		{
			step= limit;
			if (!(difference>limit))
			{
				step= difference;
			}
		}
		slot->unknown2C= step+slot->unknown2C;
	}
	slot->flags^= (slot->flags^flag)&FLAG(_sound_console_slot_unknown0_bit);
	sound_console_slot_update_room(slot_index, parameters->unknown18);
	slot->flags|= FLAG(_sound_console_slot_unknown1_bit);
}

/* a gain in hundredths of a decibel, from -6400 to 0 */
static inline __attribute__((always_inline)) long sound_gain_to_millibels(
	real gain)
{
	long millibels;

	if (gain==0.0f)
	{
		return SOUND_MINIMUM_MILLIBELS;
	}
	millibels= (long)(long long)(log10l((long double)gain)*(long double)2000.0f);
	if (millibels<SOUND_MINIMUM_MILLIBELS)
	{
		return SOUND_MINIMUM_MILLIBELS;
	}

	return millibels>0 ? 0 : millibels;
}

/* 0x002211e0: the console is told how the sound of a slot is heard in a
room, and its filter. Of the three gains of the sound of the console, the
first two are for the sound as it comes; the third, for the room, less up
to 64 decibels for how far `decibels` is from 0, to no less than -100
decibels. What is left of the two reals of the slot, from 1, is how much
gets through what is in the way and through a wall. The filter is of
unknown_00470084 (a frequency) and unknown_00470088 (some decibels). The
name is made up */
void __attribute__((stdcall)) sound_console_slot_update_room(
	long slot_index,
	real decibels)
{
	const byte *console= unknown_0051ebe4;
	const real *gains= (const real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET);
	struct sound_console_slot *slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET)+slot_index;
	struct sound_console_i3dl2_source room;
	struct sound_console_filter filter;
	real fraction;
	long millibels;

	memset(&room, 0, sizeof(room));
	room.direct= sound_gain_to_millibels(gains[0]);
	room.direct_high_frequencies= sound_gain_to_millibels(gains[1]);
	room.room_rolloff= 0.0f;
	room.obstruction_low_frequency_ratio= 0.0f;
	room.occlusion_low_frequency_ratio= 0.25f;
	fraction= sound_interpolation_fraction(SOUND_MINIMUM_DECIBELS, 0.0f, decibels, TRUE);
	millibels= sound_gain_to_millibels(gains[2])-(long)(6400.0f-fraction*6400.0f);
	if (millibels<-10000)
	{
		millibels= -10000;
	}
	else if (millibels>0)
	{
		millibels= 0;
	}
	room.room= millibels;
	room.room_high_frequencies= millibels;
	room.obstruction_high_frequencies= sound_gain_to_millibels(1.0f-slot->unknown30);
	room.occlusion_high_frequencies= sound_gain_to_millibels(1.0f-slot->unknown2C);
	unknown_0037c6e5(slot->stream, &room, 1);

	filter.mode= 1;
	filter.unknown4= 0;
	filter.coefficients[0]= sound_frequency_to_filter_coefficient((real)unknown_00470084);
	filter.coefficients[1]= (long)sound_unknown_0021e2d0(unknown_00470088);
	filter.coefficients[2]= 0;
	filter.coefficients[3]= 0;
	unknown_0037b68b(slot->stream, &filter);
}

/* 0x001282d0: each frame, the sounds that play whose time has come get a
channel. One whose samples are asked for (sound_source_request_data) gets
what sound_source_get_channel says: with none, it is let go (reason 7); in a
channel that was of another sound, that one is let go, with the reason that
was given, and the channel too; and who is told of the sound is told that it
has a channel, if its flag 5 is not set. One whose samples cannot be asked
for, with no channel, is let go (reason 14) if what is at 0xE of its class
is 0 and its flag 4 is not set: and its recording is set aside to be the
next of its pitch range, if none is and the class has no flag 5. The name
is made up */
void sound_sources_get_channels(
	void)
{
	long source_index;

	for (source_index= data_next_index(sound_sources, NONE);
		source_index!=NONE;
		source_index= data_next_index(sound_sources, source_index))
	{
		byte *source= sound_source_get(source_index);

		if (*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)>*(long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET))
		{
			continue;
		}
		if (sound_source_request_data(source))
		{
			long reason;
			short channel_index= sound_source_get_channel(source_index, &reason);

			if (channel_index==NONE)
			{
				sound_source_delete(source_index, _sound_source_deleted_no_voice);
			}
			else
			{
				struct sound_channel *channel= sound_channels+channel_index;
				long other_source_index= channel->unknown0;

				if (other_source_index!=source_index)
				{
					if (other_source_index!=NONE)
					{
						sound_source_delete(other_source_index, reason);
						sound_channel_delete(channel_index);
					}
					channel->unknown0= source_index;
					*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)=
						*(long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
					*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)= channel_index;
				}
				if (!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown5_bit)&1))
				{
					const struct sound_source_callbacks *callbacks=
						*(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);

					if (callbacks && callbacks->got_channel)
					{
						callbacks->got_channel(*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET),
							*(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET), source_index,
							source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET);
					}
				}
			}
		}
		else if (*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)==NONE)
		{
			const struct sound_definition *sound= sound_definition_get(*(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
			short unknown;

			if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown4_bit)&1)
			{
				unknown= 1;
			}
			else
			{
				unknown= *(const short *)((const byte *)sound_class_get(sound->sound_class)+SOUND_CLASS_UNKNOWNE_OFFSET);
			}
			if (unknown==0)
			{
				struct sound_pitch_range *pitch_range=
					(struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
					((char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]+sound->first_pitch_range_index);

				if (!((sound_class_get(sound->sound_class)->internal_flags>>_sound_class_internal_unknown5_bit)&1) &&
					(short)sound_pitch_range_get_unknown5(pitch_range, sound)==NONE)
				{
					sound_pitch_range_set_unknown5(pitch_range, sound, (char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
				}
				sound_source_delete(source_index, _sound_source_deleted_no_data);
			}
		}
	}
}

/* 0x00128020: each frame, of each sound that plays whose channel is not
paused. It is let go when the voice of its channel does nothing and it has
the flags 6 and 9 (reason 2); when its promotion keeps it from playing: it
is of a rule under the one that the promotion is at, and of the last time
that the promotion was looked at (reason 15); and when
sound_source_unknown_00127fc0 says FALSE (reason 3). If not, the listener
that hears it is looked for, as far as the sound is heard: with none, the
sound starts to fade out, in two seconds, and its flag 2 says so; with one,
a sound with that flag fades back in half a second. The name is made up */
void sound_sources_update(
	void)
{
	long source_index;

	for (source_index= data_next_index(sound_sources, NONE);
		source_index!=NONE;
		source_index= data_next_index(sound_sources, source_index))
	{
		byte *source= sound_source_get(source_index);
		const struct sound_definition *sound= sound_definition_get(*(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
		char promotion_index= ((const char *)sound)[SOUND_DEFINITION_PROMOTION_INDEX_OFFSET];
		const byte *promotion= NULL;
		short channel_index;
		struct sound_source *said;
		long listener_index;
		word flags;

		if (promotion_index!=NONE)
		{
			promotion= (const byte *)((uintptr_t)*(const long *)((const byte *)global_sound_cache_file_gestalt+
				SOUND_GESTALT_PROMOTIONS_OFFSET+4)+(uintptr_t)(promotion_index*SOUND_PROMOTION_SIZE));
		}
		channel_index= *(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET);
		if (channel_index!=NONE && sound_channels[channel_index].unknown9)
		{
			continue;
		}
		flags= *(word *)(source+SOUND_SOURCE_DATUM_FLAGS_OFFSET);
		if ((flags>>_sound_source_datum_unknown9_bit)&1 && (flags>>_sound_source_datum_unknown6_bit)&1 &&
			!sound_voice_state(sound_channel_voice(sound_channels+channel_index)))
		{
			sound_source_delete(source_index, _sound_source_deleted_unknown2);
			continue;
		}
		if (promotion && (char)source[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET]!=NONE &&
			(char)source[SOUND_SOURCE_DATUM_UNKNOWNA0_OFFSET]<*(const long *)(promotion+SOUND_PROMOTION_RULE_INDEX_OFFSET) &&
			*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)==*(const long *)(promotion+SOUND_PROMOTION_LAST_TIME_OFFSET) &&
			*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)<*(const long *)(promotion+SOUND_PROMOTION_SUPPRESSED_UNTIL_OFFSET))
		{
			sound_source_delete(source_index, _sound_source_deleted_promotion);
			continue;
		}
		if (!sound_source_unknown_00127fc0(source_index))
		{
			sound_source_delete(source_index, _sound_source_deleted_unknown3);
			continue;
		}
		said= (struct sound_source *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET);
		if (sound->flags&FLAG(_sound_definition_unknown3_bit))
		{
			source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET]|= FLAG(_sound_source_unknown0_bit);
		}
		listener_index= sound_source_get_listener(said,
			sound_get_maximum_distance(said, *(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)), NULL);
		if (listener_index==NONE)
		{
			if (!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown2_bit)&1))
			{
				byte *fading= sound_source_get(source_index);

				*(real *)(fading+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(source_index);
				*(short *)(fading+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= 0;
				*(long *)(fading+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= SOUND_UNHEARD_FADE_OUT_TIME;
				*(long *)(fading+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
				fading[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
				source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown2_bit);
			}
		}
		else
		{
			source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]= (byte)listener_index;
			if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown2_bit)&1)
			{
				sound_sources_cross_fade(source_index, NONE, 0, 0.5f);
				source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]&= ~FLAG(_sound_source_datum_unknown2_bit);
			}
		}
	}
}

/* 0x00222930: four things of a sound from the tags, for a voice of the
console's sound: at 2, the number that sound_unknown_001914f0 has for the
name of the first element of a block of a tag; at 4 and 8, a number and
where four bytes more are: of the element itself, or of what the first
element of what unknown_0018d090 gives for the sound has in a block of its
own, if it has it; at 0xC, the same as at 8. The tag is the one that
sound_request_unknown_002226b0 looks at. With no tag, NONE at 2 and 0xC, and
0 at 8. What they are is not known */
void sound_unknown_00222930(
	long sound_index,
	long unknown,
	byte *result)
{
	const byte *collection;
	const byte *element;
	const byte *tag;
	const byte *block;
	long tag_index;

	*(short *)(result+SOUND_VOICE_ENVIRONMENT_NUMBER_OFFSET)= NONE;
	*(const byte **)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET)= NULL;
	*(long *)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWNC_OFFSET)= NONE;
	collection= unknown_0018d090(sound_index, unknown);
	/* a count of the least number there is passes, as in the game */
	if (!collection || (long)(*(const unsigned long *)(collection+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)-1)<0)
	{
		return;
	}
	element= *(const byte *const *)(collection+SOUND_EFFECT_COLLECTION_COUNT_OFFSET+4);
	if (!element)
	{
		return;
	}
	tag_index= *(const long *)(element+4);
	if (tag_index==NONE)
	{
		return;
	}
	tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (*(const long *)(tag+SOUND_ENVIRONMENT_TAG_NAMES_OFFSET)<=0)
	{
		return;
	}
	block= *(const byte *const *)(tag+SOUND_ENVIRONMENT_TAG_NAMES_OFFSET+4);
	*(short *)(result+SOUND_VOICE_ENVIRONMENT_NUMBER_OFFSET)= (short)sound_unknown_001914f0(*(const long *)block);
	if (*(const long *)(element+SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET)>0)
	{
		const byte *other= *(const byte *const *)(element+SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET+4);

		if (*(const long *)(other+8)>0)
		{
			const byte *entry= *(const byte *const *)(other+0xC);

			*(long *)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN4_OFFSET)= *(const long *)(entry+0xC);
			*(const byte **)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET)= entry+4;
		}
	}
	if (!*(const byte **)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET))
	{
		*(long *)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN4_OFFSET)= *(const long *)(block+0xC);
		*(const byte **)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET)= block+4;
	}
	*(long *)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWNC_OFFSET)= *(const long *)(result+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET);
}

/* 0x002225a0: a list for the console's sound, from the elements of a block
of a tag: how many they are, and then what sound_effect_entry_build writes for each,
one after another, in pieces of four bytes: the word at 0xA of what it
writes says how many bytes more than 0xC it took. An element of the name
0x11000138 is written four times, with the numbers 0 to 3; any other, once,
with NONE. `overrides` has, at 4, something of 0x1C bytes for each element
whose bit is set in its mask at 0x10: sound_effect_entry_build is given it. `size`
takes how many bytes the list is. What the list is of is not known */
void __attribute__((stdcall)) sound_unknown_002225a0(
	const byte *overrides,
	const long *block,
	long unknown,
	long *list,
	long *size)
{
	long position= 0;
	long override_offset= 0;
	long index;

	list[0]= block[0];
	*size= sizeof(long);
	for (index= 0; index<block[0]; index++)
	{
		const long *element= (const long *)((const byte *)(uintptr_t)block[1]+index*SOUND_EFFECT_ELEMENT_SIZE);
		const byte *override= NULL;
		boolean four_times;
		long number;

		if (*(const unsigned long *)(overrides+SOUND_EFFECT_OVERRIDES_MASK_OFFSET)&(1UL<<(index&31)))
		{
			override= *(const byte *const *)(overrides+4)+override_offset;
			override_offset+= SOUND_EFFECT_OVERRIDE_SIZE;
		}
		four_times= element[0]==SOUND_EFFECT_NAME_FOUR_TIMES;
		for (number= four_times ? 0 : NONE; number<(four_times ? 4 : 0); number++)
		{
			const word *bytes= (const word *)((const byte *)list+position*4+sizeof(long)+SOUND_EFFECT_ENTRY_BYTES_OFFSET);

			sound_effect_entry_build((long *)((byte *)list+position*4+sizeof(long)), element, override,
				(const real *)(uintptr_t)unknown, number);
			*size+= *bytes+SOUND_EFFECT_ENTRY_SIZE;
			position+= (*bytes+SOUND_EFFECT_ENTRY_SIZE+3)>>2;
		}
	}
}

/* the first element of a block of a tag; NULL if it has none. A count of
the least number there is passes, as in the game */
static inline __attribute__((always_inline)) const byte *sound_block_first_element(
	const byte *block)
{
	return (long)(*(const unsigned long *)block-1)>=0 ? *(const byte *const *)(block+4) : NULL;
}

/* whether what unknown_0018d090 or sound_get_gestalt_unknown30 gives has a list for
the console's sound: its first element says a tag and has a block of its
own with something */
static inline __attribute__((always_inline)) boolean sound_effect_collection_has_list(
	const byte *element)
{
	return element && *(const long *)(element+4)!=NONE &&
		*(const long *)(element+SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET)>0;
}

/* 0x00222770: what a voice of the console's sound takes of a sound from
the tags: three things at 0x10, 0x14 and 0x18, the first elements of three
blocks of what sound_get_gestalt_unknown30 gives for the sound, or of what
unknown_0018d090 gives, where it has them; and at 0x1C, the list that
sound_unknown_002225a0 makes, with its size at 0x41C: 0 if neither has a
list. The list is always made from what unknown_0018d090 gives, as in the
game. What they are is not known */
void __attribute__((stdcall)) sound_unknown_00222770(
	long sound_index,
	long unknown,
	long unknown2,
	byte *result)
{
	const byte *first= unknown_0018d090(sound_index, unknown);
	const byte *second= sound_get_gestalt_unknown30(sound_index);
	const byte *chosen= NULL;

	if (second)
	{
		const byte *unknown10= sound_block_first_element(second+SOUND_EFFECT_COLLECTION_UNKNOWN14_OFFSET);
		const byte *unknown14= sound_block_first_element(second+SOUND_EFFECT_COLLECTION_UNKNOWN1C_OFFSET);
		const byte *unknown18= sound_block_first_element(second+SOUND_EFFECT_COLLECTION_UNKNOWN24_OFFSET);
		const byte *element= sound_block_first_element(second+SOUND_EFFECT_COLLECTION_COUNT_OFFSET);

		*(long *)(result+SOUND_VOICE_EFFECTS_UNKNOWN8_OFFSET)= 0;
		*(long *)(result+SOUND_VOICE_EFFECTS_UNKNOWNC_OFFSET)= 0;
		*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN10_OFFSET)= unknown10;
		*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN14_OFFSET)= unknown14;
		*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN18_OFFSET)= unknown18;
		if (sound_effect_collection_has_list(element))
		{
			chosen= second;
		}
	}
	if (first && first!=second)
	{
		const byte *unknown10= sound_block_first_element(first+SOUND_EFFECT_COLLECTION_UNKNOWN14_OFFSET);
		const byte *unknown14= sound_block_first_element(first+SOUND_EFFECT_COLLECTION_UNKNOWN1C_OFFSET);
		const byte *unknown18= sound_block_first_element(first+SOUND_EFFECT_COLLECTION_UNKNOWN24_OFFSET);
		const byte *element= sound_block_first_element(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET);

		if (unknown10)
		{
			*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN10_OFFSET)= unknown10;
		}
		if (unknown14)
		{
			*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN14_OFFSET)= unknown14;
		}
		if (unknown18)
		{
			*(const byte **)(result+SOUND_VOICE_EFFECTS_UNKNOWN18_OFFSET)= unknown18;
		}
		if (sound_effect_collection_has_list(element))
		{
			chosen= first;
		}
	}
	if (chosen)
	{
		const byte *element= *(const long *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET)>0 ?
			*(const byte *const *)(first+SOUND_EFFECT_COLLECTION_COUNT_OFFSET+4) : NULL;
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(element+4)&0xFFFF].address;

		sound_unknown_002225a0(*(const byte *const *)(element+SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET+4),
			*(const long *const *)(tag+SOUND_ENVIRONMENT_TAG_BLOCK_OFFSET+4), unknown2,
			(long *)(result+SOUND_VOICE_EFFECTS_LIST_OFFSET), (long *)(result+SOUND_VOICE_EFFECTS_LIST_SIZE_OFFSET));
	}
	else
	{
		*(long *)(result+SOUND_VOICE_EFFECTS_LIST_SIZE_OFFSET)= 0;
	}
}

/* a class of sounds, as sound_class_get gives it: the game has it written
out where it does not call it */
static inline __attribute__((always_inline)) const struct sound_class *sound_class_get_inline(
	short class_index)
{
	const struct tag_block *block= (const struct tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_SOUND_OFFSET);
	const byte *reference= block->count ? (const byte *)(uintptr_t)block->address : NULL;
	const struct sound_classes *classes=
		(const struct sound_classes *)(uintptr_t)global_tag_instances[*(const long *)(reference+4)&0xFFFF].address;

	return (const struct sound_class *)(uintptr_t)classes->classes.address+class_index;
}

/* a flag of a number is set or not */
static inline __attribute__((always_inline)) unsigned long sound_set_flag(
	unsigned long flags,
	long bit,
	boolean set)
{
	return set ? flags|FLAG(bit) : flags&~FLAG(bit);
}

/* 0x0012a1b0: what a voice of the console's sound takes of the sound of a
channel is filled: from the tag of the sound and its class (two numbers, a
real of the class at 0x14 and the flags at 0x30), from the sound that plays
(a number at 0x12, its seed at 0x48 and its scale at 0x4C: times the square
root of what is left to 1 of the real at 0x18 of `parameters`, if the sound
has the flag 8), from the channel (its slot, at 0x24) and from `parameters`,
which is what sound_channel_compute_parameters works out of how the sound is heard. At 0x48
goes too what sound_unknown_00222770 fills, which is given the seconds that
the sound has played, and at 0x468 what sound_unknown_00222930 fills. The
name is made up */
void __attribute__((stdcall)) sound_channel_build_voice_parameters(
	short channel_index,
	const byte *parameters,
	byte *result,
	const byte *source)
{
	const struct sound_channel *channel= sound_channels+channel_index;
	long sound_index= *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET);
	const struct sound_definition *sound= sound_definition_get(sound_index);
	const struct sound_class *sound_class= sound_class_get_inline(sound->sound_class);
	boolean unknown3= (source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]>>3)&1;
	unsigned long flags;
	struct
	{
		real unknown0;
		real seconds;
		real scale;
		real unknownC;
		long unknown10[3];
	} played;

	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN4_OFFSET)= (char)sound->unknown3[0];
	*(short *)(result+SOUND_VOICE_PARAMETERS_CLASS_OFFSET)= sound->sound_class;
	*(short *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN12_OFFSET)= (char)source[SOUND_SOURCE_DATUM_UNKNOWNA4_OFFSET];
	*(short *)(result+SOUND_VOICE_PARAMETERS_SLOT_OFFSET)= channel->unknownE;
	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET)=
		*(const long *)((const byte *)sound_class_get_inline(sound->sound_class)+SOUND_CLASS_UNKNOWN10_OFFSET);
	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN18_OFFSET)= *(const long *)(parameters+0x8);
	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN28_OFFSET)= *(const long *)(parameters+0x4);
	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN1C_OFFSET)= *(const long *)(parameters+0xC);
	*(long *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN20_OFFSET)= *(const long *)(parameters+0x10);

	flags= *(unsigned long *)(result+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET);
	flags= sound_set_flag(flags, 1, (sound_class->internal_flags>>3)&1);
	flags= sound_set_flag(flags, 3, (sound_class->internal_flags>>1)&1);
	flags= sound_set_flag(flags, 4, ((const byte *)sound_class)[SOUND_CLASS_UNKNOWN4C_OFFSET]==1);
	flags= sound_set_flag(flags, 2, ((sound_class->internal_flags>>1)&1) || unknown3);
	flags= sound_set_flag(flags, 6, (sound_class->internal_flags>>9)&1);
	*(unsigned long *)(result+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET)= flags;
	if (parameters[0]&FLAG(4))
	{
		*(unsigned long *)(result+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET)|= FLAG(5);
		memcpy(result+SOUND_VOICE_PARAMETERS_UNKNOWN38_OFFSET, parameters+0x28, 0x10);
	}
	if (*(short *)(result+SOUND_VOICE_PARAMETERS_UNKNOWN12_OFFSET)!=NONE)
	{
		*(unsigned long *)(result+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET)|= FLAG(0);
	}

	played.unknown0= 0.0f;
	played.seconds= (real)((*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET)-
		*(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET))/1000);
	played.scale= *(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET);
	played.unknownC= *(const real *)(parameters+0x18);
	memcpy(played.unknown10, parameters+0x1C, sizeof(played.unknown10));
	sound_unknown_00222770(sound_index, *(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8_OFFSET), (long)(uintptr_t)&played,
		result+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET);
	sound_unknown_00222930(sound_index, *(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8_OFFSET),
		result+SOUND_VOICE_PARAMETERS_ENVIRONMENT_OFFSET);

	*(long *)(result+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET)= *(const long *)(source+SOUND_SOURCE_DATUM_SEED_OFFSET);
	if (source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET+1]&1)
	{
		*(real *)(result+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET+4)=
			(real)(sqrtl((long double)1.0f-(long double)*(const real *)(parameters+0x18))*
			(long double)*(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET));
	}
	else
	{
		*(real *)(result+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET+4)= *(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET);
	}
}

/* some decibels, from -64 to 0 */
static inline __attribute__((always_inline)) real sound_pin_decibels(
	real decibels)
{
	if (SOUND_MINIMUM_DECIBELS>decibels)
	{
		return SOUND_MINIMUM_DECIBELS;
	}

	return decibels>0.0f ? 0.0f : decibels;
}

/* 0x00128ca0: what is done each time with the sound of a channel that has
a voice of the console's sound, for a sound of none of the kinds 1 to 4.
`decibels` is how it is heard, and `parameters` what sound_channel_compute_parameters worked
out. The voice is given what sound_channel_build_voice_parameters fills,
with the pitch of the sound (that of its scale and its own, less that of the
recordings as they were made, and for a sound with a sound voice the Doppler
effect: that of its sound playback controller, or what
sound_listener_doppler_pitch says) and how loud it is: what
sound_definition_decibels says plus the gain of the recording. A sound with
no flag 9 starts: its voice starts, its first piece of samples is queued and
its sound playback controller has one less to start; any other goes on
(sound_console_voice_set_parameters), if its channel has a recording. Then the next piece is
seen to: one that waits (flag 3) is queued when the channel has no second
recording and its samples are asked for; if none waits, the sound goes on
to the next piece of its recording and asks for it, or with no more pieces
it is marked as at its end (flag 6), and its voice too. The name is made
up */
void __attribute__((stdcall)) sound_channel_update_voice(
	short channel_index,
	real decibels,
	const byte *parameters)
{
	struct sound_channel *channel= sound_channels+channel_index;
	byte *source= sound_source_get(channel->unknown0);
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	byte voice_parameters[SOUND_VOICE_PARAMETERS_SIZE];
	real scale= *(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET);
	const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
	const struct sound_pitch_range *pitch_range;
	const struct sound_permutation *permutation;
	real pitch;
	real loudness;
	short chunk;

	memset(voice_parameters, 0, sizeof(voice_parameters));
	pitch= ((real)bends[1]-(real)bends[0])*scale + *(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET) + (real)bends[0];
	loudness= sound_definition_decibels(sound,
		*(const real *)(source+SOUND_SOURCE_DATUM_DECIBELS_OFFSET)+(*(const real *)(source+SOUND_SOURCE_DATUM_GAIN_OFFSET)+decibels),
		scale);
	sound_channel_build_voice_parameters(channel_index, parameters, voice_parameters, source);

	pitch_range= (const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		((char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]+sound->first_pitch_range_index);
	permutation= (const struct sound_permutation *)(uintptr_t)global_sound_cache_file_gestalt->permutations.address+
		(pitch_range->first_permutation_index+(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
	*(short *)(voice_parameters+SOUND_VOICE_PARAMETERS_UNKNOWN34_OFFSET)= 0;
	*(long *)(voice_parameters+SOUND_VOICE_START_CONTROLLER_INDEX_OFFSET)=
		*(const long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET);
	*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET)= pitch-(real)sound_pitch_range_natural_pitch(pitch_range);
	if (source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit))
	{
		const struct sound_class *sound_class= sound_class_get(sound->sound_class);
		long controller_index= *(const long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET);
		real doppler;

		if (controller_index!=NONE)
		{
			const byte *controller= (const byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;

			doppler= *(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE ?
				*(const real *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET+(char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]*4) :
				0.0f;
		}
		else
		{
			doppler= (real)sound_listener_doppler_pitch((char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET],
				(const real_point3d *)(source+SOUND_SOURCE_DATUM_POSITION_OFFSET));
		}
		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET)=
			*(const real *)((const byte *)sound_class+SOUND_CLASS_UNKNOWN48_OFFSET)*
			*(const real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN14_OFFSET)*doppler +
			*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET);
	}

	if (!((*(const word *)(source+SOUND_SOURCE_DATUM_FLAGS_OFFSET)>>_sound_source_datum_unknown9_bit)&1))
	{
		long controller_index;

		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)=
			sound_pin_decibels((real)(char)permutation->unknown0[SOUND_PERMUTATION_GAIN_OFFSET]+loudness);
		sound_console_voice_start(voice_parameters, voice_parameters+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET, channel->unknownC);
		sound_console_stream_set_function(voice_parameters+SOUND_VOICE_PARAMETERS_ENVIRONMENT_OFFSET);
		sound_channel_queue_chunk(channel_index, permutation, *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET));
		controller_index= *(const long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET);
		if (controller_index!=NONE)
		{
			byte *controller= (byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;
			byte count= 0xFF;

			if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE)
			{
				count= controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
				controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]=
					(byte)(controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]-1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
			}
			source[SOUND_SOURCE_DATUM_UNKNOWN3_OFFSET]= count;
		}
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET+1]|= FLAG(_sound_source_datum_unknown9_bit-8);
	}
	else if (channel->unknown1C)
	{
		const struct sound_permutation *playing= (const struct sound_permutation *)(uintptr_t)channel->unknown1C;

		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)=
			sound_pin_decibels((real)(char)playing->unknown0[SOUND_PERMUTATION_GAIN_OFFSET]+loudness);
		sound_console_voice_set_parameters(voice_parameters, channel->unknownC, voice_parameters+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET, 1);
	}

	if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown6_bit)&1)
	{
		return;
	}
	if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown3_bit)&1 && !channel->unknown20 &&
		sound_source_request_data(source))
	{
		sound_channel_queue_chunk(channel_index, permutation, *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET));
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]&= ~FLAG(_sound_source_datum_unknown3_bit);
	}
	if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown3_bit)&1)
	{
		return;
	}
	chunk= *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET);
	if (permutation && chunk<permutation->chunk_count-1 && (short)(chunk+1)!=NONE)
	{
		sound_source_set_sound(channel->unknown0, *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET),
			(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], (char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET],
			(short)(chunk+1));
		sound_source_request_data(source);
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown3_bit);
		return;
	}
	source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown6_bit);
	sound_channel_voice(sound_channels+channel_index)[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown3_bit);
}

/* 0x00129aa0: how the sound of a channel is heard is worked out, for its
voice of the console's sound. The channel has two flags of how its sound
goes to the console, 0 and 1: as it comes, and through a slot with a place.
A sound with no place goes as it comes, all of it or none (its flag 8). One
with a place, of the types 1 and 2, is given what sound_source_spatial_gain
says of where it is and where it points as its listener sees them: a sound
with a sound voice goes through its slot with that; any other loses it from
`decibels`, and goes as it comes. When the flags of the channel change, it
goes from the one to the other in a second (sound_unknown_0012a6d0), from
the time at 0x14 of the channel. `parameters` takes the flags, three
loudnesses in decibels and as a gain (4, 8 and 0x14), what
sound_source_spatial_gain says (0x18) and where the sound is (0x1C). Who is
told of the sound can change it all. The name is made up */
void __attribute__((stdcall)) sound_channel_compute_parameters(
	short channel_index,
	real *decibels,
	byte *parameters)
{
	struct sound_channel *channel= sound_channels+channel_index;
	byte *source= sound_source_get(channel->unknown0);
	struct sound_source *said= (struct sound_source *)(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET);
	long *fade_start= (long *)channel->unknown14;
	real unknown4;
	long time;
	long type;

	channel->unknownB= channel->unknownA;
	channel->unknownA= 0;
	type= (signed char)(source[SOUND_SOURCE_DATUM_TYPE_OFFSET]<<4)>>4;
	if (type==0)
	{
		channel->unknownA|= FLAG(_sound_channel_direct_bit);
		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_GAIN_OFFSET)=
			source[SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET+1]&1 ? 0.0f : 1.0f;
		*(real_point3d *)(parameters+SOUND_CHANNEL_PARAMETERS_POSITION_OFFSET)= *unknown_00468788;
	}
	else if (type>0 && type<=2)
	{
		long listener_index= (char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET];
		const struct sound_listener *listener=
			(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+listener_index;
		long sound_index;
		real_vector3d forward= *global_zero_vector3d;
		real_point3d position;
		real_vector3d velocity;
		real gain;

		if (type==1)
		{
			real_point3d *place= sound_source_get_position(said, listener_index, &position);

			matrix4x3_inverse_transform_point(&listener->matrix, place, place);
			if (sound_unknown_00125060(said, *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)))
			{
				const real_vector3d *direction=
					vector3d_decompress(*(const unsigned long *)(source+SOUND_SOURCE_DATUM_FORWARD_OFFSET), &forward);
				real i= direction->i;
				real j= direction->j;
				real k= direction->k;

				forward.i= listener->matrix.forward.k*k + listener->matrix.forward.j*j + listener->matrix.forward.i*i;
				forward.j= listener->matrix.left.k*k + listener->matrix.left.j*j + listener->matrix.left.i*i;
				forward.k= listener->matrix.up.k*k + listener->matrix.up.j*j + listener->matrix.up.i*i;
			}
			matrix4x3_inverse_transform_vector(&listener->matrix,
				(const real_vector3d *)(source+SOUND_SOURCE_DATUM_VELOCITY_OFFSET), &velocity);
		}
		else
		{
			position= *(const real_point3d *)(source+SOUND_SOURCE_DATUM_POSITION_OFFSET);
			if (sound_unknown_00125060(said, *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)))
			{
				vector3d_decompress(*(const unsigned long *)(source+SOUND_SOURCE_DATUM_FORWARD_OFFSET), &forward);
			}
			velocity= *(const real_vector3d *)(source+SOUND_SOURCE_DATUM_VELOCITY_OFFSET);
		}
		sound_index= *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET);
		gain= sound_source_spatial_gain((const real_vector3d *)&position, said, sound_index, &forward);
		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_GAIN_OFFSET)= gain;
		if (source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit))
		{
			channel->unknownA|= FLAG(_sound_channel_slot_bit);
		}
		else
		{
			*decibels= *decibels+sound_gain_to_decibels(gain);
			channel->unknownA|= FLAG(_sound_channel_direct_bit);
		}
		if ((said->flags>>_sound_source_unknown11_bit)&1)
		{
			channel->unknownA|= FLAG(_sound_channel_direct_bit);
		}
		*(real_point3d *)(parameters+SOUND_CHANNEL_PARAMETERS_POSITION_OFFSET)= position;
	}

	*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN14_OFFSET)=
		channel->unknownA&FLAG(_sound_channel_direct_bit) ? 0.0f : 1.0f;
	time= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
	if (!channel->unknownB)
	{
		*fade_start= 0;
		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN8_OFFSET)=
			channel->unknownA&FLAG(_sound_channel_direct_bit) ? 0.0f : SOUND_MINIMUM_DECIBELS;
		unknown4= channel->unknownA&FLAG(_sound_channel_slot_bit) ?
			sound_gain_to_decibels(*(const real *)(parameters+SOUND_CHANNEL_PARAMETERS_GAIN_OFFSET)) : SOUND_MINIMUM_DECIBELS;
	}
	else
	{
		real up= 0.0f;
		real down= SOUND_MINIMUM_DECIBELS;

		if (channel->unknownB!=channel->unknownA)
		{
			/* as the game has it: the time is taken away, not added */
			long left= (long)((unsigned long)SOUND_CHANNEL_FADE_TIME-(unsigned long)*fade_start-(unsigned long)time);

			if (left<0)
			{
				left= 0;
			}
			else if (left>SOUND_CHANNEL_FADE_TIME)
			{
				left= SOUND_CHANNEL_FADE_TIME;
			}
			*fade_start= (long)((unsigned long)time-(unsigned long)left);
		}
		if (*fade_start>0)
		{
			long elapsed= (long)((unsigned long)time-(unsigned long)*fade_start);

			if (elapsed>SOUND_CHANNEL_FADE_TIME)
			{
				*fade_start= 0;
			}
			else
			{
				real milliseconds= (real)elapsed;

				up= sound_unknown_0012a6d0(1, milliseconds, (real)SOUND_CHANNEL_FADE_TIME);
				down= sound_unknown_0012a6d0(1, milliseconds, -(real)SOUND_CHANNEL_FADE_TIME);
			}
		}
		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN14_OFFSET)=
			(real)sound_decibels_to_gain(channel->unknownA&FLAG(_sound_channel_slot_bit) ? up : down);
		channel->unknownB= channel->unknownA;
		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN8_OFFSET)=
			channel->unknownA&FLAG(_sound_channel_direct_bit) ? up : down;
		unknown4= (channel->unknownA&FLAG(_sound_channel_slot_bit) ? up : down)+
			sound_gain_to_decibels(*(const real *)(parameters+SOUND_CHANNEL_PARAMETERS_GAIN_OFFSET));
	}
	*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN4_OFFSET)= unknown4;
	*(unsigned long *)parameters= channel->unknownA;
	*(long *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWNC_OFFSET)= 0;
	*(long *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWNC_OFFSET+4)= 0;
	if ((said->flags>>_sound_source_unknown11_bit)&1)
	{
		const struct sound_class *sound_class=
			sound_class_get(sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class);

		*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN8_OFFSET)=
			*(const real *)((const byte *)sound_class+SOUND_CLASS_UNKNOWN14_OFFSET)+
			*(real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN8_OFFSET);
	}
	if (!((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown5_bit)&1))
	{
		const struct sound_source_callbacks *callbacks=
			*(const struct sound_source_callbacks *const *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET);

		if (callbacks && callbacks->heard)
		{
			byte changed[SOUND_CHANNEL_PARAMETERS_SIZE];

			memcpy(changed, parameters, sizeof(changed));
			if (callbacks->heard(*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET),
				*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET), source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET, changed))
			{
				memcpy(parameters, changed, sizeof(changed));
			}
		}
	}
}

/* a fraction that is not over 1 */
static inline __attribute__((always_inline)) real sound_fraction_up_to_one(
	real fraction)
{
	return 1.0f>fraction ? fraction : 1.0f;
}

/* 0x00127010: how loud a class of sounds is, in decibels: where its own
loudness is now (sound_classes_update takes it there), and, if the fourth of
its numbers has no flag 0, what the state of the sound of the game adds. The
globals of the sound have the state that the game is in, at 0x1F8, with the
seconds that it has been in it, and the one that it was in before, at 0x200,
with its seconds. A class has for each state four reals at 0x28: some
decibels, the seconds in which they come, those that they wait before they
go, and those in which they go. The decibels of the state come in from 0,
and those of the state before go out to 0; going back to the state before,
they come in from where they were. The name is a guess */
real sound_class_get_decibels(
	short sound_class)
{
	const real *gains= sound_class_gains+sound_class*SOUND_CLASS_GAIN_REALS;
	real decibels= gains[1];
	const byte *class_states;
	const byte *globals;
	const real *state;
	long current;
	long previous;

	if (*(const byte *)&gains[3]&1)
	{
		return decibels;
	}
	class_states= (const byte *)sound_class_get(sound_class)+SOUND_CLASS_STATES_OFFSET;
	globals= game_sound_globals;
	previous= *(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN200_OFFSET);
	current= *(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET);
	if (previous!=NONE && current==previous)
	{
		real seconds= *(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)-
			*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET);

		state= (const real *)(class_states+current*SOUND_CLASS_STATE_SIZE);
		if (!(seconds>state[2]))
		{
			return state[0]+decibels;
		}
		if (!(state[1]>=0.001f))
		{
			return decibels;
		}
		if (state[3]>=0.001f && state[3]+state[2]>seconds)
		{
			real inverse= 1.0f/state[1];
			real total= (*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)-seconds) +
				(seconds-state[2])*inverse*state[3];
			real fraction= total/state[1];

			if (!(1.0f>total*inverse))
			{
				fraction= 1.0f;
			}

			return decibels + ((state[0]-0.0f)*fraction + 0.0f);
		}

		return ((state[0]-0.0f)*
			sound_fraction_up_to_one((*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)-seconds)/state[1]) + 0.0f) +
			decibels;
	}
	if (previous!=NONE)
	{
		state= (const real *)(class_states+previous*SOUND_CLASS_STATE_SIZE);
		if (state[3]>=0.001f)
		{
			real left= *(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN204_OFFSET)-state[2]/state[3];
			real fraction;

			if (0.0f>left)
			{
				fraction= 0.0f;
			}
			else
			{
				fraction= 1.0f;
				if (!(left>1.0f))
				{
					fraction= left;
				}
			}
			decibels= ((0.0f-state[0])*fraction + state[0]) + decibels;
		}
	}
	current= *(const long *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1F8_OFFSET);
	if (current==NONE)
	{
		return decibels;
	}
	state= (const real *)(class_states+current*SOUND_CLASS_STATE_SIZE);
	if (!(state[1]>=0.001f))
	{
		return decibels;
	}

	return ((state[0]-0.0f)*sound_fraction_up_to_one(*(const real *)(globals+GAME_SOUND_GLOBALS_UNKNOWN1FC_OFFSET)/state[1]) +
		0.0f) + decibels;
}

/* seconds in which something comes or goes: no less than a thousandth */
static inline __attribute__((always_inline)) real sound_state_duration(
	real duration)
{
	return duration>0.001f ? duration : 0.001f;
}

/* 0x00126660: the two states that the globals of the sound keep at 0x1A8
are given what they are to be: two, each of something or of nothing (NONE).
Each one that is of something takes the state that is of the same thing, or
if there is none the last one that is free: one that is not taken this time
and of which there is no more than a thousandth. The state goes towards
what it is given by what the seconds of this time are of its seconds: its
four reals (the first two, as angles: by pi times as much; or all at once,
if there was none of it) and how much of it there is, towards 1. Of a state
that is not taken there is less, towards 0. The name is made up */
void sound_unknown_states_set(
	const struct sound_unknown_state *wanted)
{
	unsigned long taken= 0;
	long index;
	long slot;

	for (index= 0; index<NUMBER_OF_SOUND_UNKNOWN_STATES; index++, wanted++)
	{
		struct sound_unknown_state *states=
			(struct sound_unknown_state *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN_STATES_OFFSET);
		struct sound_unknown_state *state;
		long found= NONE;
		real step;

		if (wanted->index==NONE)
		{
			continue;
		}
		for (slot= 0; slot<NUMBER_OF_SOUND_UNKNOWN_STATES; slot++)
		{
			if (taken&(1UL<<slot))
			{
				continue;
			}
			if (states[slot].index==wanted->index)
			{
				found= slot;
				break;
			}
			if (0.001f>=states[slot].amount)
			{
				found= slot;
			}
		}
		if (found==NONE)
		{
			continue;
		}
		step= *(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET)/sound_state_duration(wanted->duration);
		state= (struct sound_unknown_state *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN_STATES_OFFSET)+found;
		if (0.001f>=state->amount)
		{
			state->unknownC= wanted->unknownC;
			state->unknown10= wanted->unknown10;
		}
		else
		{
			real turn= step*SOUND_PI;

			state->unknownC= sound_step_towards(wanted->unknownC-state->unknownC, turn)+state->unknownC;
			state->unknown10= state->unknown10+sound_step_towards(wanted->unknown10-state->unknown10, turn);
		}
		state->unknown18= state->unknown18+sound_step_towards(wanted->unknown18-state->unknown18, step);
		state->unknown14= sound_step_towards(wanted->unknown14-state->unknown14, step)+state->unknown14;
		state->amount= state->amount+sound_step_towards(1.0f-state->amount, step);
		state->duration= wanted->duration;
		state->index= wanted->index;
		taken|= 1UL<<found;
	}
	for (slot= 0; slot<NUMBER_OF_SOUND_UNKNOWN_STATES; slot++)
	{
		if (!(taken&(1UL<<slot)))
		{
			struct sound_unknown_state *state=
				(struct sound_unknown_state *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN_STATES_OFFSET)+slot;
			real step= *(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET)/
				sound_state_duration(state->duration);

			state->amount= state->amount+sound_step_towards(0.0f-state->amount, step);
		}
	}
}

/* 0x0021e3b0: the console's sound takes another handler of the sound of the
places of the map: the one that it had, if it had one, is told that it is let
go; the new one is given what sound_console_set_environment keeps, with the
byte at 1 of the console's sound, and then unknown_00470060 with that byte.
The name is made up */
void sound_console_set_environment_handler(
	struct sound_console_environment_handler *handler)
{
	byte *console= unknown_0051ebe4;
	struct sound_console_environment_handler *old=
		*(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);

	if (old)
	{
		old->functions->dispose(old);
		console= unknown_0051ebe4;
	}
	*(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET)= handler;
	handler->functions->set(handler, console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET,
		console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET]);
	console= unknown_0051ebe4;
	handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
	handler->functions->unknown8(handler, unknown_00470060, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET]);
}

/* 0x0021f5d0: a voice of the console's sound is stopped and left as new: its
stream is asked what it does and is told to stop, its other thing is told
something, it does nothing (its state is 0) and is at -64 decibels;
unknown_002ae8e0 and unknown_002ae6f0 are given it. Its number at 0x2C is
the one given, or 0 if that is less. A voice with a sound playback
controller is told so again (unknown_0037b822). While it is done the voice
has the flag 2. The name is a guess */
void sound_console_voice_stop(
	long voice_index,
	long unknown)
{
	byte *voice= unknown_0051ebe4+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;
	void *stream;
	void *object;
	long status;

	voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown2_bit);
	stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
	(*(const struct sound_console_stream_functions *const *)stream)->unknownC(stream, &status);
	stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
	(*(const struct sound_console_stream_functions *const *)stream)->unknown18(stream);
	object= *(void **)(voice+VOICE_UNKNOWN30_OFFSET);
	(*(const struct sound_console_voice_object_functions *const *)object)->unknown1C(object);
	voice[VOICE_STATE_OFFSET]&= ~VOICE_STATE_MASK;
	unknown_002ae8e0(voice);
	voice[VOICE_FLAGS_OFFSET]&= ~FLAG(_voice_unknown2_bit);
	*(real *)(voice+VOICE_DECIBELS_OFFSET)= SOUND_MINIMUM_DECIBELS;
	unknown_002ae6f0(voice);
	*(long *)(voice+VOICE_UNKNOWN2C_OFFSET)= unknown>0 ? unknown : 0;
	if (*(const long *)(voice+VOICE_CONTROLLER_INDEX_OFFSET)!=NONE)
	{
		unknown_0037b822(*(void **)(voice+VOICE_UNKNOWN24_OFFSET), 1);
	}
	unknown_002ae7a0(voice, 0.1f, 0.1f);
}

/* 0x00221980: the classes of sounds whose name has some text in it are to be
as loud as some decibels, from -64 to 0, in some seconds: at once, in none.
sound_classes_update takes them there. It is what a script of a map does to
turn classes of sounds up and down. The name is a guess */
void __attribute__((stdcall)) sound_classes_set_decibels(
	const char *name,
	real decibels,
	real seconds)
{
	long index;

	for (index= 0; index<NUMBER_OF_SOUND_CLASSES; index++)
	{
		const char *class_name= sound_class_names[index];

		if (class_name[0] && strstr(class_name, name))
		{
			real *gain= sound_class_gains+index*SOUND_CLASS_GAIN_REALS;
			real pinned= sound_pin_decibels(decibels);

			gain[0]= pinned;
			gain[2]= 0.0f>seconds ? 0.0f : seconds;
			if (seconds==0.0f)
			{
				gain[1]= pinned;
			}
		}
	}
}

/* 0x0021e410: how the samples of a sound of a kind are, for the console, and
what of the game goes with them: for the kind 2, samples of 16 bits with
two channels, and unknown_0047f0d0; for the rest, compressed samples (four
bits each, 64 in each block of 36 bytes for each channel), with one channel
or with two for the kind 1, and unknown_0047f0f0. All at 44100 samples each
second. The name is made up */
void sound_console_get_format(
	long kind,
	struct sound_console_format *format,
	void **unknown)
{
	long channels;

	if (kind==_sound_samples_uncompressed)
	{
		format->format= SOUND_CONSOLE_FORMAT_UNCOMPRESSED;
		format->channels= 2;
		format->samples_per_second= SOUND_SAMPLES_PER_SECOND;
		format->bytes_per_second= SOUND_SAMPLES_PER_SECOND*4;
		format->block_size= 4;
		format->bits_per_sample= 16;
		format->extra_size= 0;
		*unknown= unknown_0047f0d0;
		return;
	}
	format->format= SOUND_CONSOLE_FORMAT_COMPRESSED;
	format->bits_per_sample= 4;
	channels= (long)(long long)(long double)(kind==_sound_samples_compressed_stereo ? 2.0f : 1.0f);
	format->channels= (word)channels;
	format->samples_per_second= SOUND_SAMPLES_PER_SECOND;
	format->extra_size= 2;
	format->samples_per_block= SOUND_COMPRESSED_SAMPLES_PER_BLOCK;
	format->block_size= (word)(channels*SOUND_COMPRESSED_BLOCK_SIZE);
	format->bytes_per_second= format->block_size*(SOUND_SAMPLES_PER_SECOND/SOUND_COMPRESSED_SAMPLES_PER_BLOCK);
	*unknown= unknown_0047f0f0;
}

/* 0x00220730: what the console calls for the stream of a voice of the
console's sound: sound_console_voice_create_stream gives it this function, with the number of
the voice. With something to tell, of a voice that there is, the thing that
the voice has at 0x30 is told of it; and if the last number is 0 and the
voice has neither of the flags 2 and 3, unknown_002ae820 is given the voice.
The name is made up */
void __attribute__((stdcall)) sound_console_voice_stream_callback(
	long voice_index,
	long unknown,
	long unknown2)
{
	byte *console;
	byte *voice;
	void *object;
	long last;

	if (!unknown)
	{
		return;
	}
	console= unknown_0051ebe4;
	last= *(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET)-1;
	if (voice_index<0 || voice_index>last)
	{
		return;
	}
	voice= console+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;
	object= *(void **)(voice+VOICE_UNKNOWN30_OFFSET);
	(*(const struct sound_console_voice_object_functions *const *)object)->unknown18(object, voice, unknown, unknown2);
	if (!unknown2 && !(voice[VOICE_FLAGS_OFFSET]&(FLAG(_voice_unknown2_bit)|FLAG(_voice_unknown3_bit))))
	{
		unknown_002ae820(voice);
	}
}

/* 0x0021f4e0: a voice of the console's sound is given its stream, for the
samples of a kind: unknown_002ae660 makes it, with how the samples are
(sound_console_get_format), up to two pieces of them at a time, and
sound_console_voice_stream_callback for the console to call with the number
of the voice. Whether it was made. The name is made up */
boolean __attribute__((stdcall)) sound_console_voice_create_stream(
	long kind,
	long voice_index)
{
	byte *voice= unknown_0051ebe4+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;
	struct sound_console_stream_description description;
	struct sound_console_format format;
	void *object= NULL;

	memset(&description, 0, sizeof(description));
	sound_console_get_format(kind, &format, &object);
	description.format= &format;
	description.flags= 0;
	description.maximum_packets= 2;
	description.callback= sound_console_voice_stream_callback;
	description.context= voice_index;

	return unknown_002ae660(voice, object, &description, kind);
}

/* 0x00219f80: how loud a pitch range is played at a pitch that is out of
those that it is played at, in decibels, for something with the flag 13: it
comes up from silence from the second number of its parameters to the
fourth, and goes back to it from the fifth to the third. 0 for a pitch from
the fourth to the fifth, and with no flag. The name is made up */
real __attribute__((stdcall)) sound_pitch_range_fade_decibels(
	const byte *flags,
	const struct sound_pitch_range *pitch_range,
	real pitch)
{
	const short *parameters;
	real bound;
	real fraction;

	if (!(flags[1]&FLAG(_sound_unknown13_bit-8)))
	{
		return 0.0f;
	}
	parameters= sound_pitch_range_parameters(pitch_range);
	bound= (real)parameters[3];
	if (!((long double)0.0001f>fabsl((long double)pitch-(long double)bound)) && bound>pitch)
	{
		fraction= sound_interpolation_fraction((real)parameters[1], bound, pitch, TRUE);
	}
	else
	{
		bound= (real)parameters[4];
		if ((long double)0.0001f>fabsl((long double)pitch-(long double)bound) || !(pitch>bound))
		{
			return 0.0f;
		}
		fraction= sound_interpolation_fraction((real)parameters[2], bound, pitch, FALSE);
	}

	return sound_decibels_fade_in(2, 0.0f, fraction);
}

/* the stream of a voice of the console's sound is asked what it does and
told to stop, the other thing of the voice is told, the voice does nothing
and unknown_002ae8e0 is given it */
static inline __attribute__((always_inline)) void sound_console_voice_halt(
	byte *voice)
{
	void *stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
	void *object;
	long status;

	(*(const struct sound_console_stream_functions *const *)stream)->unknownC(stream, &status);
	stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
	(*(const struct sound_console_stream_functions *const *)stream)->unknown18(stream);
	object= *(void **)(voice+VOICE_UNKNOWN30_OFFSET);
	(*(const struct sound_console_voice_object_functions *const *)object)->unknown1C(object);
	voice[VOICE_STATE_OFFSET]&= ~VOICE_STATE_MASK;
	unknown_002ae8e0(voice);
	voice[VOICE_FLAGS_OFFSET]&= ~FLAG(_voice_unknown2_bit);
}

/* 0x0021f290: all the voices of the console's sound are stopped: each as
sound_console_voice_stop does it, at -64 decibels, and once more if it
plays after that; and the two numbers of three bits at 2 of it are 0. The
name is a guess */
void sound_console_voices_stop(
	void)
{
	long voice_index;

	for (voice_index= 0;
		voice_index<*(const short *)(unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET);
		voice_index++)
	{
		byte *voice= unknown_0051ebe4+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;

		voice[VOICE_FLAGS_OFFSET]|= FLAG(_voice_unknown2_bit);
		sound_console_voice_halt(voice);
		*(real *)(voice+VOICE_DECIBELS_OFFSET)= SOUND_MINIMUM_DECIBELS;
		if ((voice[VOICE_STATE_OFFSET]&VOICE_STATE_MASK)==_voice_state_playing)
		{
			sound_console_voice_halt(voice);
		}
		if (voice[VOICE_UNKNOWN2_OFFSET]&0x38)
		{
			voice[VOICE_UNKNOWN2_OFFSET]&= ~0x38;
		}
		if (voice[VOICE_UNKNOWN2_OFFSET]&7)
		{
			voice[VOICE_UNKNOWN2_OFFSET]&= ~7;
		}
	}
}

/* 0x0021eae0: the console's sound is let go, when the sound of the game is:
its voices are stopped and their streams let go, and there are no voices;
the two things of the console that each slot has are let go, and the one
that the console's sound has at 0x2AB0; unknown_002ae170 is given
unknown_0047f0d0; and the first byte of the console's sound is 0, and
the memory of the libraries of the console (xmem_static_buffer) is no more.
The name is a guess */
void sound_console_dispose(
	void)
{
	byte *console;
	void *object;
	long index;

	sound_console_voices_stop();
	console= unknown_0051ebe4;
	for (index= 0; index<*(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET); index++)
	{
		void **stream= (void **)(console+UNKNOWN_0051EBE4_VOICE_OFFSET+index*UNKNOWN_0051EBE4_VOICE_SIZE+VOICE_UNKNOWN24_OFFSET);

		if (*stream)
		{
			(*(const struct sound_console_stream_functions *const *)*stream)->unknown4(*stream);
			console= unknown_0051ebe4;
			*stream= NULL;
		}
	}
	*(short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET)= 0;
	for (index= 0; index<*(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET); index++)
	{
		struct sound_console_slot *slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET)+index;

		if (slot->stream)
		{
			unknown_00379f45(slot->stream);
			console= unknown_0051ebe4;
			slot->stream= NULL;
		}
		if (slot->unknown38)
		{
			unknown_00379f45(slot->unknown38);
			console= unknown_0051ebe4;
			slot->unknown38= NULL;
		}
	}
	object= *(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET);
	if (object)
	{
		unknown_00379f2a(object);
		console= unknown_0051ebe4;
		*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET)= NULL;
	}
	unknown_002ae170(unknown_0047f0d0);
	console= unknown_0051ebe4;
	xmem_static_buffer= NULL;
	console[0]= 0;
}

/* 0x002228d0: the list that sound_unknown_002225a0 makes, for what
unknown_0018d090 gives for no sound and a number, if its first element says
a tag and has a block of its own with something: the list goes at the start
of `result`, and its size at 0x400 of it. Nothing is done if not. What the
list is of is not known */
void __attribute__((stdcall)) sound_unknown_002228d0(
	long unknown,
	byte *result,
	long unknown2)
{
	const byte *collection= unknown_0018d090(NONE, unknown);
	const byte *element;
	const byte *tag;

	if (!collection)
	{
		return;
	}
	element= sound_block_first_element(collection+SOUND_EFFECT_COLLECTION_COUNT_OFFSET);
	if (!sound_effect_collection_has_list(element))
	{
		return;
	}
	tag= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(element+4)&0xFFFF].address;
	sound_unknown_002225a0(*(const byte *const *)(element+SOUND_EFFECT_COLLECTION_ELEMENT_BLOCK_OFFSET+4),
		*(const long *const *)(tag+SOUND_ENVIRONMENT_TAG_BLOCK_OFFSET+4), unknown2, (long *)result,
		(long *)(result+SOUND_EFFECT_LIST_SIZE));
}

/* 0x002221d0: a real from -1 to 1 as a number of 24 bits with a sign: 1 is
0x7FFFFF and -1 is 0x800000. A real that is not a number gives 0x1000000.
The name is made up */
long __attribute__((stdcall)) sound_real_to_fixed24(
	real value)
{
	if (-1.0f>value)
	{
		value= -1.0f;
	}
	else if (value>1.0f)
	{
		return (long)(long long)((long double)1.0f*(long double)8388607.0f);
	}
	else if (value>=0.0f)
	{
		return (long)(long long)((long double)value*(long double)8388607.0f);
	}

	return 0x1000000-(long)(long long)((long double)value*(long double)-8388608.0f);
}

/* 0x0021dc60: what the game gives to start a sound becomes that of a sound
of a sound effect: the effect is told of the sound (sound_effect_callbacks),
with its own index and who was to be told before it, at the end of the bytes
for who is told; with no one before, the effect has the flag 3. And the
sound has a sound playback controller: the one that it was to have, or a
new one of the effect, which the effect keeps. The name is made up */
void sound_effect_prepare_request(
	long effect_index,
	byte *request)
{
	byte *effect= (byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
	unsigned long flags;

	*(short *)(request+SOUND_REQUEST_CALLBACK_SIZE_OFFSET)= SOUND_REQUEST_CALLBACK_BYTES_SIZE;
	if (request[0]&FLAG(_sound_request_callbacks_bit))
	{
		*(long *)(request+SOUND_REQUEST_EFFECT_CALLBACKS_OFFSET)= *(const long *)(request+SOUND_REQUEST_CALLBACKS_OFFSET);
	}
	else
	{
		effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]|= FLAG(_sound_effect_datum_unknown3_bit);
		*(long *)(request+SOUND_REQUEST_EFFECT_CALLBACKS_OFFSET)= 0;
	}
	*(long *)(request+SOUND_REQUEST_EFFECT_INDEX_OFFSET)= effect_index;
	flags= *(unsigned long *)request|FLAG(_sound_request_callbacks_bit);
	*(const struct sound_source_callbacks **)(request+SOUND_REQUEST_CALLBACKS_OFFSET)= &sound_effect_callbacks;
	*(unsigned long *)request= flags;
	if (flags&FLAG(_sound_request_controller_bit))
	{
		*(long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)=
			sound_playback_controller_acquire(*(const long *)(request+SOUND_REQUEST_CONTROLLER_OWNER_OFFSET));
	}
	else
	{
		long owner= effect_index|SOUND_EFFECT_CONTROLLER_OWNER_FLAG;
		long controller_index= sound_playback_controller_new(owner);

		*(long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)= controller_index;
		if (controller_index!=NONE)
		{
			*(long *)(request+SOUND_REQUEST_CONTROLLER_OWNER_OFFSET)= owner;
			*(unsigned long *)request|= FLAG(_sound_request_controller_bit);
		}
	}
}

/* who was to be told of a sound of a sound effect before the effect: at the
end of the bytes for who is told */
static inline __attribute__((always_inline)) const struct sound_source_callbacks *sound_effect_source_callbacks(
	const void *unknown5C)
{
	return *(const struct sound_source_callbacks *const *)((const byte *)unknown5C+
		(SOUND_REQUEST_EFFECT_CALLBACKS_OFFSET-SOUND_REQUEST_CALLBACK_BYTES_OFFSET));
}

/* 0x0021d9f0: the first thing that a sound effect is told of a sound of
its own: whether the effect has no flag 4; and if it has none, what the game
says of the sound is that of the effect (sound_effect_get_source) */
boolean __attribute__((stdcall)) sound_effect_source_unknown0(
	long data,
	long sound_index,
	void *unknown5C,
	void *unknown18)
{
	long effect_index= *(const long *)((const byte *)unknown5C+
		(SOUND_REQUEST_EFFECT_INDEX_OFFSET-SOUND_REQUEST_CALLBACK_BYTES_OFFSET));
	const byte *effect= (const byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
	boolean result= !((effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]>>_sound_effect_datum_unknown4_bit)&1);

	if (result)
	{
		sound_effect_get_source(effect_index, (struct sound_source *)unknown18);
	}

	return result;
}

/* 0x0021da30, 0x0021da50 and 0x0021da70: a sound effect that is told that a
sound of its own has a channel, where it plays, and how it is heard, tells
who was to be told before it, if there was one */
void __attribute__((stdcall)) sound_effect_source_got_channel(
	long data,
	long sound_index,
	long source_index,
	void *unknown5C)
{
	const struct sound_source_callbacks *callbacks= sound_effect_source_callbacks(unknown5C);

	if (callbacks && callbacks->got_channel)
	{
		callbacks->got_channel(data, sound_index, source_index, unknown5C);
	}
}

void __attribute__((stdcall)) sound_effect_source_playing(
	long data,
	void *unknown5C,
	long sound_index,
	long pitch_range,
	long permutation,
	real position)
{
	const struct sound_source_callbacks *callbacks= sound_effect_source_callbacks(unknown5C);

	if (callbacks && callbacks->playing)
	{
		callbacks->playing(data, unknown5C, sound_index, pitch_range, permutation, position);
	}
}

boolean __attribute__((stdcall)) sound_effect_source_heard(
	long data,
	long sound_index,
	void *unknown5C,
	byte *parameters)
{
	const struct sound_source_callbacks *callbacks= sound_effect_source_callbacks(unknown5C);

	if (callbacks && callbacks->heard)
	{
		return callbacks->heard(data, sound_index, unknown5C, parameters);
	}

	return FALSE;
}

/* 0x0021db30: a sound effect that is told that a sound of its own stops does
as when it is let go, with the reason 1; and the sound is no more of the
effect: who was to be told before is told from now on */
void __attribute__((stdcall)) sound_effect_source_stopped(
	long data,
	long source_index)
{
	byte *source;

	sound_effect_source_deleted(data, source_index, 1);
	source= sound_source_get(source_index);
	*(long *)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET)= *(const long *)(source+SOUND_SOURCE_DATUM_EFFECT_CALLBACKS_OFFSET);
	*(long *)(source+SOUND_SOURCE_DATUM_EFFECT_INDEX_OFFSET)= 0;
	*(long *)(source+SOUND_SOURCE_DATUM_EFFECT_CALLBACKS_OFFSET)= 0;
}

/* 0x00219b40: the sound playback controllers are set up: the array of them,
for 64, and one more that is not in the array (unknown_0051ebdc), both of the
memory that the game has from its start. The one that is apart is made new
for no one, with a number made of the time, a random number and the clock of
the console, and with the flag that follows the byte at 0x7F of the globals
of the sound. FALSE if there is no memory. The name is a guess */
boolean sound_playback_controllers_initialize(
	void)
{
	struct data_array *data=
		shell_memory_data_new("sound playback controllers", MAXIMUM_SOUND_PLAYBACK_CONTROLLERS, UNKNOWN_0051EBD8_DATUM_SIZE);
	byte *controller;

	unknown_0051ebd8= data;
	controller= shell_memory_allocate(UNKNOWN_0051EBD8_DATUM_SIZE);
	unknown_0051ebdc= controller;
	if (!data)
	{
		return FALSE;
	}
	if (controller)
	{
		byte unknown;
		unsigned long seed;

		data_make_valid(data);
		unknown= game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET];
		seed= (unsigned long)time(NULL);
		seed^= (unsigned long)rand()^GetTickCount();
		controller= unknown_0051ebdc;
		controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]=
			(controller[UNKNOWN_0051EBD8_DATUM_SOUNDS_OFFSET]&UNKNOWN_0051EBD8_DATUM_COUNT_MASK)|(byte)(~unknown<<7);
		*(unsigned long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET)= seed;
		*(long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)= NONE;
		controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]= UNKNOWN_0051EBD8_DATUM_UNKNOWN_FLAG;
		memset(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET, 0, UNKNOWN_0051EBD8_DATUM_UNKNOWNC_SIZE);
	}

	return unknown_0051ebd8 && unknown_0051ebdc;
}

/* 0x00218850: a piece of the samples of a sound is asked for. With the flag
1 of what is asked, a piece that is not in the cache of the sound is put in
it and its samples start to be read (sound_cache_chunk_load); with the flag
0, the game waits until they are read; with the flag 2, the piece is held:
its count goes up. A piece that is in the cache is touched, so that it is
not the first to be thrown out. What comes back says, with the bit 1, that
the samples are there; with the bit 0, that they are being read; and with
the bit 2, that the piece was held. The name is a guess */
unsigned long __attribute__((stdcall)) sound_cache_chunk_request(
	long sound_index,
	const struct sound_chunk *chunk,
	long flags)
{
	const long *cache_index= (const long *)(chunk->unknown0+SOUND_CHUNK_CACHE_INDEX_OFFSET);
	boolean wait= flags&FLAG(_sound_cache_request_wait_bit);
	boolean hold= (flags>>_sound_cache_request_hold_bit)&1;
	unsigned long result= 0;

	if (*cache_index==NONE && sound_index!=NONE && (flags>>_sound_cache_request_load_bit)&1)
	{
		sound_cache_chunk_load((struct sound_chunk *)chunk, sound_index);
	}
	if (*cache_index!=NONE)
	{
		volatile byte *datum;

		lruv_block_touch(sound_lruv_cache, *cache_index);
		datum= (byte *)unknown_00502104->data+(*cache_index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;
		if (wait)
		{
			/* another thread reads them */
			while (!datum[UNKNOWN_00502104_DATUM_DONE_OFFSET])
			{
				SwitchToThread();
			}
		}
		if (datum[UNKNOWN_00502104_DATUM_DONE_OFFSET])
		{
			if (!datum[UNKNOWN_00502104_DATUM_UNKNOWN3_OFFSET])
			{
				datum[UNKNOWN_00502104_DATUM_UNKNOWN3_OFFSET]= TRUE;
			}
			result= FLAG(_sound_cache_result_unknown1_bit);
		}
		if (hold)
		{
			datum[UNKNOWN_00502104_DATUM_COUNT_OFFSET]++;
			result|= FLAG(_sound_cache_result_held_bit);
		}
	}
	if (!(result&FLAG(_sound_cache_result_unknown1_bit)) && *cache_index!=NONE)
	{
		return result|FLAG(_sound_cache_result_unknown0_bit);
	}

	return result&~(unsigned long)FLAG(_sound_cache_result_unknown0_bit);
}

/* 0x002189a0: whether a piece of samples of the cache of the sound cannot
be thrown out: one that is held, or in use, or whose samples are not read
yet. For the cache */
boolean __attribute__((stdcall)) sound_cache_block_locked(
	long block_index)
{
	const byte *datum= (const byte *)unknown_00502104->data+(block_index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;

	return datum[UNKNOWN_00502104_DATUM_COUNT_OFFSET] || datum[UNKNOWN_00502104_DATUM_USES_OFFSET] ||
		!datum[UNKNOWN_00502104_DATUM_DONE_OFFSET];
}

/* 0x002189e0: a piece of samples is thrown out of the cache of the sound:
the piece of the recording that it was of is of none, and its element is
let go. For the cache */
void __attribute__((stdcall)) sound_cache_block_deleted(
	long block_index)
{
	const byte *datum= (const byte *)unknown_00502104->data+(block_index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;
	struct sound_chunk *chunk= *(struct sound_chunk *const *)(datum+UNKNOWN_00502104_DATUM_CHUNK_OFFSET);

	*(long *)(chunk->unknown0+SOUND_CHUNK_CACHE_INDEX_OFFSET)= NONE;
	datum_delete(unknown_00502104, block_index);
}

/* 0x002185d0: the cache of the sound is made, when the sound is set up: the
array of its pieces of samples, for 512, and the cache that says where
each is, of 192 pages of 16384 bytes: three megabytes, which are taken from
the high end of the memory of the console. The name is a guess */
void sound_cache_initialize(
	void)
{
	struct allocation *allocation;
	struct data_array *data;
	struct lruv_cache *cache;

	allocation= heap_allocation;
	data= allocation->functions->allocate(allocation, SOUND_CACHE_BLOCKS_SIZE);
	if (data)
	{
		data_initialize(data, "xbox sound", MAXIMUM_CACHED_SOUND_CHUNKS, UNKNOWN_00502104_DATUM_SIZE, 0, allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}
	unknown_00502104= data;

	allocation= heap_allocation;
	cache= allocation->functions->allocate(allocation, SOUND_CACHE_LRUV_SIZE);
	lruv_initialize(cache, "xbox sound cache", SOUND_CACHE_PAGE_COUNT, SOUND_CACHE_PAGE_SIZE_BITS,
		MAXIMUM_CACHED_SOUND_CHUNKS, sound_cache_block_deleted, sound_cache_block_locked, NULL, allocation);
	cache->hole_algorithm= _lruv_hole_algorithm_age_per_page;
	sound_lruv_cache= cache;
	sound_cache_base= (long)(uintptr_t)physical_memory_allocate_high(SOUND_CACHE_PAGE_COUNT<<SOUND_CACHE_PAGE_SIZE_BITS);
}

/* 0x002186f0: the cache of the sound is closed, when a map is let go: each
piece of samples is thrown out of it. One that cannot be (it is held, or in
use, or its samples are not read yet) is waited for, while the tasks that
are not done are counted: for no more than five seconds in all. The array
of the pieces is left as not valid. The name is a guess, after
texture_cache_close */
void sound_cache_close(
	void)
{
	unsigned long start= GetTickCount();
	struct data_iterator iterator;
	const byte *datum;

	iterator.data= unknown_00502104;
	iterator.absolute_index= NONE;
	while ((datum= data_iterator_next(&iterator))!=NULL)
	{
		long index= iterator.index;
		const struct sound_chunk *chunk;
		long block_index;

		for (; ; )
		{
			const volatile byte *waited=
				(const byte *)unknown_00502104->data+(index&0xFFFF)*UNKNOWN_00502104_DATUM_SIZE;

			if (!waited[UNKNOWN_00502104_DATUM_COUNT_OFFSET] && !waited[UNKNOWN_00502104_DATUM_USES_OFFSET] &&
				waited[UNKNOWN_00502104_DATUM_DONE_OFFSET])
			{
				break;
			}
			async_globals.tasks_added= async_task_count();
			if (GetTickCount()-start>=SOUND_CACHE_CLOSE_WAIT)
			{
				break;
			}
		}
		chunk= *(const struct sound_chunk *const *)(datum+UNKNOWN_00502104_DATUM_CHUNK_OFFSET);
		block_index= *(const long *)(chunk->unknown0+SOUND_CHUNK_CACHE_INDEX_OFFSET);
		if (block_index!=NONE)
		{
			lruv_block_delete(sound_lruv_cache, block_index);
		}
	}
	unknown_00502104->valid= FALSE;
}

/* 0x0021bd00: the sounds of a ring start to fade out, in some seconds: each
from how loud it is heard now, and in the way that its class says (with the
flag 10 of the class, the kind 0; if not, the kind 1). Nothing for no ring.
The name is made up */
void __attribute__((stdcall)) sound_source_ring_fade_out_in_seconds(
	real seconds,
	long first_source_index)
{
	long source_index= first_source_index;
	long count;

	if (first_source_index==NONE)
	{
		return;
	}
	for (count= 0; ; count++)
	{
		byte *source;
		const struct sound_class *sound_class;
		short kind;

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
		source= sound_source_get(source_index);
		sound_class= sound_class_get_inline(
			sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->sound_class);
		kind= !((sound_class->internal_flags>>_sound_class_internal_unknown10_bit)&1);
		*(real *)(source+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(source_index);
		*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= kind;
		*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= (long)(seconds*1000.0f);
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= SOUND_SOURCE_DATUM_FADING_FLAG;
	}
}

/* a real at random between two, with the second seed of the game */
static inline __attribute__((always_inline)) long double sound_random_between(
	const real *bounds)
{
	unsigned long *seed= &global_random_seed_reference[1];
	long double random;

	*seed= *seed*1664525UL+1013904223UL;
	random= (long double)(long)(*seed>>16)*(long double)(1.0f/65535.0f);

	return ((long double)bounds[1]-(long double)bounds[0])*random+(long double)bounds[0];
}

/* 0x0021ce60: a place at random for a sound of a looping sound that plays
now and then: at a distance, a pitch and a yaw between those that it says
(two reals each, at 0x2C, 0x24 and 0x1C). With no distance, the zero vector,
and the other two are not drawn. The name is made up */
void looping_sound_detail_random_position(
	const byte *detail,
	real_vector3d *position)
{
	long double distance= sound_random_between((const real *)(detail+LOOPING_SOUND_DETAIL_DISTANCE_BOUNDS_OFFSET));
	long double pitch;
	long double yaw;

	if (distance==0.0L)
	{
		*position= *global_zero_vector3d;
		return;
	}
	pitch= sound_random_between((const real *)(detail+LOOPING_SOUND_DETAIL_PITCH_BOUNDS_OFFSET));
	yaw= sound_random_between((const real *)(detail+LOOPING_SOUND_DETAIL_YAW_BOUNDS_OFFSET));
	position->i= (real)(cosl(yaw)*cosl(pitch)*distance);
	position->j= (real)(sinl(yaw)*cosl(pitch)*distance);
	position->k= (real)(sinl(pitch)*distance);
}

/* 0x0021bfb0: the first thing that a looping sound is told of a sound of its
own that plays now and then: what the game says of the sound follows what it
says of the looping sound. With a place, its direction, how fast it moves,
where it is in the structure bsp, and its distances and its cone, with their
flags; with none, it points forward and does not move. The place is the three
reals of the bytes for who is told, from the place of the looping sound if
the sound has a place. FALSE if there is no such looping sound */
boolean __attribute__((stdcall)) looping_sound_source_unknown0(
	long data,
	long sound_index,
	void *unknown5C,
	void *unknown18)
{
	const byte *looping_sound= datum_try_and_get(looping_sounds, data);
	const struct sound_source *from;
	struct sound_source *said= (struct sound_source *)unknown18;
	byte *bytes= (byte *)unknown18;

	if (!looping_sound)
	{
		return FALSE;
	}
	from= (const struct sound_source *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET);
	if (((const byte *)from)[SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)
	{
		long bit;

		memcpy(bytes+SOUND_SOURCE_VELOCITY_OFFSET, (const byte *)from+SOUND_SOURCE_VELOCITY_OFFSET, sizeof(real_vector3d));
		*(long *)(bytes+SOUND_SOURCE_FORWARD_OFFSET)= *(const long *)((const byte *)from+SOUND_SOURCE_FORWARD_OFFSET);
		memcpy(bytes+SOUND_SOURCE_LOCATION_OFFSET, (const byte *)from+SOUND_SOURCE_LOCATION_OFFSET, sizeof(struct scenario_location));
		for (bit= _sound_source_minimum_distance_bit; bit<=_sound_source_outer_cone_gain_bit; bit++)
		{
			if ((((const byte *)from)[0]>>bit)&1)
			{
				bytes[0]|= FLAG(bit);
			}
			else
			{
				bytes[0]&= ~FLAG(bit);
			}
		}
		said->minimum_distance= from->minimum_distance;
		said->maximum_distance= from->maximum_distance;
		said->inner_cone_angle= from->inner_cone_angle;
		said->outer_cone_angle= from->outer_cone_angle;
		said->outer_cone_gain= from->outer_cone_gain;
	}
	else
	{
		*(unsigned long *)(bytes+SOUND_SOURCE_FORWARD_OFFSET)= vector3d_compress(global_forward3d);
		*(real_vector3d *)(bytes+SOUND_SOURCE_VELOCITY_OFFSET)= *global_zero_vector3d;
	}
	memcpy(bytes+SOUND_SOURCE_POSITION_OFFSET, unknown5C, sizeof(real_point3d));
	if ((bytes[SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK)==_sound_source_datum_type_positioned)
	{
		real *position= (real *)(bytes+SOUND_SOURCE_POSITION_OFFSET);
		const real *origin= (const real *)((const byte *)from+SOUND_SOURCE_POSITION_OFFSET);

		position[0]= origin[0]+position[0];
		position[1]= origin[1]+position[1];
		position[2]= origin[2]+position[2];
	}

	return TRUE;
}

/* 0x0021c140: whether two sounds of a looping sound that play now and then
are the same: their three reals of the bytes for who is told are, to less
than a ten thousandth */
boolean __attribute__((stdcall)) looping_sound_source_matches(
	const void *unknown5C,
	const void *unknown14)
{
	const real *a= (const real *)unknown5C;
	const real *b= (const real *)unknown14;

	return (long double)0.0001f>fabsl((long double)a[0]-(long double)b[0]) &&
		(long double)0.0001f>fabsl((long double)a[1]-(long double)b[1]) &&
		(long double)0.0001f>fabsl((long double)a[2]-(long double)b[2]);
}

/* 0x0021cc60: what the game gives to start a sound is made for a sound of a
looping sound that plays now and then. What it says of the sound is what it
says of the looping sound. For a looping sound with a place, the sound is at
a place at random from it (looping_sound_detail_random_position), and the
looping sound is told of it (looping_sound_callbacks), with that place. For
any other, the sound is at a place at random from the listener of one of the
players of the console, taken at random; FALSE if there are none. The name
is made up */
boolean __attribute__((stdcall)) looping_sound_detail_request(
	const byte *detail,
	long looping_sound_index,
	byte *request)
{
	const byte *looping_sound= (const byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	long sound_index= *(const long *)(detail+LOOPING_SOUND_DETAIL_SOUND_INDEX_OFFSET);
	const byte *sound= (const byte *)sound_definition_get(sound_index);
	byte *said= request+SOUND_REQUEST_SOURCE_OFFSET;
	real_vector3d offset;
	boolean result= FALSE;
	byte type;

	memcpy(said, looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET, LOOPING_SOUND_DATUM_SOURCE_SIZE);
	said[SOUND_SOURCE_POSITION_KIND_OFFSET]= _sound_source_position_world;
	*(long *)(request+SOUND_REQUEST_UNKNOWN8_OFFSET)= 0x10;
	*(long *)(request+SOUND_REQUEST_UNKNOWN4_OFFSET)= sound[SOUND_DEFINITION_UNKNOWN4_OFFSET] ? 0 : 2;
	type= looping_sound[LOOPING_SOUND_DATUM_SOURCE_OFFSET+SOUND_SOURCE_TYPE_OFFSET]&SOUND_SOURCE_DATUM_TYPE_MASK;
	if (type==_sound_source_datum_type_positioned)
	{
		said[SOUND_SOURCE_TYPE_OFFSET]= (said[SOUND_SOURCE_TYPE_OFFSET]&0xF1)|type;
		looping_sound_detail_random_position(detail, &offset);
		looping_sound_source_unknown0(looping_sound_index, sound_index, &offset, said);
		*(real_vector3d *)(request+SOUND_REQUEST_CALLBACK_BYTES_OFFSET)= offset;
		*(unsigned long *)request= 0x2F;
		*(long *)(request+SOUND_REQUEST_CALLBACK_DATA_OFFSET)= looping_sound_index;
		*(const struct sound_source_callbacks **)(request+SOUND_REQUEST_CALLBACKS_OFFSET)= &looping_sound_callbacks;
		*(short *)(request+SOUND_REQUEST_CALLBACK_SIZE_OFFSET)= sizeof(real_vector3d);
		result= TRUE;
	}
	else
	{
		short count= *(const short *)(players_globals->unknown0+PLAYERS_GLOBALS_LOCAL_PLAYER_COUNT_OFFSET);

		if (count>0)
		{
			unsigned long *seed= &global_random_seed_reference[1];
			const struct sound_listener *listener;
			long player_index;
			long pick;

			*seed= *seed*1664525UL+1013904223UL;
			pick= (short)(((*seed>>16)*(unsigned long)(long)count)>>16);
			player_index= players_first_local_player();
			for (; pick>0; pick--)
			{
				player_index= sound_next_local_player(player_index);
			}
			said[SOUND_SOURCE_TYPE_OFFSET]= (said[SOUND_SOURCE_TYPE_OFFSET]&0xF1)|_sound_source_datum_type_positioned;
			listener= (const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+player_index;
			looping_sound_detail_random_position(detail, &offset);
			((real *)(said+SOUND_SOURCE_POSITION_OFFSET))[0]= listener->matrix.position.x+offset.i;
			((real *)(said+SOUND_SOURCE_POSITION_OFFSET))[1]= listener->matrix.position.y+offset.j;
			((real *)(said+SOUND_SOURCE_POSITION_OFFSET))[2]= listener->matrix.position.z+offset.k;
			/* where the listener is, not the sound */
			scenario_location_from_point((struct scenario_location *)(said+SOUND_SOURCE_LOCATION_OFFSET),
				&listener->matrix.position);
			*(unsigned long *)(said+SOUND_SOURCE_FORWARD_OFFSET)= vector3d_compress(global_forward3d);
			*(real_vector3d *)(said+SOUND_SOURCE_VELOCITY_OFFSET)= *global_zero_vector3d;
			*(unsigned long *)request= 7;
			result= TRUE;
		}
	}
	said[SOUND_SOURCE_TYPE_OFFSET]= (said[SOUND_SOURCE_TYPE_OFFSET]&0xF)|(byte)(said[SOUND_SOURCE_TYPE_OFFSET]<<4);
	said[0]|= 0x80;

	return result;
}

/* a sound starts to play, as sound_source_new has it, with no one to say
why if it does not: the game has it written out here */
static inline __attribute__((always_inline)) long sound_source_new_inline(
	byte *request,
	long sound_index)
{
	long source_index= NONE;

	if (sound_available_inline())
	{
		long first;
		long second= sound_index;

		if (sound_start_test(request, sound_index, &first, NULL) && !sound_promotion_update(sound_index, &second))
		{
			source_index= sound_source_create(sound_index, first, request, second);
		}
	}

	return source_index;
}

/* 0x0021d110: a sound starts to play, with a sound effect if the tags have
one for it (sound_effect_definition_get) with something at 8 and there is
room for one more: the effect is made, what the game gives to start the sound
becomes that of a sound of the effect (sound_effect_prepare_request), and
the effect keeps the sound and is given to sound_effect_start_sounds; if the sound
does not start, the effect is let go. With the flag 0 of what
unknown_0018d090 gives for the sound, what the game says of the sound has the
flag 8. With no effect the sound starts as any other. Gives the sound that
plays, or NONE. The name is made up */
long __attribute__((stdcall)) sound_effect_source_new(
	byte *request,
	long sound_index)
{
	long unknown= *(const unsigned long *)request&FLAG(_sound_request_unknown8_bit) ?
		*(const long *)(request+SOUND_REQUEST_UNKNOWN98_OFFSET) : NONE;
	const byte *definition= NULL;
	long effect_index;

	if (sound_effect_definition_get(sound_index, unknown, &definition) &&
		*(const long *)(definition+SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET) &&
		(effect_index= datum_new(sound_effects))!=NONE)
	{
		byte *effect= (byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
		const byte *collection;
		long source_index;

		*(const byte **)(effect+SOUND_EFFECT_DATUM_DEFINITION_OFFSET)= definition;
		*(long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET)= NONE;
		collection= unknown_0018d090(sound_index, unknown);
		if (collection && collection[SOUND_EFFECT_COLLECTION_FLAGS_OFFSET]&1)
		{
			request[SOUND_REQUEST_SOURCE_OFFSET+1]|= FLAG(_sound_source_unknown8_bit-8);
		}
		sound_effect_prepare_request(effect_index, request);
		source_index= sound_source_new_inline(request, sound_index);
		if (source_index!=NONE)
		{
			effect= (byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
			effect[SOUND_EFFECT_DATUM_KIND_OFFSET]= 1;
			*(long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET)= source_index;
			*(short *)(effect+SOUND_EFFECT_DATUM_UNKNOWN6_OFFSET)= *(const short *)(request+SOUND_REQUEST_UNKNOWN4_OFFSET);
			sound_effect_start_sounds(effect_index, 0);
		}
		else
		{
			sound_effect_delete(effect_index);
		}

		return source_index;
	}

	return sound_source_new_inline(request, sound_index);
}

/* 0x00220a70: a slot of the console's sound is given what plays its sound:
the console makes it (unknown_0037d4be), and it is to be heard in five
places (6, 8, 7, 9 and 10), as far as can be and with nothing that changes
it by how far it is or how fast it moves; the slot has 0x1F at its start.
FALSE if the console does not make it. Either way the slot has nothing at
0x38. What the functions of the console are is a guess: they are not in
this executable by name. The name is made up */
boolean sound_console_slot_create_stream(
	long slot_index)
{
	byte *console= unknown_0051ebe4;
	struct sound_console_slot *slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET)+slot_index;
	struct sound_console_buffer_description description;
	struct sound_console_mixbins mixbins;
	long pairs[10];

	memset(&description, 0, sizeof(description));
	description.size= sizeof(description);
	description.flags= SOUND_CONSOLE_SLOT_BUFFER_FLAGS;
	if (unknown_0037d4be(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, &slot->stream, NULL)<0)
	{
		slot->unknown38= NULL;
		return FALSE;
	}
	pairs[0]= 6;
	pairs[1]= 0;
	pairs[2]= 8;
	pairs[3]= 0;
	pairs[4]= 7;
	pairs[5]= 0;
	pairs[6]= 9;
	pairs[7]= 0;
	pairs[8]= 10;
	pairs[9]= 0;
	mixbins.count= 5;
	mixbins.pairs= pairs;
	unknown_0037c5e4(slot->stream, &mixbins);
	*(long *)slot->unknown0= 0x1F;
	unknown_0037c620(slot->stream, SOUND_REAL_MAXIMUM, 1);
	unknown_0037c644(slot->stream, SOUND_REAL_MAXIMUM, 1);
	unknown_0037c6c1(slot->stream, 0.0f, 1);
	unknown_0037c69d(slot->stream, 0.0f, 1);
	unknown_0037c600(slot->stream, 0, 1);
	slot->unknown38= NULL;

	return TRUE;
}

/* what has changed between where a sound was to be heard and where it is to
be heard now. `old` is how many pairs there were and the pairs, each of
where and how loud; `new` is as sound_mixbins_add makes it: the pairs, and
then how many. With the same places in the same order, 1 if a loudness is
not the same, and 0 if all are: `pairs` takes each place once, with how
loud, and `count` how many. If not, 2: all is to be set again; and so with
a place that is twice and changes. The game looks at the loudness of a
place that is twice by where the pair is, not by the place: it is done the
same, and what it finds there may never have been set */
static inline __attribute__((always_inline)) long sound_mixbins_compare(
	const long *old,
	const long *new,
	long *pairs,
	long *count)
{
	volatile long loudness[32];
	unsigned long seen= 0;
	unsigned long changed= 0;
	boolean any= FALSE;
	long index;

	*count= 0;
	if (!old || old[0]!=new[SOUND_MIXBINS_COUNT_INDEX])
	{
		return 2;
	}
	for (index= 0; index<old[0]; index++)
	{
		long mixbin= new[index*2];
		long volume= new[index*2+1];
		unsigned long bit;

		if (old[1+index*2]!=mixbin)
		{
			return 2;
		}
		bit= 1UL<<(mixbin&31);
		if (old[1+index*2+1]!=volume)
		{
			if (seen&bit && (volume!=loudness[index] || changed&bit))
			{
				return 2;
			}
			changed|= bit;
			any= TRUE;
		}
		if (!(seen&bit))
		{
			pairs[*count*2+1]= volume;
			seen|= bit;
			pairs[*count*2]= mixbin;
			++*count;
			loudness[mixbin]= volume;
		}
	}

	return any;
}

/* 0x002215d0 and 0x002216f0: a stream of a voice of the console's sound, and
what plays the sound of a slot, are told where the sound is to be heard: all
again (unknown_0037c70a, unknown_0037c5e4), or only how loud in each place
(unknown_0037b81d, unknown_0037b6c3), or nothing if nothing has changed. The
names are made up */
void __attribute__((stdcall)) sound_console_stream_set_mixbins(
	const long *old,
	const long *new,
	void *stream)
{
	struct sound_console_mixbins volumes;
	long pairs[SOUND_MIXBINS_COUNT_INDEX];

	volumes.count= 0;
	volumes.pairs= pairs;
	switch (sound_mixbins_compare(old, new, pairs, &volumes.count))
	{
	case 2:
		unknown_0037c70a(stream, (const struct sound_console_mixbins *)&new[SOUND_MIXBINS_COUNT_INDEX]);
		break;
	case 1:
		unknown_0037b81d(stream, &volumes);
		break;
	}
}

void __attribute__((stdcall)) sound_console_slot_stream_set_mixbins(
	const long *old,
	const long *new,
	void *stream)
{
	struct sound_console_mixbins volumes;
	long pairs[SOUND_MIXBINS_COUNT_INDEX];

	volumes.count= 0;
	volumes.pairs= pairs;
	switch (sound_mixbins_compare(old, new, pairs, &volumes.count))
	{
	case 2:
		unknown_0037c5e4(stream, (const struct sound_console_mixbins *)&new[SOUND_MIXBINS_COUNT_INDEX]);
		break;
	case 1:
		unknown_0037b6c3(stream, &volumes);
		break;
	}
}

/* 0x00221490: the console makes something that plays a thousand bytes of
silence over and over: samples of 8 bits with one channel, a thousand each
second, to be heard in the place 14, at -100 decibels. The bytes are of the
memory that the game has from its start. Where it is told gets it. What it
is for is not known; the names of the functions of the console are not in
this executable. The name is made up */
void __attribute__((stdcall)) sound_console_create_silence(
	void **silence)
{
	struct sound_console_buffer_description description;
	struct sound_console_format format;
	struct sound_console_mixbins mixbins;
	long pair[2];
	void *buffer= NULL;
	void *samples;

	memset(&description, 0, sizeof(description));
	memset(&format, 0, SOUND_CONSOLE_FORMAT_BASE_SIZE);
	format.format= SOUND_CONSOLE_FORMAT_UNCOMPRESSED;
	format.channels= 1;
	format.samples_per_second= SOUND_SILENCE_SIZE;
	format.bytes_per_second= SOUND_SILENCE_SIZE;
	format.block_size= 1;
	format.bits_per_sample= 8;
	format.extra_size= 0;
	pair[0]= 14;
	pair[1]= 0;
	mixbins.count= 1;
	mixbins.pairs= pair;
	description.size= sizeof(description);
	description.buffer_size= 0;
	description.format= &format;
	description.mixbins= &mixbins;
	description.unknown14= 0;
	unknown_0037d7de(&description, &buffer);

	samples= shell_memory_allocate(SOUND_SILENCE_SIZE);
	memset(samples, 0, SOUND_SILENCE_SIZE);
	unknown_0037cc4a(buffer, samples, SOUND_SILENCE_SIZE);
	unknown_0037b6a7(buffer, 0);
	unknown_0037b66f(buffer, -10000);
	unknown_0037b6df(buffer, 0, 0, 1);
	*silence= buffer;
}

/* 0x0021ec00: what the console's sound does each frame. If the number that
it keeps at 0x2AD4 is not unknown_00386ad8, unknown_001915f0 is called, the
two bytes at 0x2A08 are set and the number is kept. The console does its
work (unknown_0037b844). And each voice that plays whose stream says that it
does not (the bit 16 of what it does) is stopped: its stream is told so, its
other thing too, it does nothing and unknown_002ae8e0 is given it. The name
is a guess */
void sound_console_update(
	void)
{
	byte *console= unknown_0051ebe4;
	long voice_index;

	if (*(const long *)(console+UNKNOWN_0051EBE4_UNKNOWN2AD4_OFFSET)!=unknown_00386ad8)
	{
		unknown_001915f0();
		console= unknown_0051ebe4;
		*(word *)(console+UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET)= 0x101;
		*(long *)(console+UNKNOWN_0051EBE4_UNKNOWN2AD4_OFFSET)= unknown_00386ad8;
	}
	unknown_0037b844();
	for (voice_index= 0;
		voice_index<*(const short *)(unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET);
		voice_index++)
	{
		byte *voice= unknown_0051ebe4+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE+UNKNOWN_0051EBE4_VOICE_OFFSET;

		if ((voice[VOICE_STATE_OFFSET]&VOICE_STATE_MASK)==_voice_state_playing)
		{
			void *stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
			unsigned long status;

			(*(const struct sound_console_stream_functions *const *)stream)->unknownC(stream, (long *)&status);
			if (!((status>>16)&1))
			{
				void *object;

				stream= *(void **)(voice+VOICE_UNKNOWN24_OFFSET);
				(*(const struct sound_console_stream_functions *const *)stream)->unknown18(stream);
				object= *(void **)(voice+VOICE_UNKNOWN30_OFFSET);
				(*(const struct sound_console_voice_object_functions *const *)object)->unknown1C(object);
				voice[VOICE_STATE_OFFSET]&= ~VOICE_STATE_MASK;
				unknown_002ae8e0(voice);
				voice[VOICE_FLAGS_OFFSET]&= ~FLAG(_voice_unknown2_bit);
			}
		}
	}
}

/* 0x00222250: what unknown_00192b90 says of a direction and one of five
things, from 0 to 1: the direction is made of length 1 first. 1 for a number
that is of none of the five. It is for the entries of the list that
sound_effect_entry_build writes; what it is of is not known */
real sound_unknown_00222250(
	const real_vector3d *vector,
	long index)
{
	real_vector3d direction;
	real result;

	if (index<0 || index>SOUND_UNKNOWN_00222250_MAXIMUM)
	{
		return 1.0f;
	}
	direction= *vector;
	normalize3d(&direction);
	result= unknown_00192b90(index, &direction, 1);
	if (0.0f>result)
	{
		return 0.0f;
	}

	return result>1.0f ? 1.0f : result;
}

/* 0x0021f960: the console is given a list of those that
sound_unknown_002225a0 makes: they are what the effects of the processor of
the console's sound are to have. Each entry says, with a bit for each, which
effects it is for (up to 32), and has some bytes for them: either all that
the effect has (flag 0: unknown_0037ba6f), or a part of it, from where the
entry says, which is given only if it is not what the game knows that the
effect has (unknown_0037b60d). An entry with the flag 1 is passed over. The
effects that were given something are remembered in the word at 0x22 of
unknown_00510c90. `size` is how many bytes the list is. The name is made
up */
void __attribute__((stdcall)) sound_console_apply_effect_list(
	long size,
	const byte *list)
{
	long entries_size;
	long position;

	if (!list || !size)
	{
		return;
	}
	entries_size= (long)((unsigned long)(size-(long)sizeof(long))>>2);
	for (position= 0; position<entries_size; )
	{
		const byte *entry= list+sizeof(long)+position*4;
		word bytes= *(const word *)(entry+SOUND_EFFECT_ENTRY_BYTES_OFFSET);

		if (!(entry[SOUND_EFFECT_ENTRY_FLAGS_OFFSET]&FLAG(_sound_effect_entry_skip_bit)))
		{
			unsigned long effects= *(const unsigned long *)entry;
			unsigned long effect_index;

			for (effect_index= 0; effect_index<32 && effects>=1UL<<effect_index; effect_index++)
			{
				if (!(effects&(1UL<<effect_index)))
				{
					continue;
				}
				if (entry[SOUND_EFFECT_ENTRY_FLAGS_OFFSET]&FLAG(_sound_effect_entry_whole_bit))
				{
					unknown_0037ba6f(effect_index, entry+SOUND_EFFECT_ENTRY_SIZE, 0);
				}
				else
				{
					const byte *effect= *(const byte *const *)unknown_00510c90+effect_index*SOUND_CONSOLE_EFFECT_SIZE;
					unsigned long offset= *(const word *)(entry+SOUND_EFFECT_ENTRY_OFFSET_OFFSET);

					bytes= *(const word *)(entry+SOUND_EFFECT_ENTRY_BYTES_OFFSET);
					if (offset+bytes<=*(const unsigned long *)(effect+SOUND_CONSOLE_EFFECT_STATE_SIZE_OFFSET)<<2 &&
						memcmp(entry+SOUND_EFFECT_ENTRY_SIZE,
							*(const byte *const *)(effect+SOUND_CONSOLE_EFFECT_STATE_OFFSET)+offset, bytes))
					{
						unknown_0037b60d(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), effect_index,
							*(const word *)(entry+SOUND_EFFECT_ENTRY_OFFSET_OFFSET), entry+SOUND_EFFECT_ENTRY_SIZE, bytes, 0);
					}
				}
			}
			*(word *)(unknown_00510c90+UNKNOWN_00510C90_EFFECTS_OFFSET)|= *(const word *)entry;
		}
		position+= (*(const word *)(entry+SOUND_EFFECT_ENTRY_BYTES_OFFSET)+SOUND_EFFECT_ENTRY_SIZE+3)>>2;
	}
}

/* some decibels and those of unknown_00468834, as the part that they are of
the way from -64 to some more */
static inline __attribute__((always_inline)) real sound_reverb_fraction(
	real decibels,
	real master,
	real maximum)
{
	return sound_interpolation_fraction(SOUND_MINIMUM_DECIBELS, maximum,
		(real)((long double)decibels+(long double)master), TRUE);
}

/* 0x0021ece0: an effect of the processor of the console's sound is given how
a place sounds: its four loudnesses, with that of unknown_00468834 added, in
hundredths of a decibel (from -6400 to 0, 1000 and 2000), and the rest as
they are, two of them as parts of a hundred. The name is a guess */
void __attribute__((stdcall)) sound_console_set_reverb(
	const real *environment,
	long effect_index)
{
	struct sound_console_reverb reverb;
	real master= sound_gain_to_decibels(unknown_00468834);

	reverb.count= 12;
	reverb.room= (long)(sound_reverb_fraction(environment[2], master, 0.0f)*6400.0f-6400.0f);
	reverb.room_high_frequencies= (long)(sound_reverb_fraction(environment[3], master, 0.0f)*6400.0f-6400.0f);
	reverb.room_rolloff= environment[4];
	reverb.decay_time= environment[5];
	reverb.decay_high_frequency_ratio= environment[6];
	reverb.reflections= (long)(sound_reverb_fraction(environment[7], master, 10.0f)*7400.0f-6400.0f);
	reverb.reflections_delay= environment[8];
	reverb.reverb= (long)(sound_reverb_fraction(environment[9], master, 20.0f)*8400.0f-6400.0f);
	reverb.reverb_delay= environment[10];
	reverb.diffusion= environment[11]*100.0f;
	reverb.density= environment[12]*100.0f;
	reverb.high_frequency_reference= environment[13];
	unknown_0037ba6f(effect_index, (const byte *)&reverb, 0);
}

/* 0x0021aeb0: a looping sound is made, of a tag, for something: NONE if the
sound cannot play now or there is no room. It has a sound playback controller
of what it is for. Each of its sounds that play now and then is to play first
at a time at random between the two that it says, from now, or now with its
flag 2; and with one of the flags 4 and 5 of the tag, each track takes a
number at random of the controller, to tell which recordings it has
played. The name is a guess */
long __attribute__((stdcall)) looping_sound_new(
	long definition_index,
	short unknown2,
	long unknown8,
	long owner)
{
	long looping_sound_index;
	byte *looping_sound;
	const byte *definition;
	long index;

	if (!sound_available_inline())
	{
		return NONE;
	}
	looping_sound_index= datum_new(looping_sounds);
	if (looping_sound_index==NONE)
	{
		return NONE;
	}
	looping_sound= (byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	*(short *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN2_OFFSET)= unknown2;
	*(long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)= definition_index;
	*(long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN8_OFFSET)= unknown8;
	*(long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET)= sound_playback_controller_new(owner);
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1]= 0;
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]= 0;
	definition= (const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;
	for (index= 0; index<*(const long *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET); index++)
	{
		const byte *detail= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET+4)+
			index*LOOPING_SOUND_DETAIL_SIZE;
		real seconds= 0.0f;

		if (!(detail[LOOPING_SOUND_DETAIL_FLAGS_OFFSET]&FLAG(_looping_sound_detail_unknown2_bit)))
		{
			seconds= (real)sound_random_between((const real *)(detail+LOOPING_SOUND_DETAIL_PERIOD_BOUNDS_OFFSET));
		}
		*(long *)(looping_sound+LOOPING_SOUND_DATUM_DETAIL_TIMES_OFFSET+index*4)=
			(long)(seconds*1000.0f+(real)*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET));
	}
	if (definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS)
	{
		byte *controller=
			sound_playback_controller_get(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET));

		for (index= 0; index<*(const long *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET); index++)
		{
			unsigned long *seed= (unsigned long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET);

			if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)==NONE)
			{
				*seed= *seed*1664525UL+1013904223UL;
			}
			*(unsigned long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_SEED_OFFSET+index*LOOPING_SOUND_TRACK_SIZE)= *seed;
		}
	}

	return looping_sound_index;
}

/* a real from 0 to 1 */
static inline __attribute__((always_inline)) real sound_pin_gain(
	real gain)
{
	if (0.0f>gain)
	{
		return 0.0f;
	}

	return gain>1.0f ? 1.0f : gain;
}

/* 0x00220fd0: the console's sound takes its gains: four reals, from 0 to 1,
a frequency and some decibels, which it keeps from 0x2ABC on. With them
unknown_00470060 is filled, which is how a sound is heard in a room when
nothing is in the way (the first two gains for the sound as it comes, the
third for the room, in hundredths of a decibel), and the frequency and the
decibels of the filter (unknown_00470084 and unknown_00470088). Each slot
loses its flag 1, so that it is told all again, and unknown_0047005c is set.
The name is made up */
void sound_console_set_gains(
	const real *gains)
{
	byte *console= unknown_0051ebe4;
	real *kept= (real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET);
	struct sound_console_i3dl2_source *room= (struct sound_console_i3dl2_source *)unknown_00470060;
	long millibels;
	long index;

	kept[0]= sound_pin_gain(gains[0]);
	kept[1]= sound_pin_gain(gains[1]);
	kept[2]= sound_pin_gain(gains[2]);
	kept[3]= sound_pin_gain(gains[3]);
	*(long *)&kept[4]= *(const long *)&gains[4];
	*(long *)&kept[5]= *(const long *)&gains[5];
	room->direct= sound_gain_to_millibels(kept[0]);
	room->direct_high_frequencies= sound_gain_to_millibels(kept[1]);
	room->room_rolloff= 0.0f;
	room->obstruction_low_frequency_ratio= 0.0f;
	room->occlusion_low_frequency_ratio= 0.25f;
	millibels= sound_gain_to_millibels(kept[2]);
	room->room= millibels;
	room->room_high_frequencies= millibels;
	unknown_00470084= *(const long *)&kept[4];
	unknown_00470088= kept[5];
	for (index= 0; index<*(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET); index++)
	{
		((struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET))[index].flags&=
			~FLAG(_sound_console_slot_unknown1_bit);
	}
	unknown_0047005c= TRUE;
}

/* 0x00220b90: the console's sound has one slot more, up to 64: NONE if there
are that many. The console makes for it two things. The one at 0x38 is to be
heard in the places 0 to 5, and plays at once; if the console does not make
it, the slot goes on with none. The one at 0x34 is only for the flag 0 of
`flags`: it is to be heard in the places 6, 8, 7, 9 and 10, as far as can
be, and with the flag 1 it is of another kind and plays at once; if the
console does not make it, there is no slot, and NONE. `unknown` is given to
the console for both, and the slot keeps it at its start. Gives the slot.
The name is a guess */
short __attribute__((stdcall)) sound_console_slot_new(
	unsigned long flags,
	long unknown)
{
	byte *console= unknown_0051ebe4;
	short slot_index= *(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET);
	boolean positioned= flags&1;
	boolean unknown1= (flags>>1)&1;
	struct sound_console_slot *slot;
	struct sound_console_buffer_description description;
	struct sound_console_mixbins mixbins;
	long pairs[12];
	long index;

	if (slot_index>=MAXIMUM_SOUND_CONSOLE_SLOTS)
	{
		return NONE;
	}
	*(short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET)= slot_index+1;
	slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET)+slot_index;

	memset(&description, 0, sizeof(description));
	description.size= sizeof(description);
	description.flags= SOUND_CONSOLE_SLOT_MIX_BUFFER_FLAGS;
	description.unknown14= (unsigned long)unknown;
	if (unknown_0037d4be(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, &slot->unknown38, NULL)>=0)
	{
		unknown_0037b66f(slot->unknown38, 0);
		for (index= 0; index<6; index++)
		{
			pairs[index*2]= index;
			pairs[index*2+1]= 0;
		}
		mixbins.count= 6;
		mixbins.pairs= pairs;
		unknown_0037c5e4(slot->unknown38, &mixbins);
		unknown_0037b6df(slot->unknown38, 0, 0, 0);
	}
	if (!positioned)
	{
		slot->stream= NULL;
		return slot_index;
	}

	memset(&description, 0, sizeof(description));
	description.size= sizeof(description);
	description.flags= (unknown1 ? 0x100000 : 0x80000)|0x10;
	description.unknown14= (unsigned long)unknown;
	if (unknown_0037d4be(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, &slot->stream, NULL)<0)
	{
		--*(short *)(unknown_0051ebe4+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET);
		return NONE;
	}
	pairs[0]= 6;
	pairs[1]= 0;
	pairs[2]= 8;
	pairs[3]= 0;
	pairs[4]= 7;
	pairs[5]= 0;
	pairs[6]= 9;
	pairs[7]= 0;
	pairs[8]= 10;
	pairs[9]= 0;
	mixbins.count= 5;
	mixbins.pairs= pairs;
	unknown_0037c5e4(slot->stream, &mixbins);
	*(long *)slot->unknown0= unknown;
	unknown_0037c620(slot->stream, SOUND_REAL_MAXIMUM, 1);
	unknown_0037c644(slot->stream, SOUND_REAL_MAXIMUM, 1);
	if (unknown1)
	{
		unknown_0037b6df(slot->stream, 0, 0, 0);
	}

	return slot_index;
}

/* 0x0021c190: the sound that plays of a track of a looping sound goes on to
the sound of the next state of the looping sound (the byte at 0xD of
`state`; the one at 0xC then follows it). Its pitch range is chosen by its
pitch, and its recording as for any sound: with the bits of those that were
played, which are of the track if the tag of the looping sound has one of
the flags 4 and 5 and the looping sound has a sound playback controller, and
of the pitch range if not. If the sound has a channel, and there are as many
like it as its class lets there be, or as many of its sound, one of them is
let go (reasons 5 and 6): TRUE if it is this very one. The name is made up */
boolean __attribute__((stdcall)) looping_sound_source_next_state(
	byte *looping_sound,
	byte *state,
	long source_index,
	unsigned long *seed)
{
	const byte *definition= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)&0xFFFF].address;
	byte *source= sound_source_get(source_index);
	const byte *track= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4)+
		(char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET]*LOOPING_SOUND_DEFINITION_TRACK_SIZE;
	long sound_index= *(const long *)(track+looping_sound_unknown_0044a188[(long)(char)state[LOOPING_SOUND_UNKNOWND_OFFSET]]+4);
	const struct sound_definition *sound= sound_definition_get(sound_index);
	struct sound_pitch_range *pitch_ranges;
	struct sound_same_channels same;
	unsigned long *own_bits;
	unsigned long played;
	boolean result= FALSE;
	short pitch_range;
	long permutation;
	long last;

	source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown3_bit);
	pitch_range= sound_choose_pitch_range((char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], sound,
		*(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET));
	pitch_ranges= (struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address;
	own_bits= (unsigned long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_PLAYED_OFFSET+
		(char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET]*LOOPING_SOUND_TRACK_SIZE);
	if (definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS &&
		*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET)!=NONE)
	{
		played= *own_bits;
		*own_bits= 0;
	}
	else
	{
		played= sound_pitch_range_get_played_bits(pitch_ranges+(sound->first_pitch_range_index+pitch_range), sound);
	}
	if (!(definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS))
	{
		last= (short)sound_pitch_range_get_unknown4(
			pitch_ranges+(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]), sound);
	}
	else
	{
		last= NONE;
	}
	permutation= sound_pitch_range_choose_permutation(sound, pitch_range, &played, last, seed, NULL, FALSE);
	if (definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS &&
		*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET)!=NONE)
	{
		*(unsigned long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_PLAYED_OFFSET+
			(char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET]*LOOPING_SOUND_TRACK_SIZE)= played;
	}
	else
	{
		struct sound_pitch_range *chosen= pitch_ranges+(sound->first_pitch_range_index+pitch_range);

		sound_pitch_range_set_played_bits(chosen, sound, played);
		sound_pitch_range_set_unknown4(chosen, sound, (char)permutation);
	}

	source= sound_source_get(source_index);
	sound_source_release_chunk(source);
	*(long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)= sound_index;
	source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]= (byte)pitch_range;
	source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]= (byte)permutation;
	*(short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)= 0;
	if (*(const short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)!=NONE)
	{
		const short *channels= NULL;
		short count= 0;
		long reason= 0;

		sound_source_find_same(&same, source_index);
		if (same.instance_count>=same.instance_limit)
		{
			reason= _sound_source_deleted_too_many_instances;
			channels= same.instance_channels;
			count= same.instance_count;
		}
		else if (same.sound_count>=same.sound_limit)
		{
			reason= _sound_source_deleted_too_many_sounds;
			channels= same.sound_channels;
			count= same.sound_count;
		}
		if (count>0)
		{
			short channel_index= sound_source_choose_channel(source_index, count, channels);
			long other_source_index= channel_index==NONE ? source_index : sound_channels[channel_index].unknown0;

			if (other_source_index!=NONE)
			{
				sound_source_delete(other_source_index, reason);
				result= other_source_index==source_index;
			}
		}
	}
	state[LOOPING_SOUND_UNKNOWNC_OFFSET]= state[LOOPING_SOUND_UNKNOWND_OFFSET];

	return result;
}

/* whether two reals are not the same, to some */
static inline __attribute__((always_inline)) boolean sound_real_differs(
	real a,
	real b,
	real epsilon)
{
	return !((long double)epsilon>fabsl((long double)a-(long double)b));
}

/* 0x00220790: the console's sound is told of who hears: where it is (only
the first time: the byte at the start of the console's sound is 0 then),
which way it faces, when that has changed by 0.05 or more, and how up to
two places sound, with four numbers and how much of each there is. What is
told is kept, and for each of the two places a byte says that how it sounds
has changed (at 0x2A08) and another that the rest has (at 0x2A0A). A place
that is not given sounds as sound_environment_default, with nothing of it.
The console has the height second. The name is made up */
void __attribute__((stdcall)) sound_console_set_listener(
	const byte *listener)
{
	byte *console= unknown_0051ebe4;
	const real *facing= (const real *)(listener+SOUND_CONSOLE_LISTENER_FACING_OFFSET);
	const real *kept_facing;
	long count;
	long index;

	if (!console[0])
	{
		const real *position= (const real *)listener;

		unknown_0037d598(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), position[0], position[2], position[1], 1);
		console= unknown_0051ebe4;
		memcpy(console+UNKNOWN_0051EBE4_LISTENER_POSITION_OFFSET, listener, sizeof(real_point3d));
	}
	kept_facing= (const real *)(console+UNKNOWN_0051EBE4_LISTENER_FACING_OFFSET);
	if (sound_real_differs(facing[0], kept_facing[0], 0.05f) || sound_real_differs(facing[1], kept_facing[1], 0.05f) ||
		sound_real_differs(facing[2], kept_facing[2], 0.05f) || sound_real_differs(facing[3], kept_facing[3], 0.05f) ||
		sound_real_differs(facing[4], kept_facing[4], 0.05f) || sound_real_differs(facing[5], kept_facing[5], 0.05f) ||
		!console[0])
	{
		unknown_0037d54e(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), facing[0], facing[2], facing[1], facing[3],
			facing[5], facing[4], 1);
		console= unknown_0051ebe4;
		memcpy(console+UNKNOWN_0051EBE4_LISTENER_FACING_OFFSET, facing, 2*sizeof(real_vector3d));
	}

	count= *(const long *)(listener+SOUND_CONSOLE_LISTENER_PLACE_COUNT_OFFSET);
	if (count>=NUMBER_OF_SOUND_CONSOLE_PLACES)
	{
		count= NUMBER_OF_SOUND_CONSOLE_PLACES;
	}
	for (index= 0; index<count; index++)
	{
		const byte *place= *(const byte *const *)(listener+SOUND_CONSOLE_LISTENER_PLACE_COUNT_OFFSET+4)+
			index*SOUND_CONSOLE_PLACE_SIZE;
		byte *kept= console+UNKNOWN_0051EBE4_PLACE_ENVIRONMENTS_OFFSET+index*SOUND_ENVIRONMENT_SIZE;
		byte *kept_numbers= console+UNKNOWN_0051EBE4_PLACE_NUMBERS_OFFSET+index*SOUND_CONSOLE_PLACE_NUMBERS_SIZE;
		real *kept_amount= (real *)(console+UNKNOWN_0051EBE4_PLACE_AMOUNTS_OFFSET)+index;

		if (memcmp(*(const byte *const *)place, kept, SOUND_ENVIRONMENT_SIZE) || !console[0])
		{
			memcpy(kept, *(const byte *const *)place, SOUND_ENVIRONMENT_SIZE);
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+index]= TRUE;
		}
		if (memcmp(kept_numbers, place+4, SOUND_CONSOLE_PLACE_NUMBERS_SIZE) || !console[0])
		{
			memcpy(kept_numbers, place+4, SOUND_CONSOLE_PLACE_NUMBERS_SIZE);
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+2+index]= TRUE;
		}
		if (sound_real_differs(*kept_amount, *(const real *)(place+SOUND_CONSOLE_PLACE_AMOUNT_OFFSET), 0.0001f) || !console[0])
		{
			*(long *)kept_amount= *(const long *)(place+SOUND_CONSOLE_PLACE_AMOUNT_OFFSET);
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+2+index]= TRUE;
		}
	}
	for (index= count; index<NUMBER_OF_SOUND_CONSOLE_PLACES; index++)
	{
		byte *kept= console+UNKNOWN_0051EBE4_PLACE_ENVIRONMENTS_OFFSET+index*SOUND_ENVIRONMENT_SIZE;
		real *kept_amount= (real *)(console+UNKNOWN_0051EBE4_PLACE_AMOUNTS_OFFSET)+index;

		if (memcmp(sound_environment_default, kept, SOUND_ENVIRONMENT_SIZE) || !console[0])
		{
			memcpy(kept, sound_environment_default, SOUND_ENVIRONMENT_SIZE);
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+index]= TRUE;
		}
		if (sound_real_differs(*kept_amount, 0.0f, 0.0001f) || !console[0])
		{
			*kept_amount= 0.0f;
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+2+index]= TRUE;
		}
	}
}

/* whether a bit of a row of them is set */
static inline __attribute__((always_inline)) boolean sound_bit_vector_test(
	const unsigned long *bits,
	long index)
{
	return (bits[index>>5]&(1UL<<(index&31)))!=0;
}

/* 0x00221a70: what a listener hears of each cluster through the structure
bsp's openings for the sound, and with it what is worked out of each query
of that listener (sound_query_compute). Of the openings that the listener's
cluster has (unknown_00249d10 says which), those that say something
(sound_unknown_0018cac0 turns what they say into a real) have the clusters of each
of their two sides (unknown_00249c90): if the listener's cluster is on one,
the clusters on the other are heard with that real, or with the most of
those of the openings they are behind. The listener's own cluster is never
written. The name is made up */
void __attribute__((stdcall)) sound_listener_compute_queries(
	long listener_index,
	struct sound_update_listeners *listeners)
{
	const struct sound_listener *listener=
		(const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+listener_index;

	if (listener && listener->cluster_index!=NONE)
	{
		const byte *structure_bsp= global_structure_bsp;
		real heard[MAXIMUM_STRUCTURE_BSP_CLUSTERS];
		struct sound_query *query;
		long index;

		memset(heard, 0, sizeof(heard));
		if (*(const long *)(structure_bsp+STRUCTURE_BSP_SOUND_OPENINGS_OFFSET)>0)
		{
			const long *openings= *(const long *const *)(structure_bsp+STRUCTURE_BSP_SOUND_OPENINGS_OFFSET+4);
			unsigned long mine[4]= { 0, 0, 0, 0 };

			unknown_00249d10(structure_bsp, mine, listener->cluster_index);
			for (index= 0; index<openings[0]; index++)
			{
				unsigned long sides[2][MAXIMUM_STRUCTURE_BSP_CLUSTERS/32];
				const signed char *said;
				real amount;
				long cluster_index;
				long cluster_count;
				long other;
				long side;

				if (!sound_bit_vector_test(mine, index))
				{
					continue;
				}
				said= *(const signed char *const *)(*(const byte *const *)(structure_bsp+STRUCTURE_BSP_SOUND_OPENINGS_OFFSET+4)+
					STRUCTURE_BSP_SOUND_OPENINGS_SAID_OFFSET);
				if (said[index]==NONE)
				{
					continue;
				}
				amount= sound_unknown_0018cac0(said[index]);
				array_construct_elements(sides, sizeof(sides[0]), 2, unknown_0031bc70);
				unknown_00249c90(sides[0], structure_bsp, index, sides[1]);
				cluster_index= listener->cluster_index;
				if (sound_bit_vector_test(sides[0], cluster_index))
				{
					side= 1;
				}
				else if (sound_bit_vector_test(sides[1], cluster_index))
				{
					side= 0;
				}
				else
				{
					continue;
				}
				cluster_count= *(const long *)(structure_bsp+STRUCTURE_BSP_CLUSTER_COUNT_OFFSET);
				for (other= 0; other<cluster_count; other++)
				{
					if (other!=cluster_index && sound_bit_vector_test(sides[side], other))
					{
						heard[other]= heard[other]>amount ? heard[other] : amount;
					}
				}
			}
		}

		query= (struct sound_query *)((byte *)listeners+sizeof(long));
		for (index= listeners->count; index>0; index--, query++)
		{
			if (query->listener_index==listener_index)
			{
				sound_query_compute(query, heard, listener);
			}
		}
	}
}

/* 0x00222150: two reals for a sound that is in another cluster than the
listener, from the portal of the listener's cluster that is nearest the
listener (unknown_0018cfd0): the first is nearer 1 the further the sound is
from that portal (unknown_0014b240), 0 at it; the second is what the
listener hears of the sound's cluster. With no portal, 1 and 0. The cluster
of the listener is taken in ecx, where the listener is in esi and where the
reals go in ebx. The name is made up */
void __attribute__((stdcall)) sound_query_portal_results(
	long listener_cluster_index,
	const real_point3d *listener_position,
	real *results,
	const real *heard,
	const real_point3d *position,
	long cluster_index)
{
	const byte *structure_bsp= global_structure_bsp;
	real distance;
	long portal_index= unknown_0018cfd0(listener_cluster_index, listener_position, &distance);

	if (portal_index!=NONE)
	{
		const byte *portal= *(const byte *const *)(structure_bsp+STRUCTURE_BSP_PORTALS_OFFSET)+
			portal_index*STRUCTURE_BSP_PORTAL_SIZE;
		real amount= heard[cluster_index];
		real away= unknown_0014b240(structure_bsp, portal, position);

		results[1]= amount;
		results[0]= (real)(1.0L-1.0L/((long double)away+1.0L));
	}
	else
	{
		results[0]= 1.0f;
		results[1]= 0.0f;
	}
}

/* what unknown_001de630 says of a line through the level: how far along it
something is, from 0 to 1, and more that is not known */
struct sound_line_result
{
	real fraction;
	byte unknown4[0x418];
};

/* 0x00221e30: what is worked out of a query, for a listener that is not in
the query's leaf. The first real has added to it how much of the line
between the two is behind the level: a line is tested from the listener to
the sound and, if it hits, another from the sound to the listener, and what
is between the two hits is that much (and the two hits are kept in the
query, with the flag at 0x45). If only the first hits, it is what is after
it. A query with the flag at 0x47 has no lines: with the flag at 0x44
nothing is added, and without it and in another cluster, how much of the way
from the listener to the sound's cluster (unknown_00249d60) and on to the
sound is the first part. For a sound in another cluster the other two reals
are those of sound_query_portal_results, with 0 for the first when the two
clusters have the same short at 0x7A; with the flag at 0x44 they are 0 and
what the listener hears of the sound's cluster. The query is taken in edi.
The name is made up */
void __attribute__((stdcall)) sound_query_compute(
	struct sound_query *query,
	const real *heard,
	const struct sound_listener *listener)
{
	real amount;
	real results[2];

	if (listener->leaf_index==query->leaf_index)
	{
		return;
	}
	amount= 0.0f;
	if (!query->unknown47)
	{
		struct sound_line_result first;
		struct sound_line_result second;
		const byte *unknown= unknown_004ed280+global_structure_bsp_index*UNKNOWN_004ED280_SIZE+1;
		real_vector3d there;
		real_vector3d back;

		there.i= query->position.x-listener->matrix.position.x;
		there.j= query->position.y-listener->matrix.position.y;
		there.k= query->position.z-listener->matrix.position.z;
		if (unknown_001de630(&first, 1, global_collision_bsp, 1.0f, 0x100, unknown, &listener->matrix.position, &there))
		{
			back.i= 0.0f-there.i;
			back.j= 0.0f-there.j;
			back.k= 0.0f-there.k;
			if (unknown_001de630(&second, 1, global_collision_bsp, 1.0f, 0x100, unknown, &query->position, &back))
			{
				real far_fraction= second.fraction;
				real near_fraction= first.fraction;
				boolean between;

				amount= 1.0f-far_fraction-near_fraction;
				if (0.0f>amount)
				{
					amount= 0.0f;
					between= FALSE;
				}
				else if (amount>1.0f)
				{
					amount= 1.0f;
					between= TRUE;
				}
				else
				{
					between= amount>0.0f;
				}
				if (between)
				{
					query->unknown45= TRUE;
					query->near_point.x= there.i*near_fraction+listener->matrix.position.x;
					query->near_point.y= there.j*near_fraction+listener->matrix.position.y;
					query->near_point.z= there.k*near_fraction+listener->matrix.position.z;
					query->near_fraction= near_fraction;
					query->far_point.x= back.i*far_fraction+query->position.x;
					query->far_point.y= back.j*far_fraction+query->position.y;
					query->far_point.z= back.k*far_fraction+query->position.z;
					query->far_fraction= far_fraction;
				}
			}
			else
			{
				amount= 1.0f-first.fraction;
			}
		}
	}
	else if (!query->unknown44 && listener->cluster_index!=query->cluster_index)
	{
		real around= unknown_00249d60(listener->cluster_index, query->cluster_index, global_structure_bsp);
		real straight= distance3d(&query->position, &listener->matrix.position);
		real all= straight+around;

		amount= 1.0f-straight/(all>0.001f ? all : 0.001f);
	}

	if (listener->cluster_index!=NONE && query->cluster_index!=NONE && listener->cluster_index!=query->cluster_index)
	{
		if (query->unknown44)
		{
			query->results[1]= 0.0f;
			*(long *)&query->results[2]= *(const long *)&heard[query->cluster_index];
		}
		else
		{
			const byte *clusters;

			sound_query_portal_results(listener->cluster_index, &listener->matrix.position, results, heard, &query->position,
				query->cluster_index);
			clusters= *(const byte *const *)((const byte *)global_structure_bsp+STRUCTURE_BSP_CLUSTER_COUNT_OFFSET+4);
			query->results[1]= *(const short *)(clusters+listener->cluster_index*STRUCTURE_BSP_CLUSTER_SIZE+
				STRUCTURE_BSP_CLUSTER_UNKNOWN7A_OFFSET)==*(const short *)(clusters+query->cluster_index*
				STRUCTURE_BSP_CLUSTER_SIZE+STRUCTURE_BSP_CLUSTER_UNKNOWN7A_OFFSET) ? 0.0f : results[0];
			query->results[2]= results[1];
		}
	}
	query->results[0]+= amount;
}

/* one of the numbers of an element of the effects of a tag: of which type
it is (0, an integer; 1, a real), what it is when nothing else says, and
what it has to be between */
struct sound_effect_parameter
{
	long type;
	real initial;
	real minimum;
	real maximum;
};

typedef char verify_sound_effect_parameter_size[
	sizeof(struct sound_effect_parameter) == 0x10 ? 1 : -1];

/* one of the two reals a function of a sound effect is given, from those
of the sound: the one of the number 1 is divided by the function's real and
only what is after its point is kept, and the one of the number 3 is
multiplied by what sound_unknown_00222250 says of the three reals at 0x10 and
the number of the entry, when there is one */
static inline __attribute__((always_inline)) real sound_effect_function_input(
	const real *inputs,
	short input,
	real value,
	real scale,
	long number)
{
	if (input==_sound_effect_input_scaled)
	{
		value= (real)fmodl((long double)value/(long double)scale, 1.0L);
	}
	if (number!=NONE && input==_sound_effect_input_directional)
	{
		value= sound_unknown_00222250((const real_vector3d *)(inputs+SOUND_EFFECT_INPUTS_VECTOR_INDEX), number)*value;
	}

	return value;
}

/* 0x002222c0: an entry of the list for the console's sound, from an element
of the effects of a tag. It starts with three numbers: which effects of the
console it is for (sound_unknown_00191660 says, of the element's name and the
number), and the two that the element has at 8. Then one number for each of
the element's parameters, after a first one that says which kind of thing
it is when the element has the flag 0: 10, 4 or NONE, by its name. With that
flag the reals are written as they are; without it, as
sound_real_to_fixed24 has them. A number is what its parameter says, or
what the override has for it when it has its bit: either a real, or a
function of two of the reals of the sound (unknown_0013b390), which is put
between the two reals of the function unless it has one of four flags. In
the end it is put between the two of the parameter. The entry is taken in
eax. The name is made up */
void sound_effect_entry_build(
	long *entry,
	const long *element,
	const byte *override,
	const real *inputs,
	long number)
{
	long *out= &entry[SOUND_EFFECT_ENTRY_SIZE/4];
	long override_index= 0;
	long index;

	memcpy(entry, &element[1], SOUND_EFFECT_ENTRY_SIZE);
	entry[0]= sound_unknown_00191660(element[0], number);
	if (*(const byte *)&element[2]&FLAG(_sound_effect_entry_whole_bit))
	{
		long kind= NONE;

		if (element[0]==SOUND_EFFECT_NAME_KIND10)
		{
			kind= 10;
		}
		else if (element[0]==SOUND_EFFECT_NAME_KIND4)
		{
			kind= 4;
		}
		*out= kind;
	}

	for (index= 0; index<element[4]; index++, out++)
	{
		const struct sound_effect_parameter *parameter=
			(const struct sound_effect_parameter *)(uintptr_t)element[5]+index;
		real value= parameter->initial;

		if (override && (*(const unsigned long *)(override+SOUND_EFFECT_OVERRIDE_MASK_OFFSET)&(1UL<<(index&31))))
		{
			signed char which= (*(const signed char *const *)(override+SOUND_EFFECT_OVERRIDE_WHICH_OFFSET))[override_index];

			if (which&1)
			{
				const byte *function= *(const byte *const *)(override+SOUND_EFFECT_OVERRIDE_FUNCTIONS_OFFSET)+
					(which>>1)*SOUND_EFFECT_FUNCTION_SIZE;
				short first_input= *(const short *)function;
				boolean ranged= (*(const byte *const *)(function+SOUND_EFFECT_FUNCTION_DATA_OFFSET+4))[1]&1;
				real first= inputs[first_input];
				real second= ranged ? inputs[*(const short *)(function+2)] : 0.0f;
				real scale= *(const real *)(function+SOUND_EFFECT_FUNCTION_SCALE_OFFSET);

				first= sound_effect_function_input(inputs, first_input, first, scale, number);
				if (ranged)
				{
					second= sound_effect_function_input(inputs, *(const short *)(function+2), second, scale, number);
				}
				if (*(const byte *const *)(function+SOUND_EFFECT_FUNCTION_DATA_OFFSET+4) &&
					*(const long *)(function+SOUND_EFFECT_FUNCTION_DATA_OFFSET)>0)
				{
					const byte *data;

					value= unknown_0013b390(function+SOUND_EFFECT_FUNCTION_DATA_OFFSET, first, second);
					data= *(const byte *const *)(function+SOUND_EFFECT_FUNCTION_DATA_OFFSET+4);
					if (!(data[1]&0xF0))
					{
						real low= *(const real *)(data+4);
						real high= *(const real *)(data+8);

						if (0.0f>value)
						{
							value= 0.0f;
						}
						else if (value>1.0f)
						{
							value= 1.0f;
						}
						value= (high-low)*value+low;
					}
				}
				else
				{
					value= 0.0f;
				}
			}
			else
			{
				value= (*(const real *const *)(override+SOUND_EFFECT_OVERRIDE_REALS_OFFSET))[which>>1];
			}
			override_index++;
		}
		if (parameter->minimum>value)
		{
			value= parameter->minimum;
		}
		else if (value>parameter->maximum)
		{
			value= parameter->maximum;
		}

		if (*(const byte *)&element[2]&FLAG(_sound_effect_entry_whole_bit))
		{
			if (parameter->type==0)
			{
				out[1]= lrintf(value);
			}
			else if (parameter->type==1)
			{
				*(real *)&out[1]= value;
			}
		}
		else
		{
			if (parameter->type==0)
			{
				out[0]= lrintf(value);
			}
			else if (parameter->type==1)
			{
				out[0]= sound_real_to_fixed24(value);
			}
		}
	}
}

/* 0x0021f720: one of the two streams of the console's sound (the short at
the start of what a voice has of its environment says which) is given the
shape of a function of a tag: a thousand bytes, from 0 to 255, of what the
function is from 0 to 1 (unknown_0013b390), put between the two reals of the
function unless it has one of four flags; 0 with no function. It is only
written when the number at 0xC is not the one of the last time, with the
stream paused. Then the stream is told two things with a 0. Nothing is done
when the short at 2 is not from 0 to 31. The name is made up */
void __attribute__((stdcall)) sound_console_stream_set_function(
	const byte *environment)
{
	short number= *(const short *)(environment+SOUND_VOICE_ENVIRONMENT_NUMBER_OFFSET);
	short stream_index= *(const short *)environment;
	byte *console;
	void *stream;

	if (number<0 || number>31 || stream_index<0 || stream_index>=NUMBER_OF_SOUND_CONSOLE_FUNCTION_STREAMS)
	{
		return;
	}
	console= unknown_0051ebe4;
	stream= *(void **)(console+UNKNOWN_0051EBE4_STREAMS_OFFSET+stream_index*4);
	if (*(const long *)(environment+SOUND_VOICE_ENVIRONMENT_UNKNOWNC_OFFSET)!=
		*(const long *)(console+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET+stream_index*4))
	{
		unsigned long size;
		byte *bytes;
		unsigned long index;

		unknown_0037b73f(stream, TRUE);
		unknown_0037b7b3(stream, 0, SOUND_CONSOLE_FUNCTION_STREAM_SIZE, &bytes, &size, NULL, NULL, 0);
		for (index= 0; index<size; index++)
		{
			const byte *function= *(const byte *const *)(environment+SOUND_VOICE_ENVIRONMENT_UNKNOWN8_OFFSET);
			real value;

			if (*(const byte *const *)(function+4) && *(const long *)function>0)
			{
				const byte *data;

				value= unknown_0013b390(function, (real)(long)index*0.001f, 0.0f);
				data= *(const byte *const *)(function+4);
				if (!(data[1]&0xF0))
				{
					real low= *(const real *)(data+4);
					real high= *(const real *)(data+8);

					if (0.0f>value)
					{
						value= 0.0f;
					}
					else if (value>1.0f)
					{
						value= 1.0f;
					}
					value= (high-low)*value+low;
				}
			}
			else
			{
				value= 0.0f;
			}
			bytes[index]= (byte)lrintf(value*255.0f);
		}
		unknown_0037b73f(stream, FALSE);
		*(long *)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET+stream_index*4)=
			*(const long *)(environment+SOUND_VOICE_ENVIRONMENT_UNKNOWNC_OFFSET);
	}
	unknown_0037b797(stream, 0);
	unknown_0037b66f(stream, 0);
}

/* 0x0021d060: what is left of something that looked at a looping sound's
tag when it starts, for a source of the type 1 and with unknown_004e636d
set: it asks the two distances of the sound of the first track that has one
(it looks at the first track each time, as many times as there are tracks)
and, if there is none or the nearer distance is 0, goes through the details
up to the first that has a sound. Nothing is done with any of it: what it
said, most likely, is not in this build of the game. The tag is taken in
eax and the source in ebx. The name is made up */
void looping_sound_check_distances(
	long definition_index,
	const struct sound_source *source)
{
	const byte *definition;
	long track_count;
	long index;

	if (!unknown_004e636d || (((const byte *)source)[3]&0xF)!=1)
	{
		return;
	}
	definition= (const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;
	track_count= *(const long *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET);
	if (track_count>0)
	{
		const byte *track= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4);
		short track_index= 0;
		boolean found= TRUE;

		while (*(const long *)(track+LOOPING_SOUND_DEFINITION_TRACK_SOUND_INDEX_OFFSET)==NONE)
		{
			track_index++;
			if (track_index>=track_count)
			{
				found= FALSE;
				break;
			}
		}
		if (found)
		{
			real minimum_distance=
				sound_get_minimum_distance(source, *(const long *)(track+LOOPING_SOUND_DEFINITION_TRACK_SOUND_INDEX_OFFSET));

			sound_get_maximum_distance(source, *(const long *)(track+LOOPING_SOUND_DEFINITION_TRACK_SOUND_INDEX_OFFSET));
			if (minimum_distance!=0.0f)
			{
				return;
			}
		}
	}
	for (index= 0; index<*(const long *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET); index++)
	{
		if (*(const long *)(*(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET+4)+
			index*LOOPING_SOUND_DETAIL_SIZE+LOOPING_SOUND_DETAIL_SOUND_INDEX_OFFSET)!=NONE)
		{
			break;
		}
	}
}

/* 0x0021d970: the first thing that a sound that a sound effect started by
itself is told: whether its effect is still there (the one of the index at
the end of the bytes for who is told, and with that identifier) and has no
flag 0; and then, if the effect has no flag 4, what the game says of the
sound is that of the effect (sound_effect_get_source). The name is made
up */
boolean __attribute__((stdcall)) sound_effect_sound_unknown0(
	long data,
	long sound_index,
	void *unknown5C,
	void *unknown18)
{
	long effect_index= *(const long *)((const byte *)unknown5C+
		(SOUND_REQUEST_EFFECT_INDEX_OFFSET-SOUND_REQUEST_CALLBACK_BYTES_OFFSET));
	const struct data_array *effects;
	const byte *effect;
	short identifier;

	if (effect_index==NONE)
	{
		return FALSE;
	}
	effects= sound_effects;
	if ((effect_index&0xFFFF)>=effects->first_unallocated)
	{
		return FALSE;
	}
	effect= (const byte *)effects->data+(effect_index&0xFFFF)*effects->size;
	identifier= *(const short *)effect;
	if (!identifier || identifier!=effect_index>>16)
	{
		return FALSE;
	}
	if (effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]&FLAG(_sound_effect_datum_unknown0_bit))
	{
		return FALSE;
	}
	if (!((effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]>>_sound_effect_datum_unknown4_bit)&1))
	{
		sound_effect_get_source(effect_index, (struct sound_source *)unknown18);
	}

	return TRUE;
}

/* 0x0021d630: the sounds of a sound effect are started, for what the effect
is at: 0, when it starts; 1, in between; 2, when it ends. Each of the
elements of the effect's definition (0x10 bytes: a group, a tag, a real and
flags) is a sound or a looping sound.

A sound only starts at 0, without its flag 0, or at 2, with its flag 1. It
is started as the game starts any, from what sound_effect_get_source says of
the effect, with the real as its gain; at 0 it also has the sound playback
controller of the effect's owner and is told of things through
sound_effect_sound_callbacks, with the effect at the end of the bytes for
it.

A looping sound is given to looping_sound_start at all three, with the real
added to the one at 8 of what the game says of the sound, with the state of
sound_effect_looping_sound_states and two flags: 1 if the element has the
flag 3, or the flag 4 when the effect has the flags 1 and 4; 2 at the end.
For an effect of the kind 2 and an element with the flag 5, the state and
the first flag are those of the element of unknown_004ed28c of the effect.
The short at 6 of the effect goes without its bit 0 if the looping sound's
tag has the flag 3, and without its bit 1 if it has the flag 6. The name is
made up */
void sound_effect_start_sounds(
	long effect_index,
	long phase)
{
	const byte *effect= (const byte *)sound_effects->data+(effect_index&0xFFFF)*SOUND_EFFECT_DATUM_SIZE;
	long owner= *(const long *)((const byte *)sound_playback_controller_get(
		*(const long *)(effect+SOUND_EFFECT_DATUM_CONTROLLER_INDEX_OFFSET))+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET);
	long source[LOOPING_SOUND_DATUM_SOURCE_SIZE/4];
	const byte *definition;
	long unknown_state;
	boolean unknown_flag;
	long index;

	*(word *)source= 0;
	sound_effect_get_source(effect_index, (struct sound_source *)source);
	if (effect[SOUND_EFFECT_DATUM_KIND_OFFSET]==2)
	{
		const byte *unknown= (const byte *)unknown_004ed28c->data+
			(*(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET)&0xFFFF)*UNKNOWN_004ED28C_DATUM_SIZE;

		unknown_flag= (unknown[4]>>4)&1;
		unknown_state= (signed char)unknown[2];
	}
	definition= *(const byte *const *)(effect+SOUND_EFFECT_DATUM_DEFINITION_OFFSET);
	for (index= 0; index<*(const long *)(definition+SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET); index++)
	{
		const byte *element= *(const byte *const *)(definition+SOUND_EFFECT_DEFINITION_UNKNOWN8_OFFSET+4)+
			index*SOUND_EFFECT_SOUND_SIZE;
		long tag_index= *(const long *)(element+4);
		byte flags= element[SOUND_EFFECT_SOUND_FLAGS_OFFSET];

		if (tag_index==NONE)
		{
			continue;
		}
		if (*(const unsigned long *)element==LOOPING_SOUND_TAG)
		{
			const byte *looping_sound= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
			long copy[LOOPING_SOUND_DATUM_SOURCE_SIZE/4];
			unsigned long unknown6;
			unsigned long start_flags;
			boolean flag;
			long state;

			if ((flags&FLAG(_sound_effect_sound_unknown5_bit)) && effect[SOUND_EFFECT_DATUM_KIND_OFFSET]==2)
			{
				state= sound_effect_looping_sound_unknown_states[unknown_state];
				flag= unknown_flag;
			}
			else
			{
				state= sound_effect_looping_sound_states[phase];
				flag= (flags&FLAG(_sound_effect_sound_unknown3_bit)) ||
					(((effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]>>_sound_effect_datum_unknown4_bit)&1) &&
					((effect[SOUND_EFFECT_DATUM_FLAGS_OFFSET]>>_sound_effect_datum_unknown1_bit)&1) &&
					(flags&FLAG(_sound_effect_sound_unknown4_bit)));
			}
			unknown6= *(const word *)(effect+SOUND_EFFECT_DATUM_UNKNOWN6_OFFSET);
			if (*(const unsigned long *)looping_sound&FLAG(3))
			{
				unknown6&= ~1UL;
			}
			if (*(const unsigned long *)looping_sound&FLAG(6))
			{
				unknown6&= ~2UL;
			}
			start_flags= flag!=0;
			if (phase==2)
			{
				start_flags|= 2;
			}
			memcpy(copy, source, sizeof(copy));
			*(real *)&copy[2]= *(const real *)(element+SOUND_EFFECT_SOUND_GAIN_OFFSET)+*(const real *)&copy[2];
			looping_sound_start(tag_index, unknown6,
				effect[SOUND_EFFECT_DATUM_KIND_OFFSET] ? *(const long *)(effect+SOUND_EFFECT_DATUM_SOURCE_INDEX_OFFSET) :
				effect_index|SOUND_EFFECT_CONTROLLER_OWNER_FLAG,
				owner, (const struct sound_source *)copy, state, start_flags, 0);
		}
		else if (*(const unsigned long *)element==SOUND_TAG)
		{
			long request[SOUND_REQUEST_SIZE/4];
			byte *bytes= (byte *)request;
			const byte *globals;
			long listener_index;
			long rule_index;

			if (phase==0)
			{
				if (flags&FLAG(_sound_effect_sound_unknown0_bit))
				{
					continue;
				}
			}
			else if (phase!=2 || !(flags&FLAG(_sound_effect_sound_unknown1_bit)))
			{
				continue;
			}
			*(unsigned long *)(bytes+SOUND_REQUEST_UNKNOWN4_OFFSET)= *(const word *)(effect+SOUND_EFFECT_DATUM_UNKNOWN6_OFFSET);
			memcpy(bytes+SOUND_REQUEST_SOURCE_OFFSET, source, sizeof(source));
			*(unsigned long *)bytes= FLAG(0)|FLAG(2)|FLAG(_sound_request_controller_bit);
			if (phase!=2)
			{
				*(long *)(bytes+SOUND_REQUEST_CONTROLLER_OWNER_OFFSET)= owner;
				memset(bytes+SOUND_REQUEST_CALLBACK_BYTES_OFFSET, 0, SOUND_REQUEST_CALLBACK_BYTES_SIZE);
				*(unsigned long *)bytes= FLAG(0)|FLAG(2)|FLAG(_sound_request_controller_bit)|FLAG(_sound_request_callbacks_bit);
				*(const struct sound_source_callbacks **)(bytes+SOUND_REQUEST_CALLBACKS_OFFSET)= &sound_effect_sound_callbacks;
				*(word *)(bytes+SOUND_REQUEST_CALLBACK_SIZE_OFFSET)= SOUND_REQUEST_CALLBACK_BYTES_SIZE;
				*(long *)(bytes+SOUND_REQUEST_EFFECT_INDEX_OFFSET)= effect_index;
			}
			*(long *)(bytes+SOUND_REQUEST_GAIN_OFFSET)= *(const long *)(element+SOUND_EFFECT_SOUND_GAIN_OFFSET);
			*(unsigned long *)bytes|= FLAG(_sound_request_gain_bit);
			globals= game_sound_globals;
			if (globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET] && globals[GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET] &&
				globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET] && sound_start_test(bytes, tag_index, &listener_index, NULL) &&
				!sound_promotion_update(tag_index, &rule_index))
			{
				sound_source_create(tag_index, listener_index, bytes, rule_index);
			}
		}
	}
}

/* 0x0021ac30: a looping sound is told that a sound of one of its tracks
has a channel: the track (the byte at 4 of the bytes for who is told) counts
one more, in its byte at 0xE. Not for a sound that is fading out, nor if the
looping sound is no longer there. The name is made up */
void __attribute__((stdcall)) looping_sound_track_source_got_channel(
	long data,
	long sound_index,
	long source_index,
	void *unknown5C)
{
	const byte *source= sound_source_get(source_index);
	byte *looping_sound;

	if (((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>7)&1) &&
		*(const long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)>*(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET))
	{
		return;
	}
	looping_sound= datum_try_and_get(looping_sounds, data);
	if (looping_sound)
	{
		looping_sound[LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+LOOPING_SOUND_TRACK_CHANNELS_OFFSET+
			(char)((const byte *)unknown5C)[LOOPING_SOUND_TOLD_TRACK_OFFSET]*LOOPING_SOUND_TRACK_SIZE]++;
	}
}

/* 0x0021acc0: a looping sound is asked how a sound of one of its tracks is
heard. Only one whose number at 8 is its tag with the flags 0x6000 answers:
four reals at 0x28 of what is asked, each the decibels of one of the four
gains at the end of the looping sound, as the sound has them
(sound_definition_decibels, with the real at 4 of what the looping sound
says of its sounds), plus the real at 8 of that; and the flag 4 of what is
asked. FALSE if it does not answer. The name is made up */
boolean __attribute__((stdcall)) looping_sound_track_source_heard(
	long data,
	long sound_index,
	void *unknown5C,
	byte *parameters)
{
	const byte *looping_sound= datum_try_and_get(looping_sounds, data);
	const struct sound_definition *sound;
	long index;

	if (!looping_sound || *(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN8_OFFSET)!=
		(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)|LOOPING_SOUND_UNKNOWN8_OWN_FLAGS))
	{
		return FALSE;
	}
	sound= (const struct sound_definition *)(uintptr_t)global_tag_instances[sound_index&0xFFFF].address;
	for (index= 0; index<NUMBER_OF_LOOPING_SOUND_GAINS; index++)
	{
		real decibels= sound_definition_decibels(sound,
			sound_gain_to_decibels(*(const real *)(looping_sound+LOOPING_SOUND_DATUM_GAINS_OFFSET+index*4)),
			*(const real *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+4));

		*(real *)(parameters+SOUND_HEARD_DECIBELS_OFFSET+index*4)=
			decibels+*(const real *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+8);
	}
	*(unsigned long *)parameters|= FLAG(_sound_heard_decibels_bit);

	return TRUE;
}

/* 0x0021adb0: a looping sound is told that a sound of one of its tracks is
let go. The sound leaves the ring of the sounds of its track: the one
before it and the one after it go one to the other. If the looping sound is
still there, it counts one sound less (its byte at 0x51), and if the sound
was the first of its track, the first is now the one after it, or none if it
was alone. And if the sound has a sound playback controller that is still
there and has something at 4, and the byte at 6 of the bytes for who is told
is 0xFF, the count of the controller goes down by one, without its flag. The
name is made up */
void __attribute__((stdcall)) looping_sound_track_source_deleted(
	long data,
	long source_index,
	long reason)
{
	byte *looping_sound= datum_try_and_get(looping_sounds, data);
	byte *source= sound_source_get(source_index);
	long track= (char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET];
	long next= *(const long *)(source+SOUND_SOURCE_DATUM_NEXT_OFFSET);
	long following= NONE;
	long controller_index;

	if (next!=source_index)
	{
		*(long *)(sound_source_get(*(const long *)(source+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET))+SOUND_SOURCE_DATUM_NEXT_OFFSET)= next;
		*(long *)(sound_source_get(next)+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET)=
			*(const long *)(source+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET);
		following= *(const long *)(source+SOUND_SOURCE_DATUM_NEXT_OFFSET);
	}
	if (looping_sound)
	{
		long *first= (long *)(looping_sound+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+track*LOOPING_SOUND_TRACK_SIZE);

		looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1]--;
		if (*first==source_index)
		{
			*first= following;
		}
	}
	controller_index= *(const long *)(source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET);
	if (controller_index!=NONE)
	{
		byte *controller= sound_playback_controller_try_and_get(controller_index);

		if (controller && source[SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET+LOOPING_SOUND_TOLD_COUNTED_OFFSET]==0xFF &&
			*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE)
		{
			controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]=
				(controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]-1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
		}
	}
}

/* 0x0021b940: makes the sound that plays of a track of a looping sound. NONE
if the sound cannot play now (sound_definition_unknown_00126bd0), if no
listener is near enough to hear it, or if there is no room for one more.

The sound is as high as the lower bound of its pitch (a number is taken from
the seed for it, and the same bound is used twice), as loud as the track
says, and says of itself what the looping sound says. It is told of things
through looping_sound_track_callbacks, with the track and its seed and
played recordings, and is alone in its ring. With the flag 5 of the looping
sound's tag it counts in the looping sound's sound playback controller.

With no pitch range given, it is chosen by the pitch, bent by the real at
0x10 of the track between the two bounds of the sound's scale. The recording
is chosen as for any sound of a track, and then the sound goes into the ring
of its track and the looping sound counts one more. `kind` is what the sound
is of the track (2: with the flag 13 of the sound, the bit 0 at 0x61), and
`seed_address` is where the seed is. The name is made up */
long __attribute__((stdcall)) looping_sound_source_new(
	long looping_sound_index,
	long sound_index,
	long pitch_range,
	long track,
	long kind,
	long seed_address)
{
	byte *looping_sound= (byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	const byte *definition= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)&0xFFFF].address;
	long track_index= (short)track;
	const byte *track_definition= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4)+
		track_index*LOOPING_SOUND_DEFINITION_TRACK_SIZE;
	const struct sound_source *said= (const struct sound_source *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET);
	unsigned long *seed= (unsigned long *)(uintptr_t)seed_address;
	const struct sound_definition *sound;
	const byte *track_state;
	unsigned long played;
	long listener_index;
	long source_index;
	long permutation;
	byte *source;
	real lower;
	long last;

	if (!sound_definition_unknown_00126bd0(sound_index))
	{
		return NONE;
	}
	sound= sound_definition_get(sound_index);
	listener_index= sound_source_get_listener(said, sound_get_maximum_distance(said, sound_index), NULL);
	if (listener_index==NONE)
	{
		return NONE;
	}
	source_index= datum_new(sound_sources);
	if (source_index==NONE)
	{
		return NONE;
	}
	source= sound_source_get(source_index);
	track_state= looping_sound+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+track_index*LOOPING_SOUND_TRACK_SIZE;
	source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]= (byte)listener_index;
	*(short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)= NONE;
	*(word *)(source+SOUND_SOURCE_DATUM_FLAGS_OFFSET)= 0;
	lower= (real)sound_definition_get_playback(sound)->pitch_lower_bound;
	*seed= *seed*1664525UL+1013904223UL;
	*(real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET)= (lower-lower)*((real)(*seed>>16)*(1.0f/65535.0f))+lower;
	*(long *)(source+SOUND_SOURCE_DATUM_GAIN_OFFSET)= *(const long *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_GAIN_OFFSET);
	*(unsigned long *)(source+SOUND_SOURCE_DATUM_SEED_OFFSET)= *seed;
	*(long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET)= looping_sound_index;
	memcpy(source+SOUND_SOURCE_DATUM_UNKNOWN18_OFFSET, said, LOOPING_SOUND_DATUM_SOURCE_SIZE);
	source[SOUND_SOURCE_DATUM_KIND_OFFSET]= (byte)kind;
	*(word *)(source+SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET)= *(const word *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN2_OFFSET);
	*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN8C_OFFSET)= *(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET);
	*(const struct sound_source_callbacks **)(source+SOUND_SOURCE_DATUM_CALLBACKS_OFFSET)= &looping_sound_track_callbacks;
	*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= 0;
	*(long *)(source+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= 0;
	*(short *)(source+SOUND_SOURCE_DATUM_UNKNOWNA2_OFFSET)= NONE;
	source[SOUND_SOURCE_DATUM_UNKNOWNA4_OFFSET]= 0xFF;
	*(long *)(source+SOUND_SOURCE_DATUM_CONTROLLER_INDEX_OFFSET)= NONE;
	source[SOUND_SOURCE_DATUM_TRACK_OFFSET]= (byte)track;
	source[SOUND_SOURCE_DATUM_TRACK_OFFSET+1]= 0;
	*(long *)(source+SOUND_SOURCE_DATUM_TRACK_SEED_OFFSET)= *(const long *)(track_state+8);
	*(long *)(source+SOUND_SOURCE_DATUM_TRACK_PLAYED_OFFSET)= *(const long *)(track_state+4);
	*(long *)(source+SOUND_SOURCE_DATUM_NEXT_OFFSET)= source_index;
	*(long *)(source+SOUND_SOURCE_DATUM_PREVIOUS_OFFSET)= source_index;
	if (source[SOUND_SOURCE_DATUM_KIND_OFFSET]==2 && (((const byte *)sound)[1]&0x20))
	{
		source[SOUND_SOURCE_DATUM_TRACK_OFFSET+1]|= 1;
	}
	if (definition[0]&FLAG(5))
	{
		sound_playback_controller_add_count(
			sound_playback_controller_get(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET)), 1);
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET)=
			*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET);
		source[SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET+LOOPING_SOUND_TOLD_COUNTED_OFFSET]= 0xFF;
	}
	else
	{
		*(long *)(source+SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET)= NONE;
	}
	if (pitch_range==NONE)
	{
		const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
		real low= (real)bends[0];

		pitch_range= sound_choose_pitch_range(NONE, sound,
			((real)bends[1]-low)**(const real *)(track_state+LOOPING_SOUND_TRACK_PITCH_OFFSET)+
			*(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET)+low);
	}
	played= looping_sound_track_get_played_bits(track_index, sound, looping_sound, definition, (short)pitch_range,
		(short)kind==2);
	last= sound_unknown_0021cc30(definition, sound, (short)pitch_range);
	permutation= sound_pitch_range_choose_permutation(sound, (short)pitch_range, &played, last, seed, NULL, FALSE);
	looping_sound_track_set_played_bits(track_index, played, looping_sound, sound, definition, (short)pitch_range,
		(char)permutation);
	sound_source_set_sound(source_index, sound_index, (char)pitch_range, (char)permutation, 0);
	sound_source_request_data(source);
	looping_sound_add_source(looping_sound_index, (short)track, source_index);
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1]++;

	return source_index;
}

/* how much of an arc, from one angle to another, is in another */
static inline __attribute__((always_inline)) real sound_arc_overlap(
	const real *arc,
	const real *other)
{
	real end= arc[1]>other[1] ? other[1] : arc[1];
	real start= arc[0]>other[0] ? arc[0] : other[0];
	real overlap= end-start;

	return overlap>0.0f ? overlap : 0.0f;
}

/* 0x0021ee80: the console's sound tells the console what has changed, and
to do all that it was told. First, if unknown_0047005c is set, the handler
of the sound of the places is given unknown_00470060. Then, for each of the
two places that are heard: if how it sounds has changed, its effect is given
it (sound_console_set_reverb, sound_console_place_effects); and if the rest
has, each of its four streams (at 0x2A70) takes its loudness: the half of
the circle that the stream is for (in three arcs, for the angles that go
round) has some of the arc between the first two numbers of the place, and
that part of a half circle goes towards the fourth number by what the third
is short of 1, times how much there is of the place; in hundredths of a
decibel. After that the console is told to do it all (unknown_0037d141),
each voice is given to unknown_002ae820, and unknown_00191550 is called. The
name is made up */
void sound_console_commit(
	void)
{
	static const real arcs[NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS][6]=
	{
		{ -0.785398185f, 2.35619450f, 5.49778748f, 6.28318548f, -6.28318548f, -3.92699099f },
		{ -2.35619450f, 0.785398185f, 3.92699099f, 6.28318548f, -6.28318548f, -5.49778748f },
		{ 0.785398185f, 3.92699099f, -5.49778748f, -2.35619450f, 0.0f, 0.0f },
		{ 2.35619450f, 5.49778748f, -3.92699099f, -0.785398185f, 0.0f, 0.0f }
	};
	byte *console;
	long place;
	long index;

	if (unknown_0047005c)
	{
		struct sound_console_environment_handler *handler;

		console= unknown_0051ebe4;
		handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
		handler->functions->unknown8(handler, unknown_00470060, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET]);
		unknown_0047005c= FALSE;
	}

	console= unknown_0051ebe4;
	for (place= 0; place<NUMBER_OF_SOUND_CONSOLE_PLACES; place++)
	{
		if (console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+place])
		{
			sound_console_set_reverb((const real *)(console+UNKNOWN_0051EBE4_PLACE_ENVIRONMENTS_OFFSET+place*SOUND_ENVIRONMENT_SIZE),
				sound_console_place_effects[place]);
			console= unknown_0051ebe4;
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+place]= FALSE;
		}
		if (console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+2+place])
		{
			real amount= *(const real *)(console+UNKNOWN_0051EBE4_PLACE_AMOUNTS_OFFSET+place*4);
			const real *numbers= (const real *)(console+UNKNOWN_0051EBE4_PLACE_NUMBERS_OFFSET+place*SOUND_CONSOLE_PLACE_NUMBERS_SIZE);

			for (index= 0; index<NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS; index++)
			{
				real first= sound_arc_overlap(numbers, &arcs[index][0]);
				real second= sound_arc_overlap(numbers, &arcs[index][2]);
				real third= sound_arc_overlap(numbers, &arcs[index][4]);
				real gain= (((third+second+first)*0.318309873f-numbers[3])*numbers[2]+numbers[3])*amount;

				unknown_0037b66f(*(void **)(console+UNKNOWN_0051EBE4_PLACE_STREAMS_OFFSET+
					(place*NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS+index)*4), sound_gain_to_millibels(sound_pin_gain(gain)));
				console= unknown_0051ebe4;
			}
			console[UNKNOWN_0051EBE4_UNKNOWN2A08_OFFSET+2+place]= FALSE;
		}
	}
	unknown_0037d141(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET));
	for (index= 0; index<*(const short *)(unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET); index++)
	{
		unknown_002ae820(unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_OFFSET+index*UNKNOWN_0051EBE4_VOICE_SIZE);
	}
	unknown_00191550();
}

/* what the console is told of the filter of a stream, and of each of two
other things of it: which kind, and five numbers */
struct sound_console_effect_description
{
	long kind;
	long values[5];
};

/* a whole number, no less than one and no more than another */
static inline __attribute__((always_inline)) long sound_pin_long(
	long value,
	long minimum,
	long maximum)
{
	if (value<minimum)
	{
		return minimum;
	}

	return value>maximum ? maximum : value;
}

/* 0x002201f0: the stream of a voice of the console's sound takes the effects
of its sound: a filter (unknown_0037b813), two other things (unknown_0037b809,
the first with a 1 and the second with a 0) and the list that
sound_console_apply_effect_list gives the console. The numbers of each come
from four reals each (sound_unknown_00219880), with the seed and the fraction
that the effects start with.

The filter is of one of three kinds, with two frequencies and two loudnesses;
the third kind is only for a voice whose three bits at 3 of its byte at 1 are
0. With `all` not set, only the filter is told, and not even that if the
fraction is the one that the voice had (to a ten thousandth). With it set,
the other two are told too: the first has three numbers (up to 0xFFFF, up to
0x3FF and from -128 to 127) and the second four; and what the effects do not
have is told as nothing. The voice keeps the fraction. The voice is taken in
eax and the effects in ebx. The name is made up */
void __attribute__((stdcall)) sound_console_voice_apply_effects(
	long voice_index,
	const byte *effects,
	boolean all)
{
	struct sound_console_effect_description description;
	boolean filter_told= FALSE;
	boolean first_told= FALSE;
	boolean second_told= FALSE;
	const byte *filter;
	unsigned long seed;
	real fraction;
	byte *voice;
	long field;

	if (!effects)
	{
		return;
	}
	voice= unknown_0051ebe4+UNKNOWN_0051EBE4_VOICE_OFFSET+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE;
	fraction= *(const real *)(effects+SOUND_VOICE_EFFECTS_FRACTION_OFFSET);
	seed= *(const unsigned long *)effects;
	filter= *(const byte *const *)(effects+SOUND_VOICE_EFFECTS_FILTER_OFFSET);
	field= (signed char)(voice[1]<<2)>>5;
	if (!filter || all ||
		!((long double)0.0001f>fabsl((long double)fraction-(long double)*(const real *)(voice+VOICE_FRACTION_OFFSET))))
	{
		if (filter)
		{
			boolean known= FALSE;
			real first_frequency;
			real first_decibels;
			real second_frequency;
			real second_decibels;

			memset(&description, 0, sizeof(description));
			first_frequency= sound_unknown_00219880((const real *)(filter+8), &seed, fraction);
			first_decibels= sound_unknown_00219880((const real *)(filter+0x18), &seed, fraction);
			second_frequency= sound_unknown_00219880((const real *)(filter+0x28), &seed, fraction);
			second_decibels= sound_unknown_00219880((const real *)(filter+0x38), &seed, fraction);
			switch (*(const long *)filter)
			{
			case 0:
				description.kind= 2;
				description.values[0]= sound_pin_long(*(const long *)(filter+4), 0, 7);
				description.values[1]= sound_frequency_to_filter_coefficient(first_frequency);
				description.values[2]= sound_unknown_0021e330(first_decibels);
				description.values[3]= sound_frequency_to_filter_coefficient(second_frequency);
				description.values[4]= sound_unknown_0021e330(second_decibels);
				known= TRUE;
				break;
			case 1:
				description.kind= 1;
				description.values[1]= sound_frequency_to_filter_coefficient(first_frequency);
				description.values[2]= (long)sound_unknown_0021e2d0(first_decibels);
				description.values[3]= sound_frequency_to_filter_coefficient(second_frequency);
				description.values[4]= (long)sound_unknown_0021e2d0(second_decibels);
				known= TRUE;
				break;
			case 2:
				if (!field)
				{
					description.kind= 3;
					description.values[0]= sound_pin_long(*(const long *)(filter+4), 0, 7);
					description.values[1]= sound_frequency_to_filter_coefficient(first_frequency);
					description.values[2]= (long)sound_unknown_0021e2d0(first_decibels);
					description.values[3]= sound_frequency_to_filter_coefficient(second_frequency);
					description.values[4]= sound_unknown_0021e330(second_decibels);
					known= TRUE;
				}
				break;
			}
			if (known)
			{
				unknown_0037b813(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
				filter_told= TRUE;
			}
		}
		if (all)
		{
			const byte *first= *(const byte *const *)(effects+SOUND_VOICE_EFFECTS_FILTER_OFFSET+4);
			const byte *second;

			if (first)
			{
				real rate;
				real depth;
				real amount;

				memset(&description, 0, sizeof(description));
				description.kind= 1;
				rate= sound_unknown_00219880((const real *)first, &seed, fraction);
				depth= sound_unknown_00219880((const real *)(first+0x10), &seed, fraction);
				amount= sound_unknown_00219880((const real *)(first+0x20), &seed, fraction);
				description.values[0]= sound_pin_long((long)(rate*1500.0f), 0, 0xFFFF);
				description.values[1]= sound_pin_long((long)(depth*43.6906662f), 0, 0x3FF);
				description.values[2]= sound_pin_long((long)(amount*128.0f), -128, 127);
				unknown_0037b809(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
				first_told= TRUE;
			}
			second= *(const byte *const *)(effects+SOUND_VOICE_EFFECTS_FILTER_OFFSET+8);
			if (second)
			{
				real rate;
				real depth;
				real amount;
				real other;

				memset(&description, 0, sizeof(description));
				rate= sound_unknown_00219880((const real *)second, &seed, fraction);
				depth= sound_unknown_00219880((const real *)(second+0x10), &seed, fraction);
				amount= sound_unknown_00219880((const real *)(second+0x20), &seed, fraction);
				other= sound_unknown_00219880((const real *)(second+0x30), &seed, fraction);
				description.values[0]= sound_pin_long((long)(rate*1500.0f), 0, 0xFFFF);
				description.values[1]= sound_pin_long((long)(depth*43.6906662f), 0, 0x3FF);
				description.values[3]= sound_pin_long((long)(amount*16.0f), -128, 127);
				description.values[4]= sound_pin_long((long)(other*16.0f), -128, 128);
				unknown_0037b809(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
				second_told= TRUE;
			}
			if (!filter_told)
			{
				memset(&description, 0, sizeof(description));
				unknown_0037b813(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
			}
			if (!first_told)
			{
				memset(&description, 0, sizeof(description));
				description.kind= 1;
				unknown_0037b809(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
			}
			if (!second_told)
			{
				memset(&description, 0, sizeof(description));
				unknown_0037b809(*(void **)(voice+VOICE_STREAM_OFFSET), &description);
			}
		}
	}
	*(real *)(voice+VOICE_FRACTION_OFFSET)= fraction;
	sound_console_apply_effect_list(*(const long *)(effects+SOUND_VOICE_EFFECTS_LIST_SIZE_OFFSET),
		effects+SOUND_VOICE_EFFECTS_LIST_OFFSET);
}

/* what the handler of the sound of the places is given of where a voice is
heard: which of its reals are set (bit 0: the four at 4; bit 1: the one at
0x14; bit 5: the first two, with the loudness of all the outputs together),
and four more flags of the voice */
struct sound_console_voice_places
{
	unsigned long flags;
	real unknown4;
	real unknown8;
	real unknownC;
	real unknown10;
	real unknown14;
};

/* a loudness in decibels, from -64 to 0, as the hundredths of a decibel
that the console takes */
static inline __attribute__((always_inline)) long sound_decibels_to_millibels(
	real decibels)
{
	return (long)(sound_interpolation_fraction(SOUND_MINIMUM_DECIBELS, 0.0f, decibels, TRUE)*6400.0f-6400.0f);
}

/* whether two reals are within a tenth of each other, on the x87 */
static inline __attribute__((always_inline)) boolean sound_reals_near(
	real a,
	real b)
{
	return (long double)0.1f>fabsl((long double)a-(long double)b);
}

/* 0x0021fa80: a voice of the console's sound takes how its sound is to be
heard: how loud, where, and how high; and then its effects
(sound_console_voice_apply_effects). Each thing is told to the console only
if the voice is new (it has no flag 0 at 3, or the console's sound has
nothing at its start) or if it has changed by a tenth or more (the pitch,
by 1.5).

The loudness goes to the stream of the voice. A voice with a slot of the
console's sound (one whose number is not under the short at 8) has the
loudness in the output of the slot (once, or twice for a voice whose three
bits are 1) and the stream at its loudest.

Where it is heard is for the handler of the sound of the places, which
fills the outputs: for a voice with a slot, those of what plays the sound of
the slot at 0x38, which starts or stops with them, and the loudness of the
one at 0x34; for the rest, those of the stream. With the flag 5 of what is
asked, the four loudnesses at 0x38 go together as one.

The pitch is that of what is asked, in the units of the console (4096 for
each octave), from a base for each of three kinds of samples; it is not
told when `keep_pitch` is set. What is asked is taken in edi. The name is
made up */
void __attribute__((stdcall)) sound_console_voice_set_parameters(
	const byte *parameters,
	long voice_index,
	const byte *effects,
	boolean keep_pitch)
{
	byte *console= unknown_0051ebe4;
	byte *voice= console+UNKNOWN_0051EBE4_VOICE_OFFSET+voice_index*UNKNOWN_0051EBE4_VOICE_SIZE;
	boolean first= !(voice[VOICE_FLAGS3_OFFSET]&1) || !console[0];
	real decibels= *(const real *)(parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)>SOUND_MINIMUM_DECIBELS ?
		*(const real *)(parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET) : SOUND_MINIMUM_DECIBELS;
	long field= (signed char)(voice[1]<<2)>>5;
	boolean spread= parameters[SOUND_VOICE_PARAMETERS_FLAGS_OFFSET]&1;
	boolean places_changed= TRUE;
	long slot_index;

	if (first || !sound_reals_near(decibels, *(const real *)(voice+VOICE_DECIBELS_OFFSET)))
	{
		console= unknown_0051ebe4;
		slot_index= *(const short *)(parameters+SOUND_VOICE_PARAMETERS_SLOT_OFFSET);
		if (slot_index>=*(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_OFFSET))
		{
			long old[SOUND_CONSOLE_STREAM_MIXBINS_SIZE/4];
			long new[SOUND_MIXBINS_COUNT_INDEX+2];
			const byte *slot;

			unknown_0037b83f(*(void **)(voice+VOICE_STREAM_OFFSET), old);
			slot= unknown_0051ebe4+UNKNOWN_0051EBE4_SLOTS_OFFSET+
				*(const short *)(parameters+SOUND_VOICE_PARAMETERS_SLOT_OFFSET)*UNKNOWN_0051EBE4_SLOT_SIZE;
			new[SOUND_MIXBINS_COUNT_INDEX]= 0;
			new[SOUND_MIXBINS_COUNT_INDEX+1]= (long)(uintptr_t)new;
			if (field==1)
			{
				sound_mixbins_add(new, *(const long *)slot, decibels);
			}
			if (field==0 || field==1)
			{
				sound_mixbins_add(new, *(const long *)slot, decibels);
			}
			sound_console_stream_set_mixbins(old, new, *(void **)(voice+VOICE_STREAM_OFFSET));
			unknown_0037b7ff(*(void **)(voice+VOICE_STREAM_OFFSET), sound_decibels_to_millibels(0.0f));
		}
		else
		{
			unknown_0037b7ff(*(void **)(voice+VOICE_STREAM_OFFSET), sound_decibels_to_millibels(decibels));
		}
		*(real *)(voice+VOICE_DECIBELS_OFFSET)= decibels;
	}

	slot_index= *(const short *)(parameters+SOUND_VOICE_PARAMETERS_SLOT_OFFSET);
	if (!first && spread==((voice[VOICE_FLAGS3_OFFSET]>>5)&1))
	{
		boolean same= spread ||
			(sound_reals_near(*(const real *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN18_OFFSET),
				*(const real *)(voice+VOICE_UNKNOWN10_OFFSET)) &&
			sound_reals_near(*(const real *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET),
				*(const real *)(voice+VOICE_UNKNOWN18_OFFSET)) &&
			sound_reals_near(*(const real *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN1C_OFFSET), 0.0f) &&
			sound_reals_near(*(const real *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN20_OFFSET), 0.0f));

		if (same && (slot_index==NONE || sound_reals_near(*(const real *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN28_OFFSET),
			*(const real *)(voice+VOICE_UNKNOWN14_OFFSET))))
		{
			places_changed= FALSE;
		}
	}

	if (places_changed)
	{
		unsigned long flags= *(const unsigned long *)(parameters+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET);
		long mixbins[SOUND_MIXBINS_COUNT_INDEX+2];
		struct sound_console_voice_places places;

		places.flags= ((flags>>1)&1 ? 4 : 0)|((flags>>3)&1 ? 8 : 0)|((flags>>4)&1 ? 0x10 : 0)|((flags>>6)&1 ? 0x40 : 0);
		voice[VOICE_FLAGS3_OFFSET]= spread ? voice[VOICE_FLAGS3_OFFSET]|0x20 : voice[VOICE_FLAGS3_OFFSET]&~0x20;
		if ((*(const unsigned long *)(parameters+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET)>>5)&1)
		{
			long double sum= 0.0L;
			real values[4];
			long index;

			memcpy(values, parameters+SOUND_VOICE_PARAMETERS_UNKNOWN38_OFFSET, sizeof(values));
			for (index= 0; index<4; index++)
			{
				long double gain= sound_decibels_to_gain(values[index]);

				sum+= gain*gain;
			}
			places.unknown4= sound_gain_to_decibels((real)sqrtl(sum));
			*(long *)&places.unknown8= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET);
			places.flags|= 0x20;
		}
		else
		{
			if (!(parameters[SOUND_VOICE_PARAMETERS_FLAGS_OFFSET]&1))
			{
				*(long *)&places.unknown8= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET);
				*(long *)&places.unknown4= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN18_OFFSET);
				*(long *)&places.unknownC= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN1C_OFFSET);
				*(long *)&places.unknown10= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN20_OFFSET);
				*(long *)(voice+VOICE_UNKNOWN10_OFFSET)= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN18_OFFSET);
				places.flags|= 1;
				*(long *)(voice+VOICE_UNKNOWN18_OFFSET)= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN14_OFFSET);
			}
			if (slot_index!=NONE)
			{
				*(long *)&places.unknown14= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN28_OFFSET);
				places.flags|= 2;
				*(long *)(voice+VOICE_UNKNOWN14_OFFSET)= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN28_OFFSET);
			}
		}

		console= unknown_0051ebe4;
		if (slot_index>=*(const short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_OFFSET) && !(places.flags&0x20))
		{
			struct sound_console_slot *slot= (struct sound_console_slot *)(console+UNKNOWN_0051EBE4_SLOTS_OFFSET+
				slot_index*UNKNOWN_0051EBE4_SLOT_SIZE);
			long first_old[SOUND_CONSOLE_STREAM_MIXBINS_SIZE/4];
			long second_old[SOUND_CONSOLE_STREAM_MIXBINS_SIZE/4];
			long status;

			unknown_0037b7e3(slot->stream, first_old);
			unknown_0037b7e3(slot->unknown38, second_old);
			if (places.flags&2)
			{
				unknown_0037b66f(slot->stream, sound_decibels_to_millibels(places.unknown14));
			}
			else
			{
				unknown_0037b66f(slot->stream, -10000);
			}
			if (places.flags&1)
			{
				struct sound_console_environment_handler *handler;

				console= unknown_0051ebe4;
				handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
				mixbins[SOUND_MIXBINS_COUNT_INDEX]= 0;
				mixbins[SOUND_MIXBINS_COUNT_INDEX+1]= (long)(uintptr_t)mixbins;
				handler->functions->unknownC(handler, &places, 0, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET], mixbins);
				sound_console_slot_stream_set_mixbins(second_old, mixbins, slot->unknown38);
				unknown_0037b75b(slot->unknown38, &status);
				if (!(status&1))
				{
					unknown_0037b6df(slot->unknown38, 0, 0, 0);
				}
			}
			else
			{
				unknown_0037b75b(slot->unknown38, &status);
				if (status&1)
				{
					unknown_0037b71b(slot->unknown38, 0, 0, 0);
				}
			}
		}
		else
		{
			struct sound_console_environment_handler *handler;

			mixbins[SOUND_MIXBINS_COUNT_INDEX]= 0;
			mixbins[SOUND_MIXBINS_COUNT_INDEX+1]= (long)(uintptr_t)mixbins;
			if (places.flags&1)
			{
				handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
				handler->functions->unknownC(handler, &places, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET], field, mixbins);
				console= unknown_0051ebe4;
			}
			if (places.flags&2)
			{
				handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
				handler->functions->unknown10(handler, 0x1F, field, places.unknown14, mixbins);
				console= unknown_0051ebe4;
			}
			if (places.flags&0x20)
			{
				handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
				handler->functions->unknown18(handler, &places, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET], field, mixbins);
				console= unknown_0051ebe4;
			}
			handler= *(struct sound_console_environment_handler **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB4_OFFSET);
			handler->functions->unknown14(handler, parameters, console[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET], field, mixbins);
			unknown_0037c70a(*(void **)(voice+VOICE_STREAM_OFFSET),
				(const struct sound_console_mixbins *)&mixbins[SOUND_MIXBINS_COUNT_INDEX]);
		}
	}

	if (!keep_pitch)
	{
		long kind= *(const long *)(parameters+SOUND_VOICE_PARAMETERS_UNKNOWN4_OFFSET);
		real pitch= *(const real *)(parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET)*3.41333342f+
			(real)(kind==0 ? -4597 : kind==1 ? -501 : -2396);
		long difference;
		long value;

		if (-32767.0f>pitch)
		{
			pitch= -32767.0f;
		}
		else if (pitch>8191.0f)
		{
			pitch= 8191.0f;
		}
		value= (long)pitch;
		difference= value-*(const long *)(voice+VOICE_PITCH_OFFSET);
		if (first || !((real)(difference<0 ? -difference : difference)<1.5f))
		{
			unknown_0037b804(*(void **)(voice+VOICE_STREAM_OFFSET), value);
			*(long *)(voice+VOICE_PITCH_OFFSET)= value;
		}
	}
	sound_console_voice_apply_effects(voice_index, effects, FALSE);
}

/* whether a track of a looping sound is to go on to another state: only the
low byte of what looping_sound_unknown_0021a050 gives is looked at */
static inline __attribute__((always_inline)) boolean looping_sound_track_goes_on(
	const byte *track_state,
	const byte *track_definition)
{
	return (byte)looping_sound_unknown_0021a050(track_state, track_definition)!=0;
}

/* the recording that a sound that plays is at */
static inline __attribute__((always_inline)) const struct sound_permutation *sound_source_permutation(
	const byte *source)
{
	const struct sound_cache_file_gestalt *gestalt= global_sound_cache_file_gestalt;
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	const struct sound_pitch_range *pitch_range= (const struct sound_pitch_range *)(uintptr_t)gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]);

	return (const struct sound_permutation *)(uintptr_t)gestalt->permutations.address+
		(pitch_range->first_permutation_index+(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
}

/* 0x0021c400: what is done each time with the sound of a channel that has
a voice of the console's sound, for a sound of a track of a looping sound
(the kinds 1 to 4). As sound_channel_update_voice: the voice is given what
sound_channel_build_voice_parameters fills, with the pitch of the sound
(bent by the real of the track, less that of the recordings as they were
made, and with the Doppler effect for a sound with a sound voice) and how
loud it is, plus the gain of the recording. A sound with no flag 9 starts.

Any other goes on. Of the kinds 1 to 3, when no piece waits (flag 3), the
next piece is chosen: the next of its recording, or at its end another
recording (with the bits of those played, of the track for its first sound
and of the sound for the rest), unless the looping sound's tag has the flag
1, which ends the sound (kind 4). The first sound of a track that the track
says is to go on to another state (looping_sound_unknown_0021a050) does so
at the end of its recording, or before it if the track lets it
(looping_sound_unknown_0021a080) and the piece has its top bit:
looping_sound_source_next_state, and if that lets the sound go there is
nothing more to do. A sound with nothing more to play is marked as at its
end (flag 6), and its voice too. A piece that waits is queued when its
samples are there, unless the voice is at 3. The first sound of a track
keeps the seed and the played recordings of the track. The name is made
up */
void __attribute__((stdcall)) looping_sound_channel_update_voice(
	short channel_index,
	real decibels,
	const byte *parameters)
{
	struct sound_channel *channel= sound_channels+channel_index;
	long source_index= channel->unknown0;
	byte *source= sound_source_get(source_index);
	const struct sound_definition *sound= sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET));
	const struct sound_pitch_range *pitch_range=
		(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
		(sound->first_pitch_range_index+(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]);
	byte *looping_sound= (byte *)looping_sounds->data+
		(*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET)&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	byte *track_state= looping_sound+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+
		(char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET]*LOOPING_SOUND_TRACK_SIZE;
	const byte *definition= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)&0xFFFF].address;
	const byte *track_definition= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4)+
		(char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET]*LOOPING_SOUND_DEFINITION_TRACK_SIZE;
	byte *controller= sound_playback_controller_get(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET));
	const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
	real scale= *(const real *)(source+SOUND_SOURCE_DATUM_SCALE_OFFSET);
	boolean first= *(const long *)track_state==source_index;
	real pitch= ((real)bends[1]-(real)bends[0])**(const real *)(track_state+LOOPING_SOUND_TRACK_PITCH_OFFSET)+
		*(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET)+(real)bends[0];
	byte voice_parameters[SOUND_VOICE_PARAMETERS_SIZE];
	const struct sound_permutation *permutation;
	short voice_state;
	word flags;

	memset(voice_parameters, 0, sizeof(voice_parameters));
	*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)= sound_definition_decibels(sound,
		*(const real *)(source+SOUND_SOURCE_DATUM_DECIBELS_OFFSET)+(*(const real *)(source+SOUND_SOURCE_DATUM_GAIN_OFFSET)+decibels),
		scale);
	*(short *)(voice_parameters+SOUND_VOICE_PARAMETERS_UNKNOWN34_OFFSET)=
		*(const short *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_UNKNOWN3C_OFFSET);
	if (*(const short *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_UNKNOWN3C_OFFSET))
	{
		*(unsigned long *)(voice_parameters+SOUND_VOICE_PARAMETERS_FLAGS_OFFSET)|= 1;
	}
	*(long *)(voice_parameters+SOUND_VOICE_START_CONTROLLER_INDEX_OFFSET)= definition[0]&FLAG(5) ?
		*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET) : NONE;
	*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET)= pitch-(real)sound_pitch_range_natural_pitch(pitch_range);
	if (source[SOUND_SOURCE_DATUM_UNKNOWN6_OFFSET]&FLAG(_sound_source_datum_voice_bit))
	{
		const struct sound_class *sound_class= sound_class_get(sound->sound_class);
		long controller_index= *(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET);
		real doppler;

		if (controller_index!=NONE)
		{
			const byte *own= (const byte *)unknown_0051ebd8->data+(controller_index&0xFFFF)*UNKNOWN_0051EBD8_DATUM_SIZE;

			doppler= *(const long *)(own+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE ?
				*(const real *)(own+UNKNOWN_0051EBD8_DATUM_UNKNOWNC_OFFSET+(char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET]*4) :
				0.0f;
		}
		else
		{
			doppler= (real)sound_listener_doppler_pitch((char)source[SOUND_SOURCE_DATUM_LISTENER_OFFSET],
				(const real_point3d *)(source+SOUND_SOURCE_DATUM_POSITION_OFFSET));
		}
		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET)=
			*(const real *)((const byte *)sound_class+SOUND_CLASS_UNKNOWN48_OFFSET)*
			*(const real *)(parameters+SOUND_CHANNEL_PARAMETERS_UNKNOWN14_OFFSET)*doppler +
			*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_PITCH_OFFSET);
	}
	sound_channel_build_voice_parameters(channel_index, parameters, voice_parameters, source);

	flags= *(const word *)(source+SOUND_SOURCE_DATUM_FLAGS_OFFSET);
	if (!((flags>>_sound_source_datum_unknown9_bit)&1))
	{
		permutation= (const struct sound_permutation *)(uintptr_t)global_sound_cache_file_gestalt->permutations.address+
			(pitch_range->first_permutation_index+(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)=
			(real)(char)permutation->unknown0[SOUND_PERMUTATION_GAIN_OFFSET]+
			*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET);
		sound_console_voice_start(voice_parameters, voice_parameters+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET, channel->unknownC);
		sound_console_stream_set_function(voice_parameters+SOUND_VOICE_PARAMETERS_ENVIRONMENT_OFFSET);
		sound_channel_queue_chunk(channel_index, permutation, *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET));
		if (definition[0]&FLAG(5))
		{
			byte count= 0xFF;

			if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)!=NONE)
			{
				count= controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
				controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]=
					(byte)(controller[UNKNOWN_0051EBD8_DATUM_COUNT_OFFSET]-1)&UNKNOWN_0051EBD8_DATUM_COUNT_MASK;
			}
			source[SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET+LOOPING_SOUND_TOLD_COUNTED_OFFSET]= count;
		}
		source[SOUND_SOURCE_DATUM_FLAGS_OFFSET+1]|= FLAG(_sound_source_datum_unknown9_bit-8);
	}
	else
	{
		byte kind= source[SOUND_SOURCE_DATUM_KIND_OFFSET];

		if (kind!=2 && kind!=3 && kind!=1)
		{
			source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown6_bit);
			sound_channel_set_voice_flag3(channel_index);
		}
		else
		{
			voice_state= sound_voice_state(sound_channel_voice(sound_channels+
				*(const short *)(source+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)));
			if (!((flags>>_sound_source_datum_unknown3_bit)&1) &&
				!(voice_state==3 && !looping_sound_track_goes_on(track_state, track_definition)))
			{
				unsigned long *seed= &global_random_seed_reference[1];
				const struct sound_permutation *playing;
				boolean chosen= FALSE;
				boolean ended= FALSE;
				boolean at_end;
				long next_chunk;
				long recording;

				if (definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS)
				{
					seed= first ? (unsigned long *)(track_state+8) : (unsigned long *)(source+SOUND_SOURCE_DATUM_TRACK_SEED_OFFSET);
				}
				recording= (short)(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET];
				playing= sound_source_permutation(source);
				next_chunk= playing && *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)<playing->chunk_count-1 ?
					*(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET)+1 : NONE;
				at_end= (short)next_chunk==NONE;
				if (!(source[SOUND_SOURCE_DATUM_TRACK_OFFSET+1]&1) && first &&
					looping_sound_track_goes_on(track_state, track_definition))
				{
					if (at_end || (looping_sound_unknown_0021a080(track_state) &&
						(*(const unsigned long *)((const byte *)(uintptr_t)global_sound_cache_file_gestalt->chunks.address+
						(playing->first_chunk_index+*(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET))*
						(long)sizeof(struct sound_chunk)+4)>>31)))
					{
						if (looping_sound_source_next_state(looping_sound, track_state, channel->unknown0, seed))
						{
							return;
						}
						pitch_range= (const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
							(sound_definition_get(*(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET))->first_pitch_range_index+
							(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]);
						source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown3_bit);
						chosen= TRUE;
					}
				}
				else if (at_end)
				{
					if (definition[0]&FLAG(1))
					{
						source[SOUND_SOURCE_DATUM_KIND_OFFSET]= 4;
						if (first)
						{
							looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]|= 8;
						}
						ended= TRUE;
					}
					else
					{
						unsigned long played= first ?
							looping_sound_track_get_played_bits((char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET], sound, looping_sound,
								definition, (short)(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], FALSE) :
							*(const unsigned long *)(source+SOUND_SOURCE_DATUM_TRACK_PLAYED_OFFSET);

						recording= sound_pitch_range_choose_permutation(sound, (short)(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET],
							&played, (short)(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET], seed, NULL, FALSE);
						if (first)
						{
							looping_sound_track_set_played_bits((char)source[SOUND_SOURCE_DATUM_TRACK_OFFSET], played, looping_sound,
								sound, definition, (short)(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], (char)recording);
						}
						else
						{
							*(unsigned long *)(source+SOUND_SOURCE_DATUM_TRACK_PLAYED_OFFSET)= played;
						}
						next_chunk= 0;
					}
				}
				if (!chosen)
				{
					if (!ended && (short)recording!=NONE)
					{
						sound_source_set_sound(channel->unknown0, *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET),
							(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET], (char)recording, (short)next_chunk);
						source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown3_bit);
					}
					else
					{
						source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= FLAG(_sound_source_datum_unknown6_bit);
						sound_channel_set_voice_flag3(channel_index);
					}
				}
			}
			if ((source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]>>_sound_source_datum_unknown3_bit)&1)
			{
				permutation= (const struct sound_permutation *)(uintptr_t)global_sound_cache_file_gestalt->permutations.address+
					(pitch_range->first_permutation_index+(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
				if (sound_source_request_data(sound_source_get(channel->unknown0)) && voice_state!=3)
				{
					sound_channel_queue_chunk(channel_index, permutation, *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET));
					source[SOUND_SOURCE_DATUM_FLAGS_OFFSET]&= ~FLAG(_sound_source_datum_unknown3_bit);
					if (first && !looping_sound_track_goes_on(track_state, track_definition) &&
						(short)sound_permutation_next_chunk(permutation, *(const short *)(source+SOUND_SOURCE_DATUM_CHUNK_OFFSET))==NONE)
					{
						if (source[SOUND_SOURCE_DATUM_KIND_OFFSET]==1)
						{
							source[SOUND_SOURCE_DATUM_KIND_OFFSET]= 2;
						}
						else if (source[SOUND_SOURCE_DATUM_KIND_OFFSET]==3)
						{
							source[SOUND_SOURCE_DATUM_KIND_OFFSET]= 4;
						}
					}
				}
			}
		}
		permutation= (const struct sound_permutation *)(uintptr_t)global_sound_cache_file_gestalt->permutations.address+
			(pitch_range->first_permutation_index+(char)source[SOUND_SOURCE_DATUM_PERMUTATION_OFFSET]);
		*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET)=
			(real)(char)permutation->unknown0[SOUND_PERMUTATION_GAIN_OFFSET]+
			*(real *)(voice_parameters+SOUND_VOICE_PARAMETERS_DECIBELS_OFFSET);
		sound_console_voice_set_parameters(voice_parameters, sound_channels[channel_index].unknownC,
			voice_parameters+SOUND_VOICE_PARAMETERS_EFFECTS_OFFSET, FALSE);
	}
	if (first)
	{
		*(long *)(source+SOUND_SOURCE_DATUM_TRACK_SEED_OFFSET)= *(const long *)(track_state+8);
		*(long *)(source+SOUND_SOURCE_DATUM_TRACK_PLAYED_OFFSET)= *(const long *)(track_state+4);
	}
}

/* 0x0021b0e0: what is done each time with each looping sound. With
`update_sources` set, one that was not kept this time (its byte at 0x53 is
not that of the game's sound) is let go, unless it has the flag 1 and
sounds that play.

For each track of the rest, the count of its sounds with a channel starts
again and, unless the looping sound is at 2, its real goes towards that of
the looping sound: as fast as the playback of the sound of its first sound
says (in each second), or at once if that says nothing. Then, with
`update_sources`, the pitch range for the pitch that results is seen to
(sound_choose_pitch_range, with the seed of the sound playback controller,
which takes a new number if the controller has nothing at 4). A sound with
the flag 13 plays all its pitch ranges at once: each one that has the pitch
has a sound of the kind 2 in the ring of the track (made if there is none),
as loud as the track says plus what sound_pitch_range_fade_decibels says,
and those that do not have it are let go (reason 8). Any other sound goes
to the sound of the new pitch range, found or made, in three quarters of a
second (sound_sources_cross_fade), and that one is the first of the track
from then on.

Last, unless the looping sound is at 2 or has the flag 2, each detail of
its tag whose time has come plays (looping_sound_detail_request,
sound_source_new) and takes its next time: in some seconds at random
between its two, in thousandths. A detail with the flag 0 does not play
when the looping sound has the flag 0, and one with the flag 1 only then.
The name is made up */
void __attribute__((stdcall)) looping_sounds_update(
	boolean update_sources)
{
	long looping_sound_index;

	for (looping_sound_index= data_next_index(looping_sounds, NONE); looping_sound_index!=NONE;
		looping_sound_index= data_next_index(looping_sounds, looping_sound_index))
	{
		byte *looping_sound= (byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
		const byte *definition;
		long track;
		long index;

		if (update_sources && looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+3]!=game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET] &&
			(!(looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&2) || !looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1]))
		{
			looping_sound_delete(looping_sound_index);
			continue;
		}
		definition= (const byte *)(uintptr_t)
			global_tag_instances[*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)&0xFFFF].address;
		for (track= 0; track<*(const long *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET); track++)
		{
			byte *track_state= looping_sound+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+track*LOOPING_SOUND_TRACK_SIZE;
			const byte *track_definition= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4)+
				track*LOOPING_SOUND_DEFINITION_TRACK_SIZE;
			real *track_scale= (real *)(track_state+LOOPING_SOUND_TRACK_PITCH_OFFSET);
			const struct sound_definition *sound;
			unsigned long seed;
			long sound_index;
			byte *controller;
			byte *source;
			short current;
			short chosen;
			real pitch;

			track_state[LOOPING_SOUND_TRACK_CHANNELS_OFFSET]= 0;
			if (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+2]==2)
			{
				continue;
			}
			if (*(const long *)track_state!=NONE)
			{
				real rate= *(const real *)(sound_definition_get_playback(sound_definition_get(
					*(const long *)(sound_source_get(*(const long *)track_state)+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)))->unknown8+4);

				if ((long double)0.0001f>fabsl((long double)rate))
				{
					*(long *)track_scale= *(const long *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+4);
				}
				else
				{
					real step= *(const real *)(game_sound_globals+GAME_SOUND_GLOBALS_SECONDS_OFFSET)*rate;
					real difference= *(const real *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+4)-*track_scale;
					real negative= 0.0f-step;

					if (negative>difference)
					{
						*track_scale= negative+*track_scale;
					}
					else
					{
						*track_scale= (difference>step ? step : difference)+*track_scale;
					}
				}
			}
			if (!update_sources || *(const long *)track_state==NONE)
			{
				continue;
			}

			source= sound_source_get(*(const long *)track_state);
			sound_index= *(const long *)(source+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET);
			sound= sound_definition_get(sound_index);
			{
				const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
				real low= (real)bends[0];

				controller= sound_playback_controller_get(*(const long *)(looping_sound+LOOPING_SOUND_DATUM_CONTROLLER_INDEX_OFFSET));
				pitch= ((real)bends[1]-low)**track_scale+*(const real *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET)+low;
			}
			if (*(const long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN4_OFFSET)==NONE)
			{
				unsigned long *controller_seed= (unsigned long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET);

				*controller_seed= *controller_seed*1664525UL+1013904223UL;
			}
			current= (short)(char)source[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET];
			seed= *(const unsigned long *)(controller+UNKNOWN_0051EBD8_DATUM_UNKNOWN8_OFFSET);
			chosen= sound_choose_pitch_range(current, sound, pitch);
			if (((const byte *)sound)[1]&0x20)
			{
				long first_source_index= *(const long *)track_state;
				long by_range[MAXIMUM_SOUND_PITCH_RANGES];
				real decibels[MAXIMUM_SOUND_PITCH_RANGES];
				unsigned long in_range= 0;
				long ring_index= first_source_index;
				long count;
				long range;

				for (range= 0; range<MAXIMUM_SOUND_PITCH_RANGES; range++)
				{
					by_range[range]= NONE;
				}
				for (range= 0; range<MAXIMUM_SOUND_PITCH_RANGES; range++)
				{
					decibels[range]= SOUND_MINIMUM_DECIBELS;
				}
				for (count= 0; ; count++)
				{
					const byte *other;

					if (count>0 && ring_index!=NONE)
					{
						ring_index= *(const long *)(sound_source_get(ring_index)+SOUND_SOURCE_DATUM_NEXT_OFFSET);
						if (ring_index==first_source_index)
						{
							ring_index= NONE;
						}
					}
					if (ring_index==NONE)
					{
						break;
					}
					other= sound_source_get(ring_index);
					if (*(const long *)(other+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)==sound_index &&
						other[SOUND_SOURCE_DATUM_KIND_OFFSET]==2)
					{
						by_range[(char)other[SOUND_SOURCE_DATUM_PITCH_RANGE_OFFSET]]= ring_index;
					}
				}
				for (range= 0; range<(char)sound->pitch_range_count; range++)
				{
					const struct sound_pitch_range *pitch_range=
						(const struct sound_pitch_range *)(uintptr_t)global_sound_cache_file_gestalt->pitch_ranges.address+
						(sound->first_pitch_range_index+range);
					const short *bounds= (const short *)((const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown8[2].address+
						*(const short *)(pitch_range->unknown0+SOUND_PITCH_RANGE_PARAMETERS_INDEX_OFFSET)*SOUND_PITCH_RANGE_PARAMETERS_SIZE+2);
					real pinned;

					decibels[range]= sound_pitch_range_fade_decibels((const byte *)sound, pitch_range, pitch);
					if ((real)bounds[0]>pitch)
					{
						pinned= (real)bounds[0];
					}
					else
					{
						pinned= pitch>(real)bounds[1] ? (real)bounds[1] : pitch;
					}
					if (pinned==pitch)
					{
						in_range|= 1UL<<(range&31);
					}
				}
				for (range= 0; range<(char)sound->pitch_range_count; range++)
				{
					boolean has= (in_range&(1UL<<(range&31)))!=0;

					if (!has && by_range[range]!=NONE)
					{
						sound_source_delete(by_range[range], _sound_source_deleted_out_of_pitch_range);
						by_range[range]= NONE;
					}
					if (has && by_range[range]==NONE)
					{
						long new_source_index= looping_sound_source_new(looping_sound_index, sound_index, range, track, 2,
							(long)(uintptr_t)&seed);

						if (new_source_index!=NONE)
						{
							if (chosen==range)
							{
								*(long *)track_state= new_source_index;
							}
							*(real *)(sound_source_get(new_source_index)+SOUND_SOURCE_DATUM_GAIN_OFFSET)= SOUND_MINIMUM_DECIBELS;
							by_range[range]= new_source_index;
						}
					}
					if (by_range[range]!=NONE)
					{
						byte *other= sound_source_get(by_range[range]);

						*(real *)(other+SOUND_SOURCE_DATUM_GAIN_OFFSET)=
							*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_GAIN_OFFSET)+decibels[range];
						other[SOUND_SOURCE_DATUM_UNKNOWN5C_OFFSET+7]= 0xFF;
					}
				}
			}
			else if (chosen!=current)
			{
				long new_source_index= looping_sound_find_source(*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET),
					(short)track, sound_index, chosen);

				if (new_source_index==NONE)
				{
					new_source_index= looping_sound_source_new(*(const long *)(source+SOUND_SOURCE_DATUM_CALLBACK_DATA_OFFSET),
						sound_index, chosen, track, 2, (long)(uintptr_t)&seed);
				}
				if (new_source_index!=NONE)
				{
					*(long *)(sound_source_get(new_source_index)+SOUND_SOURCE_DATUM_PITCH_OFFSET)=
						*(const long *)(source+SOUND_SOURCE_DATUM_PITCH_OFFSET);
					sound_sources_cross_fade(new_source_index, *(const long *)track_state, 1, 0.75f);
					*(long *)track_state= new_source_index;
				}
			}
		}

		if (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+2]==2 || (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&4))
		{
			continue;
		}
		for (index= 0; index<*(const long *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET); index++)
		{
			const byte *detail= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_DETAILS_OFFSET+4)+
				index*LOOPING_SOUND_DETAIL_SIZE;
			long *time= (long *)(looping_sound+LOOPING_SOUND_DATUM_DETAIL_TIMES_OFFSET)+index;
			long request[SOUND_REQUEST_SIZE/4];
			unsigned long flags;
			long failure;

			if (*(const long *)(detail+LOOPING_SOUND_DETAIL_SOUND_INDEX_OFFSET)==NONE ||
				*time>=*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET))
			{
				continue;
			}
			flags= *(const unsigned long *)(detail+LOOPING_SOUND_DETAIL_FLAGS_OFFSET);
			if (((flags&1) && (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&1)) ||
				((flags&2) && !(looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&1)))
			{
				continue;
			}
			*(word *)((byte *)request+SOUND_REQUEST_SOURCE_OFFSET)= 0;
			if (!looping_sound_detail_request(detail, looping_sound_index, (byte *)request))
			{
				continue;
			}
			if (sound_source_new(*(const long *)(detail+LOOPING_SOUND_DETAIL_SOUND_INDEX_OFFSET), &failure, (byte *)request)==NONE &&
				failure!=1)
			{
				continue;
			}
			*time= (long)(long long)(sound_random_between((const real *)(detail+LOOPING_SOUND_DETAIL_PERIOD_BOUNDS_OFFSET))*
				(long double)1000.0f+(long double)*(const long *)(game_sound_globals+GAME_SOUND_GLOBALS_UNKNOWN84_OFFSET));
		}
	}
}

/* 0x0021e4b0: the sound of the console is set up. Its memory (0x2AD8 bytes)
is taken of that which the game takes when it starts. The console makes its
sound (unknown_0037d797) and is asked and told some things of it; the byte
at 1 says whether its speakers are of one kind (2, with the flag 16). The
processor of the sound is given its program (unknown_00191300), and who
hears faces forward. Then the voices get their streams, as many of each of
three kinds as `configuration` says (where each kind starts and ends is
kept at 0x1A0C and 0x1A12), and the slots theirs; three pairs of things are
made for the sound of the places (at 0x2A58 and 0x2A64, of the outputs 23
to 25), the handler of the sound of the places is unknown_0047f088, each of
the two places has four streams (at 0x2A70, from the outputs 15 to 22 to
the outputs 6 to 9), the two streams of 0x2A90 are silent, and the gains
are 1. TRUE if all went well, and then the first byte of the console's
sound is 1; if not, all is taken down (sound_console_dispose). The name is
a guess */
boolean __attribute__((stdcall)) sound_console_initialize(
	const struct sound_configuration *configuration)
{
	static const long place_inputs[NUMBER_OF_SOUND_CONSOLE_PLACES*NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS]=
	{
		15, 16, 17, 18, 19, 20, 21, 22
	};
	static const long place_outputs[NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS]= { 6, 7, 8, 9 };
	const short *voice_counts= (const short *)configuration;
	byte *console= shell_memory_allocate(UNKNOWN_0051EBE4_SIZE);
	struct sound_console_buffer_description description;
	unsigned long speakers;
	byte listener[SOUND_CONSOLE_LISTENER_SIZE];
	boolean ok;
	long index;
	long kind;

	memset(console, 0, UNKNOWN_0051EBE4_SIZE);
	unknown_0051ebe4= console;
	console[0]= 0;
	if (unknown_0037d797(NULL, (void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), NULL)<0 ||
		unknown_0037b5ae(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET),
			unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AA0_OFFSET)<0 ||
		unknown_0037d506(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), 3.04800010f, 0)<0 ||
		unknown_0037d5cd(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), 0.0f, 0)<0 ||
		unknown_0037b5ca(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &speakers)<0)
	{
		sound_console_dispose();
		return FALSE;
	}
	unknown_0051ebe4[UNKNOWN_0051EBE4_UNKNOWN1_OFFSET]= (speakers&0xFFFF)==2 && (speakers&0x10000);
	for (index= 0; index<=31; index++)
	{
		unknown_0037b637(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), index, 0);
	}
	unknown_0037d52a(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), 0.0f, 1);
	unknown_00379f5b();
	if (!unknown_00191300(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET)))
	{
		sound_console_dispose();
		return FALSE;
	}
	memset(listener, 0, sizeof(listener));
	memcpy(listener+SOUND_CONSOLE_LISTENER_FACING_OFFSET, global_forward3d, sizeof(real_vector3d));
	memcpy(listener+SOUND_CONSOLE_LISTENER_FACING_OFFSET+sizeof(real_vector3d), global_up3d, sizeof(real_vector3d));
	sound_console_set_listener(listener);
	ok= TRUE;
	xmem_static_buffer_allocate();

	console= unknown_0051ebe4;
	*(long *)(unknown_0047f0d0+4)= 0;
	unknown_0047f0e4= 0;
	unknown_0047f0e0= 0x10000;
	for (kind= 0; kind<NUMBER_OF_SOUND_CHANNEL_TYPES; kind++)
	{
		long first= *(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET);

		*(short *)(console+UNKNOWN_0051EBE4_COUNTS_OFFSET+kind*2)= (short)first;
		*(short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET)+= voice_counts[kind];
		for (index= 0; ok && index<voice_counts[kind]; index++)
		{
			ok= sound_console_voice_create_stream(kind, first+index)!=0;
			console= unknown_0051ebe4;
		}
		*(short *)(console+UNKNOWN_0051EBE4_COUNTS_OFFSET+(NUMBER_OF_SOUND_CHANNEL_TYPES+kind)*2)=
			*(const short *)(console+UNKNOWN_0051EBE4_VOICE_COUNT_OFFSET)-1;
	}
	*(short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_SHORT_OFFSET)= configuration->maximum_voices;
	*(short *)(console+UNKNOWN_0051EBE4_SLOT_COUNT_OFFSET)= configuration->maximum_voices;
	for (index= 0; ok && index<configuration->maximum_voices; index++)
	{
		ok= sound_console_slot_create_stream(index)!=0;
		console= unknown_0051ebe4;
	}
	unknown_00191420(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET));

	/* how the sound of no place is: half as loud in four places, and -3 */
	console= unknown_0051ebe4;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET)= 0;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+4)= 0xC0C0A8C1;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+8)= 0;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+0xC)= 0xC0C0A8C1;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+0x10)= 0xC0C0A8C1;
	*(real *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+0x14)= -3.0f;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+0x18)= 0xC0C0A8C1;
	*(unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+0x2C)= 0xC0C0A8C1;
	*(long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE)= 23;
	*(long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE+4)= 24;
	*(long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE+8)= 25;
	for (index= 0; index<3; index++)
	{
		unsigned long output= *(const unsigned long *)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+
			UNKNOWN_0051EBE4_ENVIRONMENT_SIZE+index*4);
		void **first= (void **)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE+0xC+index*4);
		void **second= (void **)(console+UNKNOWN_0051EBE4_ENVIRONMENT_OFFSET+UNKNOWN_0051EBE4_ENVIRONMENT_SIZE+0x18+index*4);

		memset(&description, 0, sizeof(description));
		description.size= sizeof(description);
		description.flags= 0x100000;
		description.unknown14= output;
		if (unknown_0037d4be(*(void **)(console+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, first, NULL)>=0)
		{
			unknown_0037b6df(*first, 0, 0, 0);
		}
		memset(&description, 0, sizeof(description));
		description.size= sizeof(description);
		description.flags= 0x100010;
		description.unknown14= output;
		if (unknown_0037d4be(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, second, NULL)>=0)
		{
			unknown_0037c620(*second, SOUND_REAL_MAXIMUM, 1);
			unknown_0037c644(*second, SOUND_REAL_MAXIMUM, 1);
			unknown_0037b6df(*second, 0, 0, 0);
		}
		console= unknown_0051ebe4;
	}
	sound_console_set_environment_handler(&unknown_0047f088);

	for (index= 0; index<NUMBER_OF_SOUND_CONSOLE_PLACES*NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS; index++)
	{
		void **stream= (void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_PLACE_STREAMS_OFFSET+index*4);

		memset(&description, 0, sizeof(description));
		description.size= sizeof(description);
		description.flags= 0x100000;
		description.unknown14= place_inputs[index];
		if (unknown_0037d4be(*(void **)(unknown_0051ebe4+UNKNOWN_0051EBE4_UNKNOWN2AB0_OFFSET), &description, stream, NULL)>=0)
		{
			long pair[2];
			struct sound_console_mixbins mixbins;

			unknown_0037b66f(*stream, 0);
			pair[0]= place_outputs[index%NUMBER_OF_SOUND_CONSOLE_PLACE_STREAMS];
			pair[1]= 0;
			mixbins.pairs= pair;
			mixbins.count= 1;
			unknown_0037c5e4(*stream, &mixbins);
			unknown_0037b6df(*stream, 0, 0, 0);
		}
	}
	console= unknown_0051ebe4;
	for (index= 0; index<NUMBER_OF_SOUND_CONSOLE_FUNCTION_STREAMS; index++)
	{
		sound_console_create_silence((void **)(console+UNKNOWN_0051EBE4_STREAMS_OFFSET+index*4));
		console= unknown_0051ebe4;
		*(long *)(console+UNKNOWN_0051EBE4_UNKNOWN2A98_OFFSET+index*4)= NONE;
	}
	*(real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET)= 1.0f;
	*(real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET+4)= 1.0f;
	*(real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET+8)= 1.0f;
	*(real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET+0xC)= 1.0f;
	*(long *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET+0x10)= 8000;
	*(real *)(console+UNKNOWN_0051EBE4_GAINS_OFFSET+0x14)= 0.0f;
	if (!ok)
	{
		sound_console_dispose();
		return FALSE;
	}
	console[0]= 1;

	return ok;
}

/* a vector turned by a quaternion, as the game works it out */
static inline __attribute__((always_inline)) void sound_rotate_vector(
	const real_quaternion *rotation,
	const real_vector3d *vector,
	real_vector3d *result)
{
	real scalar= rotation->w*rotation->w*2.0f-1.0f;
	real along= (vector->j*rotation->v.j+vector->k*rotation->v.k+rotation->v.i*vector->i)*2.0f;
	real twice= rotation->w*2.0f;

	result->i= (vector->k*rotation->v.j-vector->j*rotation->v.k)*twice+rotation->v.i*along+scalar*vector->i;
	result->j= (rotation->v.k*vector->i-vector->k*rotation->v.i)*twice+rotation->v.j*along+vector->j*scalar;
	result->k= (vector->j*rotation->v.i-rotation->v.j*vector->i)*twice+rotation->v.k*along+vector->k*scalar;
}

/* 0x00127400: where each of the four listeners is, each time, while a game
is on. One whose player there is not has no place (NONE). The rest are where
unknown_004e9c8c says for their player (0x358 bytes each: a place, where it
is in the structure bsp, a velocity, and which way it faces and which way is
up). But if the unit of the player is in a seat, and unknown_00155760 gives
1 for the listener, the listener goes part of the way to the unit
(object_get_marker95_position says where it is), and turns part of the way to where the
unit faces: the part that the seat says at 0x40; then it moves as the unit
does, and is in the structure bsp where its place is.

The listener keeps its axes (forward, left and up, with no scale), its
place, and how fast it moves as seen along its axes. Its byte at 7 says
that its cluster has something of the structure bsp's (the byte at 0x70 of
the cluster, in the block at 0x68), with nothing more if the top bit of
that byte is not set, and on the back of its plane if it is.

With `tell_console` set, the console is then told of who hears
(sound_console_set_listener): at the origin and facing forward, since the
sounds are put where the listeners see them, with the two states of the
sound of the places as its two places. The name is made up */
void __attribute__((stdcall)) sound_listeners_update(
	boolean tell_console)
{
	byte *globals;
	long index;

	if (!main_game_globals || !((const byte *)main_game_globals)[MAIN_GAME_GLOBALS_ACTIVE_OFFSET])
	{
		return;
	}
	globals= game_sound_globals;
	for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
	{
		struct sound_listener *listener= (struct sound_listener *)(globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET)+index;
		const byte *observer= unknown_004e9c8c+index*SOUND_OBSERVER_SIZE;
		long player_index= players_globals->local_players[index];
		struct scenario_location location;
		real_vector3d velocity;
		real_point3d position;
		real_vector3d forward;
		real_vector3d up;
		const byte *camera;
		boolean seated= FALSE;
		boolean unknown7= FALSE;
		long unit_index;
		long mode;

		if (player_index==NONE)
		{
			listener->leaf_index= NONE;
			listener->cluster_index= NONE;
			continue;
		}
		unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*SOUND_PLAYER_DATUM_SIZE+
			SOUND_PLAYER_DATUM_UNIT_INDEX_OFFSET);
		mode= unknown_00155760(index);
		if (unit_index!=NONE && ((1UL<<(unknown_00155760(index)&31))&3))
		{
			const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;
			const byte *unit= (const byte *)headers[unit_index&0xFFFF].datum;
			long parent_index= *(const long *)(unit+SOUND_OBJECT_PARENT_INDEX_OFFSET);

			camera= global_structure_bsp_index!=NONE ? observer : NULL;
			if (parent_index!=NONE && *(const short *)(unit+SOUND_UNIT_SEAT_INDEX_OFFSET)!=NONE && (short)mode)
			{
				const byte *definition= (const byte *)(uintptr_t)
					global_tag_instances[*(const long *)headers[parent_index&0xFFFF].datum&0xFFFF].address;
				const byte *seat= *(const byte *const *)(definition+SOUND_UNIT_DEFINITION_SEATS_OFFSET)+
					*(const short *)(unit+SOUND_UNIT_SEAT_INDEX_OFFSET)*SOUND_UNIT_SEAT_SIZE;
				const real *place= (const real *)camera;
				real_quaternion rotation;
				real_point3d head;

				object_get_marker95_position(unit_index, &head);
				position.x= (head.x-place[0])**(const real *)(seat+SOUND_UNIT_SEAT_LISTENER_FRACTION_OFFSET)+place[0];
				position.y= (head.y-place[1])**(const real *)(seat+SOUND_UNIT_SEAT_LISTENER_FRACTION_OFFSET)+place[1];
				position.z= (head.z-place[2])**(const real *)(seat+SOUND_UNIT_SEAT_LISTENER_FRACTION_OFFSET)+place[2];
				quaternion_from_vectors3d(&rotation, (const real_vector3d *)(camera+SOUND_OBSERVER_FORWARD_OFFSET),
					(const real_vector3d *)(unit+SOUND_UNIT_FORWARD_OFFSET));
				fast_quaternion_interpolate_and_normalize(global_identity_quaternion, &rotation,
					*(const real *)(seat+SOUND_UNIT_SEAT_LISTENER_FRACTION_OFFSET), &rotation);
				sound_rotate_vector(&rotation, (const real_vector3d *)(camera+SOUND_OBSERVER_FORWARD_OFFSET), &forward);
				sound_rotate_vector(&rotation, (const real_vector3d *)(camera+SOUND_OBSERVER_FORWARD_OFFSET+sizeof(real_vector3d)), &up);
				object_get_velocities(unit_index, &velocity, NULL);
				scenario_location_from_point(&location, &position);
				if (location.cluster_index==NONE)
				{
					location.leaf_index= *(const long *)(camera+SOUND_OBSERVER_LOCATION_OFFSET);
					location.cluster_index= *(const short *)(camera+SOUND_OBSERVER_LOCATION_OFFSET+4);
				}
				seated= TRUE;
			}
		}
		else
		{
			camera= global_structure_bsp_index!=NONE ? observer : NULL;
		}
		if (!seated)
		{
			memcpy(&forward, camera+SOUND_OBSERVER_FORWARD_OFFSET, sizeof(forward));
			memcpy(&up, camera+SOUND_OBSERVER_FORWARD_OFFSET+sizeof(real_vector3d), sizeof(up));
			memcpy(&position, camera, sizeof(position));
			memcpy(&velocity, camera+SOUND_OBSERVER_VELOCITY_OFFSET, sizeof(velocity));
			location.leaf_index= *(const long *)(camera+SOUND_OBSERVER_LOCATION_OFFSET);
			location.cluster_index= *(const short *)(camera+SOUND_OBSERVER_LOCATION_OFFSET+4);
		}

		listener->valid= TRUE;
		if (location.cluster_index!=NONE)
		{
			const byte *structure_bsp= global_structure_bsp;
			byte which= (*(const byte *const *)(structure_bsp+STRUCTURE_BSP_CLUSTER_COUNT_OFFSET+4)+
				location.cluster_index*STRUCTURE_BSP_CLUSTER_SIZE)[STRUCTURE_BSP_CLUSTER_UNKNOWN70_OFFSET];

			if (which!=0xFF)
			{
				const byte *entry= *(const byte *const *)(structure_bsp+STRUCTURE_BSP_UNKNOWN68_OFFSET)+
					(which&0x7F)*STRUCTURE_BSP_UNKNOWN68_SIZE;

				if (*(const short *)(entry+2)!=NONE)
				{
					if (which&0x80)
					{
						const real *plane= (const real *)(entry+4);

						unknown7= 0.0f>plane[2]*position.z+plane[1]*position.y+plane[0]*position.x-plane[3];
					}
					else
					{
						unknown7= TRUE;
					}
				}
			}
		}
		listener->unknown7= unknown7;
		memcpy(&listener->matrix.forward, &forward, sizeof(forward));
		listener->matrix.scale= 1.0f;
		listener->matrix.left.i= up.j*forward.k-up.k*forward.j;
		listener->matrix.left.j= up.k*forward.i-forward.k*up.i;
		listener->matrix.left.k= forward.j*up.i-up.j*forward.i;
		memcpy(&listener->matrix.up, &up, sizeof(up));
		memcpy(&listener->matrix.position, &position, sizeof(position));
		listener->velocity.i= listener->matrix.forward.j*velocity.j+listener->matrix.forward.k*velocity.k+
			velocity.i*listener->matrix.forward.i;
		listener->velocity.j= listener->matrix.left.i*velocity.i+listener->matrix.left.j*velocity.j+
			listener->matrix.left.k*velocity.k;
		listener->velocity.k= listener->matrix.up.j*velocity.j+listener->matrix.up.k*velocity.k+
			listener->matrix.up.i*velocity.i;
		listener->leaf_index= location.leaf_index;
		listener->cluster_index= location.cluster_index;
	}

	if (tell_console)
	{
		const struct sound_unknown_state *states;
		byte heard[SOUND_CONSOLE_LISTENER_SIZE];
		byte places[NUMBER_OF_SOUND_CONSOLE_PLACES*SOUND_CONSOLE_PLACE_SIZE];

		globals= game_sound_globals;
		states= (const struct sound_unknown_state *)(globals+GAME_SOUND_GLOBALS_UNKNOWN_STATES_OFFSET);
		for (index= 0; index<NUMBER_OF_SOUND_CONSOLE_PLACES; index++)
		{
			byte *place= places+index*SOUND_CONSOLE_PLACE_SIZE;

			*(const void **)place= states[index].index==NONE ? (const void *)sound_environment_default :
				(const void *)(uintptr_t)global_tag_instances[states[index].index&0xFFFF].address;
			memcpy(place+4, &states[index].unknownC, SOUND_CONSOLE_PLACE_NUMBERS_SIZE);
			*(long *)(place+SOUND_CONSOLE_PLACE_AMOUNT_OFFSET)= *(const long *)&states[index].amount;
		}
		memcpy(heard, unknown_00468788, sizeof(real_point3d));
		memcpy(heard+SOUND_CONSOLE_LISTENER_FACING_OFFSET, global_forward3d, sizeof(real_vector3d));
		memcpy(heard+SOUND_CONSOLE_LISTENER_FACING_OFFSET+sizeof(real_vector3d), global_up3d, sizeof(real_vector3d));
		memcpy(heard+SOUND_CONSOLE_LISTENER_FACING_OFFSET+2*sizeof(real_vector3d), global_zero_vector3d, sizeof(real_vector3d));
		*(long *)(heard+SOUND_CONSOLE_LISTENER_PLACE_COUNT_OFFSET)= NUMBER_OF_SOUND_CONSOLE_PLACES;
		*(const byte **)(heard+SOUND_CONSOLE_LISTENER_PLACE_COUNT_OFFSET+4)= places;
		sound_console_set_listener(heard);
	}
}

/* whether a state of a track of a looping sound is one of its other set:
3, 4 and 6 */
static inline __attribute__((always_inline)) boolean looping_sound_state_is_alternate(
	long state)
{
	return ((1UL<<(state&31))&0x58)!=0;
}

/* the sound of a state of a track, in the tag: NONE if it has none */
static inline __attribute__((always_inline)) long looping_sound_track_state_sound(
	const byte *track_definition,
	long state)
{
	return *(const long *)(track_definition+looping_sound_unknown_0044a188[state]+4);
}

/* 0x0021a250: a looping sound starts (0), goes on (1) or ends (2). The one
of the tag and the owner is looked for (looping_sound_find), and made if it
is to start or go on and is not there; one of a source with a type whose tag
has the flag 7 is its own: its owner is its tag with the flags 0x6000, and
each time it is asked for, how it is heard by its listener is added to its
four gains (sound_speakers_distance_gain and sound_speakers_get_gains4 say how much, for how far
it is and for where). It is kept for this time (its byte at 0x53), and takes
what the game says of it unless that has the flag 1.

One that ends, or has the flag 3, with no sounds that play, is let go.
Else each track is seen to. Its state is one of seven: in, loop and out,
their three others (3, 4 and 6) for when the flag 0 of `flags` is set and
the track has sounds for them, and 5. When it starts, the sound of its
start plays: in place of what the track plays, or with it fading in if the
track has the flag 0. When it starts or goes on, the state that the track
is to go to is looked for along looping_sound_unknown_0044a150 or
looping_sound_unknown_0044a16c: the sound of the first state that has one
plays, crossing from the one before in half a second, or as long as the
track says; those of the states that the track's flags leave out only
start. When it ends, the sounds of the track fade out in `seconds`; with
none given, in what the track says, and the sound of its end plays, or the
first sound of the track is marked to go to it (kind 3).

Last, one with no sounds that play and no listener near enough is let go;
and it keeps three flags of `flags` and what it was asked. TRUE if nothing
was to be done; if the sound is not ready, whether it was to end. The name
is made up */
boolean __attribute__((stdcall)) looping_sound_start(
	long definition_index,
	long unknown2,
	long owner,
	long controller_owner,
	const struct sound_source *source,
	long state,
	unsigned long flags,
	real seconds)
{
	boolean alternate_wanted= flags&1;
	boolean result= state==2;
	const byte *definition;
	long looping_sound_index;
	byte *looping_sound;
	boolean created;
	boolean own;
	long track;
	long key;

	if (definition_index==NONE)
	{
		return TRUE;
	}
	if (!game_sound_globals[GAME_SOUND_GLOBALS_INITIALIZED_OFFSET] || !game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN79_OFFSET] ||
		!game_sound_globals[GAME_SOUND_GLOBALS_MAP_LOADED_OFFSET])
	{
		return result;
	}
	definition= (const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address;
	if ((((const byte *)source)[SOUND_SOURCE_TYPE_OFFSET]&0xF) && (signed char)definition[0]<0)
	{
		own= TRUE;
		key= definition_index|LOOPING_SOUND_UNKNOWN8_OWN_FLAGS;
	}
	else
	{
		own= FALSE;
		key= owner;
	}
	looping_sound_index= looping_sound_find(definition_index, key);
	created= FALSE;
	result= TRUE;
	if (looping_sound_index!=NONE)
	{
		if ((flags&0x10) && state==0)
		{
			state= 1;
		}
	}
	else
	{
		if (!((1UL<<(state&31))&3))
		{
			return result;
		}
		looping_sound_index= looping_sound_new(definition_index, (short)unknown2, key, controller_owner);
		created= TRUE;
		if (looping_sound_index==NONE)
		{
			return FALSE;
		}
	}

	looping_sound= (byte *)looping_sounds->data+(looping_sound_index&0xFFFF)*LOOPING_SOUND_DATUM_SIZE;
	*(short *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN2_OFFSET)= (short)unknown2;
	if (!((((const byte *)source)[0]>>1)&1))
	{
		memcpy(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET, source, LOOPING_SOUND_DATUM_SOURCE_SIZE);
	}
	looping_sound_check_distances(definition_index, (const struct sound_source *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET));
	if (own)
	{
		const struct sound_source *said= (const struct sound_source *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET);
		real maximum_distance;
		long listener_index;
		real distance;

		if (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+3]!=game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET])
		{
			memset(looping_sound+LOOPING_SOUND_DATUM_GAINS_OFFSET, 0, NUMBER_OF_LOOPING_SOUND_GAINS*sizeof(real));
		}
		maximum_distance= said->flags&FLAG(_sound_source_maximum_distance_bit) ? said->maximum_distance :
			*(const real *)(definition+LOOPING_SOUND_DEFINITION_MAXIMUM_DISTANCE_OFFSET);
		listener_index= sound_source_get_listener(said, maximum_distance, &distance);
		if (listener_index!=NONE)
		{
			real minimum_distance= said->flags&FLAG(_sound_source_minimum_distance_bit) ? said->minimum_distance :
				*(const real *)(definition+LOOPING_SOUND_DEFINITION_MAXIMUM_DISTANCE_OFFSET-4);
			real *gains= (real *)(looping_sound+LOOPING_SOUND_DATUM_GAINS_OFFSET);
			real_point3d position;
			real parts[NUMBER_OF_LOOPING_SOUND_GAINS];
			real sum= 0.0f;
			real scale= 1.0f;
			real gain;
			long index;

			if (!(minimum_distance>0.001f))
			{
				minimum_distance= 0.001f;
			}
			gain= sound_speakers_distance_gain(minimum_distance, maximum_distance, distance);
			sound_source_get_position(said, listener_index, &position);
			if ((looping_sound[LOOPING_SOUND_DATUM_SOURCE_OFFSET+SOUND_SOURCE_TYPE_OFFSET]&0xF)==1)
			{
				matrix4x3_inverse_transform_point(
					&((const struct sound_listener *)(game_sound_globals+GAME_SOUND_GLOBALS_LISTENERS_OFFSET))[listener_index].matrix,
					&position, &position);
			}
			if (distance>minimum_distance)
			{
				normalize3d((real_vector3d *)&position);
			}
			sound_speakers_get_gains4((const real_vector3d *)&position, parts, 1);
			for (index= 0; index<NUMBER_OF_LOOPING_SOUND_GAINS; index++)
			{
				real part= parts[index]*gain;

				sum= part*part+sum;
			}
			if (sum>1.0f)
			{
				scale= 1.0f/sum;
			}
			for (index= 0; index<NUMBER_OF_LOOPING_SOUND_GAINS; index++)
			{
				gains[index]= parts[index]*scale*gain+gains[index];
			}
		}
		looping_sound[LOOPING_SOUND_DATUM_UNKNOWN2_OFFSET]&= ~2;
	}

	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+3]= game_sound_globals[GAME_SOUND_GLOBALS_UNKNOWN7F_OFFSET];
	if ((state==2 || (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&8)) && !looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1])
	{
		looping_sound_delete(looping_sound_index);
		return result;
	}
	result= FALSE;
	for (track= 0; track<*(const long *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET); track++)
	{
		const byte *track_definition= *(const byte *const *)(definition+LOOPING_SOUND_DEFINITION_TRACKS_OFFSET+4)+
			track*LOOPING_SOUND_DEFINITION_TRACK_SIZE;
		byte *track_state= looping_sound+LOOPING_SOUND_DATUM_TRACK_SOURCE_OFFSET+track*LOOPING_SOUND_TRACK_SIZE;
		unsigned long *seed= &global_random_seed_reference[1];
		unsigned long track_flags= *(const unsigned long *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET);
		boolean alternate= alternate_wanted;
		long wanted= NONE;

		if (definition[0]&LOOPING_SOUND_TRACK_OWN_BITS_FLAGS)
		{
			seed= (unsigned long *)(track_state+8);
		}
		if (looping_sound_track_state_sound(track_definition, 3)==NONE && looping_sound_track_state_sound(track_definition, 4)==NONE &&
			looping_sound_track_state_sound(track_definition, 6)==NONE && looping_sound_track_state_sound(track_definition, 5)==NONE &&
			!(track_definition[LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET]&0x14))
		{
			alternate= FALSE;
		}
		if (created)
		{
			*(long *)(track_state+LOOPING_SOUND_TRACK_PITCH_OFFSET)= *(const long *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET+4);
			track_state[LOOPING_SOUND_TRACK_CHANNELS_OFFSET]= 0;
		}
		if (created || *(const long *)track_state==NONE)
		{
			*(long *)track_state= NONE;
			track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]= alternate ? 3 : 0;
			track_state[LOOPING_SOUND_UNKNOWND_OFFSET]= alternate ? 3 : 0;
		}

		if (state==0)
		{
			long start_sound_index= looping_sound_track_state_sound(track_definition, 0);

			if (start_sound_index!=NONE)
			{
				long out_source_index= looping_sound_find_source(looping_sound_index, (short)track,
					looping_sound_track_state_sound(track_definition, 2), NONE);

				if (!(track_definition[LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET]&1))
				{
					*(long *)track_state= looping_sound_source_new(looping_sound_index, start_sound_index, NONE, track, 1,
						(long)(uintptr_t)seed);
				}
				else
				{
					looping_sound_source_new(looping_sound_index, start_sound_index, NONE, track, 0, (long)(uintptr_t)seed);
					if (out_source_index!=NONE)
					{
						real fade_seconds= *(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_FADE_IN_OFFSET);
						byte *other= sound_source_get(out_source_index);

						*(real *)(other+SOUND_SOURCE_DATUM_UNKNOWNB0_OFFSET)= sound_source_fade_decibels(out_source_index);
						*(short *)(other+SOUND_SOURCE_DATUM_UNKNOWNAE_OFFSET)= 1;
						*(long *)(other+SOUND_SOURCE_DATUM_FADE_TIME_OFFSET)= (long)(fade_seconds*1000.0f);
						*(long *)(other+SOUND_SOURCE_DATUM_UNKNOWNB8_OFFSET)= NONE;
						other[SOUND_SOURCE_DATUM_FLAGS_OFFSET]|= 0x80;
					}
					sound_source_ring_fade_out(*(const long *)track_state, 1,
						*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_FADE_IN_OFFSET));
				}
				track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]= 0;
				track_state[LOOPING_SOUND_UNKNOWND_OFFSET]= 0;
			}
		}

		if (state==2 || (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&8))
		{
			boolean alternate_now;
			boolean start_end;
			long end_sound_index;
			long first;

			if (looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+2]==2)
			{
				continue;
			}
			if (seconds!=0.0f)
			{
				if (*(const long *)track_state!=NONE)
				{
					sound_source_ring_fade_out_in_seconds(seconds, *(const long *)track_state);
				}
				continue;
			}
			alternate_now= looping_sound_state_is_alternate((char)track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]);
			if (alternate_now && ((track_flags&0x10) || looping_sound_track_state_sound(track_definition, 6)!=NONE))
			{
				end_sound_index= looping_sound_track_state_sound(track_definition, 6);
				start_end= (track_flags>>4)&1;
				alternate= TRUE;
			}
			else
			{
				end_sound_index= looping_sound_track_state_sound(track_definition, 2);
				start_end= (track_flags>>1)&1;
				alternate= FALSE;
			}
			first= *(const long *)track_state;
			if (first!=NONE)
			{
				if (!start_end && end_sound_index==NONE && (definition[0]&2))
				{
					continue;
				}
				if (start_end || end_sound_index==NONE)
				{
					sound_source_ring_fade_out_in_seconds(alternate_now && (track_flags&0x10) ?
						*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_ALTERNATE_FADE_OUT_OFFSET) :
						*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_FADE_OUT_OFFSET), first);
				}
			}
			if (end_sound_index==NONE)
			{
				continue;
			}
			if (start_end)
			{
				looping_sound_source_new(looping_sound_index, end_sound_index, NONE, track, 0, (long)(uintptr_t)seed);
				continue;
			}
			first= *(const long *)track_state;
			if (first!=NONE)
			{
				byte *other= sound_source_get(first);

				if (*(const short *)(other+SOUND_SOURCE_DATUM_CHANNEL_INDEX_OFFSET)!=NONE)
				{
					track_state[LOOPING_SOUND_UNKNOWND_OFFSET]= alternate ? 6 : 2;
					other[SOUND_SOURCE_DATUM_KIND_OFFSET]= 3;
				}
			}
			continue;
		}

		{
			long current= (char)track_state[LOOPING_SOUND_UNKNOWNC_OFFSET];
			long next= (char)track_state[LOOPING_SOUND_UNKNOWND_OFFSET];
			unsigned long only_start= (track_flags&1)|((track_flags>>1)&1 ? 0x44 : 0)|((track_flags>>2)&1 ? 0x28 : 0);
			boolean immediate;
			long new_source_index;
			long first;

			if (!(current!=next && looping_sound_track_state_sound(track_definition, next)!=NONE &&
				alternate==looping_sound_state_is_alternate(current)))
			{
				long pending= NONE;
				long from= current;
				long to= alternate ? looping_sound_unknown_0044a16c[from] : looping_sound_unknown_0044a150[from];

				while (to!=from)
				{
					long candidate= looping_sound_unknown_0044a188[to]!=NONE ? looping_sound_track_state_sound(track_definition, to) : NONE;

					if (candidate!=NONE)
					{
						if (!(only_start&(1UL<<(to&31))))
						{
							wanted= candidate;
							break;
						}
						pending= candidate;
					}
					from= to;
					to= alternate ? looping_sound_unknown_0044a16c[to] : looping_sound_unknown_0044a150[to];
				}
				track_state[LOOPING_SOUND_UNKNOWND_OFFSET]= (byte)to;
				if (pending!=NONE)
				{
					looping_sound_source_new(looping_sound_index, pending, NONE, track, 0, (long)(uintptr_t)seed);
				}
			}
			if (wanted==NONE && alternate==looping_sound_state_is_alternate((char)track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]))
			{
				continue;
			}

			immediate= state==0 && (track_definition[LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET]&1);
			first= *(const long *)track_state;
			if (first!=NONE && !immediate)
			{
				if (*(const long *)(sound_source_get(first)+SOUND_SOURCE_DATUM_SOUND_INDEX_OFFSET)==wanted ||
					alternate==looping_sound_state_is_alternate((char)track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]) ||
					!(track_definition[LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET]&4))
				{
					continue;
				}
				new_source_index= NONE;
				if (wanted!=NONE)
				{
					new_source_index= looping_sound_get_or_new_source(wanted, (short)track, NONE, looping_sound_index, 2,
						(long)(uintptr_t)seed);
					if (new_source_index==NONE)
					{
						continue;
					}
				}
				sound_sources_cross_fade(new_source_index, *(const long *)track_state, 1,
					*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_ALTERNATE_CROSS_FADE_OFFSET));
				*(long *)track_state= new_source_index;
				track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]= alternate ? 4 : 1;
				track_state[LOOPING_SOUND_UNKNOWND_OFFSET]= alternate ? 4 : 1;
				continue;
			}
			if (wanted==NONE)
			{
				continue;
			}
			{
				const struct sound_definition *sound= sound_definition_get(wanted);
				const short *bends= (const short *)((const byte *)sound_definition_scale_decibels(sound)+SOUND_SCALE_PITCH_BENDS_OFFSET);
				real low= (real)bends[0];
				long pitch_range= sound_choose_pitch_range(NONE, sound,
					((real)bends[1]-low)**(const real *)(track_state+LOOPING_SOUND_TRACK_PITCH_OFFSET)+
					(real)sound_definition_get_playback(sound)->pitch_lower_bound+low);

				new_source_index= looping_sound_find_source(looping_sound_index, (short)track, wanted, pitch_range);
				if (new_source_index==NONE)
				{
					new_source_index= looping_sound_source_new(looping_sound_index, wanted, pitch_range, track, 2, (long)(uintptr_t)seed);
					if (new_source_index==NONE)
					{
						continue;
					}
				}
			}
			if (state!=0)
			{
				sound_sources_cross_fade(new_source_index, NONE, 1, 0.5f);
			}
			else if (track_definition[LOOPING_SOUND_DEFINITION_TRACK_FLAGS_OFFSET]&1)
			{
				sound_sources_cross_fade(new_source_index, NONE, 1,
					*(const real *)(track_definition+LOOPING_SOUND_DEFINITION_TRACK_FADE_IN_OFFSET));
			}
			*(long *)track_state= new_source_index;
			track_state[LOOPING_SOUND_UNKNOWNC_OFFSET]= track_state[LOOPING_SOUND_UNKNOWND_OFFSET];
		}
	}

	if (!looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+1])
	{
		const struct sound_source *said= (const struct sound_source *)(looping_sound+LOOPING_SOUND_DATUM_SOURCE_OFFSET);

		if (sound_source_get_listener(said, said->flags&FLAG(_sound_source_maximum_distance_bit) ? said->maximum_distance :
			*(const real *)((const byte *)(uintptr_t)global_tag_instances[
			*(const long *)(looping_sound+LOOPING_SOUND_DATUM_UNKNOWN4_OFFSET)&0xFFFF].address+
			LOOPING_SOUND_DEFINITION_MAXIMUM_DISTANCE_OFFSET), NULL)==NONE)
		{
			looping_sound_delete(looping_sound_index);
			return result;
		}
	}
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]= alternate_wanted ?
		looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]|1 : looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&~1;
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]= (flags>>1)&1 ?
		looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]|2 : looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&~2;
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]= (flags>>2)&1 ?
		looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]|4 : looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET]&~4;
	looping_sound[LOOPING_SOUND_DATUM_UNKNOWN50_OFFSET+2]= (byte)state;

	return result;
}

/* of unknown_004ed288: sixteen-bit numbers, one for each of something; and
of a sound of a map: which of the things of the block of 0x30 of the sounds
of the map is its own, or NONE. One of those things is 0x34 bytes */
enum
{
	UNKNOWN_004ED288_UNKNOWN28_OFFSET= 0x28,
	CACHE_FILE_SOUND_UNKNOWND_OFFSET= 0xD,
	SOUND_GESTALT_UNKNOWN30_SIZE= 0x34
};

/* 0x0018cac0: one of the sixteen-bit numbers of unknown_004ed288, as a real
from 0 to 1 */
real sound_unknown_0018cac0(
	long said)
{
	long value= *(const word *)(unknown_004ed288+UNKNOWN_004ED288_UNKNOWN28_OFFSET+said*sizeof(word));

	if (value==0)
	{
		return 0.0f;
	}
	if (value>=0xFFFF)
	{
		return 1.0f;
	}

	return ((real)(0xFFFF-value)*0.0f+(real)value)*(1.0f/65535.0f);
}

/* 0x0018d170: the thing of the block of 0x30 of the sounds of the map that
a sound has; NULL for NONE, for a tag that is not a sound, and for a sound
that has none */
const byte *sound_get_gestalt_unknown30(
	long tag_index)
{
	if (tag_index!=NONE && global_tag_instances[(short)tag_index].group_tag==SOUND_TAG)
	{
		const byte *sound= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		char index= (char)sound[CACHE_FILE_SOUND_UNKNOWND_OFFSET];

		if (index!=NONE)
		{
			return (const byte *)(uintptr_t)global_sound_cache_file_gestalt->unknown30[0].address+
				index*SOUND_GESTALT_UNKNOWN30_SIZE;
		}
	}

	return NULL;
}

/* 0x00191660: some bits, by a text of the game and a number: for NONE, the
bits of four texts; if not, one of four bits for the first of those texts
and a number from 0 to 3; nothing for the rest */
long sound_unknown_00191660(
	long name,
	long number)
{
	if (number==NONE)
	{
		switch (name)
		{
		case 0x11000138:
			return 0xF0;
		case 0x17000137:
			return 0xC0;
		case 0x18000136:
			return 0x30;
		case 0x1300012D:
			return 0x2;
		}

		return 0;
	}
	if (name==0x11000138 && (unsigned long)number<=3)
	{
		return 0x10<<number;
	}

	return 0;
}

/* what says how a sound is to be played: only what these two set is known */
struct sound_unknown_00189fe0_source
{
	word unknown0;
	byte unknown2;
	byte unknown3;
	real scale;
	long unknown8;
	byte unknownC[0x38];
	long unknown44;
	long unknown48;
	long unknown4C;
	long unknown50;
	long unknown54;
};

typedef char verify_sound_unknown_00189fe0_source_size[
	sizeof(struct sound_unknown_00189fe0_source) == 0x58 ? 1 : -1];

/* 0x00189fe0: a sound starts, as that says. 0x0018d5b0: something for an
output of the voice */
long __attribute__((stdcall)) unknown_00189fe0(const struct sound_unknown_00189fe0_source *source, long sound_index);
long unknown_0018d5b0(long output);

/* 0x001896c0: a sound starts, with a scale and nothing else: NONE at 0x44
and 0x48. The game has it far from the rest of this file */
void __attribute__((stdcall)) sound_unknown_001896c0(
	long sound_index,
	real scale)
{
	struct sound_unknown_00189fe0_source source;

	source.unknown2= 0;
	source.unknown0= 0;
	source.unknown3= 0;
	source.unknown8= 0;
	source.unknown50= 0;
	source.unknown4C= 0;
	source.unknown54= 0;
	source.scale= scale;
	source.unknown48= NONE;
	source.unknown44= NONE;
	unknown_00189fe0(&source, sound_index);
}

/* 0x00189760: a sound of the voice starts, as four numbers say: its scale,
and at 0x44 what unknown_0018d5b0 gives for the output */
void sound_unknown_00189760(
	const long *request)
{
	struct sound_unknown_00189fe0_source source;

	source.unknown2= 0;
	source.unknown0= 0;
	source.unknown3= 0;
	source.scale= *(const real *)&request[2];
	source.unknown8= 0;
	source.unknown48= NONE;
	source.unknown44= unknown_0018d5b0(request[0]);
	source.unknown50= 0;
	source.unknown4C= 0;
	source.unknown54= 0;
	unknown_00189fe0(&source, request[1]);
}

enum
{
	/* of what unknown_00502120 points to, 0x22C bytes: four groups, 0x88
	bytes from one to the next, of eight things of two numbers and a real;
	and of what unknown_0051ebf4 points to: ten numbers */
	SOUND_UNKNOWN_00502120_SIZE= 0x22C,
	SOUND_UNKNOWN_00502120_GROUP_COUNT= 4,
	SOUND_UNKNOWN_00502120_GROUP_SIZE= 0x88,
	SOUND_UNKNOWN_00502120_ENTRY_COUNT= 8,
	SOUND_UNKNOWN_00502120_ENTRY_SIZE= 0xC,
	SOUND_UNKNOWN_0051EBF4_COUNT= 10
};

/* 0x00502120, 0x0051ebf4: two blocks of the state of the game: what they
are of is not known */
byte *unknown_00502120;
long *unknown_0051ebf4;

/* 0x00222d80: what is done with what unknown_00502120 points to when a map
goes. game_state_malloc is of game_state.c */
void unknown_00222d80(void);
void *game_state_malloc(long size);

/* 0x002229d0: what unknown_00502120 points to takes its memory of the state
of the game. It is the first of the four functions of a part of the game
(game.c). The game has game_state_malloc written here */
void sound_unknown_002229d0(
	void)
{
	unknown_00502120= game_state_malloc(SOUND_UNKNOWN_00502120_SIZE);
}

/* 0x00222a10: for a new map, it is nothing, but for the eight things of
each of its four groups: NONE for the two numbers, and 1 for the real */
void sound_unknown_00222a10(
	void)
{
	byte *unknown= unknown_00502120;
	long group;
	long index;

	memset(unknown, 0, SOUND_UNKNOWN_00502120_SIZE);
	for (group= 0; group<SOUND_UNKNOWN_00502120_GROUP_COUNT; group++)
	{
		for (index= 0; index<SOUND_UNKNOWN_00502120_ENTRY_COUNT; index++)
		{
			byte *entry= unknown+group*SOUND_UNKNOWN_00502120_GROUP_SIZE+index*SOUND_UNKNOWN_00502120_ENTRY_SIZE;

			*(long *)entry= NONE;
			*(long *)(entry+4)= NONE;
			*(real *)(entry+8)= 1.0f;
		}
	}
}

/* 0x00222a60: when a map goes: unknown_00222d80 */
void sound_unknown_00222a60(
	void)
{
	unknown_00222d80();
}

/* 0x00225a40: what unknown_0051ebf4 points to takes its memory of the state
of the game. It is the first of the two functions of another part of the
game; whether it is of the sound is a guess, by where the global is */
void sound_unknown_00225a40(
	void)
{
	unknown_0051ebf4= game_state_malloc(SOUND_UNKNOWN_0051EBF4_COUNT*sizeof(long));
}

/* 0x00225a80: for a new map, its ten numbers are nothing */
void sound_unknown_00225a80(
	void)
{
	long *unknown= unknown_0051ebf4;
	long index;

	for (index= 0; index<SOUND_UNKNOWN_0051EBF4_COUNT; index++)
	{
		unknown[index]= 0;
	}
}

enum
{
	/* of what unknown_004ed284 points to, 0x264 bytes: a byte that says that
	a map is loaded, and four numbers that are nothing for a new map */
	SOUND_UNKNOWN_004ED284_SIZE= 0x264
};

/* 0x004ed284: one more block of the state of the game, between two globals
of this file: whether it is of the sound is a guess */
byte *unknown_004ed284;

/* 0x001872d0: something for each of the four gamepads */
void unknown_001872d0(long gamepad_index, long unknown);

/* 0x00185ab0: what unknown_004ed284 points to takes its memory of the state
of the game, and no map is loaded. The game has game_state_malloc written
here */
void sound_unknown_00185ab0(
	void)
{
	byte *unknown= game_state_malloc(SOUND_UNKNOWN_004ED284_SIZE);

	unknown_004ed284= unknown;
	unknown[0]= FALSE;
}

/* 0x00185b00: for a new map, its four numbers are nothing, each of the four
gamepads is told (unknown_001872d0, with NONE), and a map is loaded */
void sound_unknown_00185b00(
	void)
{
	byte *unknown= unknown_004ed284;
	long gamepad_index;

	*(long *)(unknown+8)= 0;
	*(long *)(unknown+4)= 0;
	*(long *)(unknown+0xC)= 0;
	*(long *)(unknown+0x10)= 0;
	for (gamepad_index= 0; gamepad_index<4; gamepad_index++)
	{
		unknown_001872d0(gamepad_index, NONE);
	}
	unknown[0]= TRUE;
}

/* 0x00502124: four numbers after unknown_00502120: what they are of is not
known */
long unknown_00502124[4];

/* 0x00225700: the four numbers are nothing, and then the third is NONE. It
is the first function of a part of the game that has no other */
void sound_unknown_00225700(
	void)
{
	unknown_00502124[0]= 0;
	unknown_00502124[1]= 0;
	unknown_00502124[2]= 0;
	unknown_00502124[3]= 0;
	unknown_00502124[2]= NONE;
}

/* 0x00188850: the table of the looping sounds of the objects
(unknown_004ed28c, which the game calls "object looping sounds") is no more.
It is the second of the functions of a part of the game (game.c) */
void object_looping_sounds_dispose(
	void)
{
	if (unknown_004ed28c)
	{
		unknown_004ed28c= NULL;
	}
}

/* 0x001889d0: the looping sounds of the objects are stopped */
void __attribute__((stdcall)) unknown_001889d0(unsigned long flags);

/* 0x00188940: when a map goes, if the table of the looping sounds of the
objects is there and can be used: they are stopped (unknown_001889d0), the
table cannot be used, and two numbers of what unknown_004ed288 points to
are nothing */
void object_looping_sounds_dispose_from_old_map(
	void)
{
	struct data_array *data= unknown_004ed28c;

	if (data && data->valid)
	{
		unknown_001889d0(0);
		data->valid= FALSE;
		*(long *)(unknown_004ed288+0x20)= 0;
		*(long *)(unknown_004ed288+0x24)= 0;
	}
}

/* 0x00510c2c, of game_state.c: what gives the memory of the state of the
game */
extern struct allocation *game_state_allocation;

/* 0x001887d0: the looping sounds of the objects make their table, in the
memory of the state of the game (data_new, which the game has written
here): 0x400 of 0x18 bytes; and what unknown_004ed288 points to takes 0x244
bytes there (game_state_malloc, written here too) */
void object_looping_sounds_initialize(
	void)
{
	unknown_004ed28c= data_new("object looping sounds", 0x400, 0x18, 0, game_state_allocation);
	unknown_004ed288= game_state_malloc(0x244);
}

enum
{
	/* of what unknown_004ed288 points to, 0x244 bytes: eight numbers, two
	more, 0x80 shorts, 0x100 bytes, four reals, a number and a real */
	OBJECT_LOOPING_SOUNDS_UNKNOWN20_OFFSET= 0x20,
	OBJECT_LOOPING_SOUNDS_SHORTS_OFFSET= 0x28,
	OBJECT_LOOPING_SOUNDS_SHORT_COUNT= 0x80,
	OBJECT_LOOPING_SOUNDS_BYTES_OFFSET= 0x128,
	OBJECT_LOOPING_SOUNDS_BYTE_COUNT= 0x100,
	OBJECT_LOOPING_SOUNDS_REALS_OFFSET= 0x228,
	OBJECT_LOOPING_SOUNDS_UNKNOWN238_OFFSET= 0x238,
	OBJECT_LOOPING_SOUNDS_UNKNOWN23C_OFFSET= 0x23C
};

/* 0x00188d60: what more is done for a new map. 0x0018bb80: something that
the fifth function of the part does first */
void sound_fade_in_for_new_map(void);
void __attribute__((stdcall)) unknown_0018bb80(long unknown);

/* 0x00188870: for a new map, if the table of the looping sounds of the
objects is there: it can be used and is empty (data_delete_all), and what
unknown_004ed288 points to is as it starts: NONE for its eight numbers,
nothing for the two after them and for its shorts (a real 0 made a short),
every bit of its bytes set, 1 for its four reals, 8000 for the number and
nothing for the last real; and the console is given the four reals
(sound_console_set_gains). In any case
sound_fade_in_for_new_map is done */
void object_looping_sounds_initialize_for_new_map(
	void)
{
	struct data_array *data= unknown_004ed28c;

	if (data)
	{
		byte *unknown;
		long index;

		data->valid= TRUE;
		data_delete_all(data);
		unknown= unknown_004ed288;
		for (index= 0; index<8; index++)
		{
			*(long *)(unknown+index*4)= NONE;
		}
		*(long *)(unknown+OBJECT_LOOPING_SOUNDS_UNKNOWN20_OFFSET)= 0;
		*(long *)(unknown+OBJECT_LOOPING_SOUNDS_UNKNOWN20_OFFSET+4)= 0;
		memset(unknown+OBJECT_LOOPING_SOUNDS_SHORTS_OFFSET, 0, OBJECT_LOOPING_SOUNDS_SHORT_COUNT*sizeof(short));
		memset(unknown+OBJECT_LOOPING_SOUNDS_BYTES_OFFSET, 0xFF, OBJECT_LOOPING_SOUNDS_BYTE_COUNT);
		for (index= 0; index<4; index++)
		{
			*(real *)(unknown+OBJECT_LOOPING_SOUNDS_REALS_OFFSET+index*4)= 1.0f;
		}
		*(long *)(unknown+OBJECT_LOOPING_SOUNDS_UNKNOWN238_OFFSET)= 8000;
		*(real *)(unknown+OBJECT_LOOPING_SOUNDS_UNKNOWN23C_OFFSET)= 0.0f;
		sound_console_set_gains((const real *)(unknown+OBJECT_LOOPING_SOUNDS_REALS_OFFSET));
	}
	sound_fade_in_for_new_map();
}

/* 0x00188980: the fifth of the functions of the part, for when the game
becomes active: if there is what unknown_004ed288 points to, unknown_0018bb80
is done with nothing, and its shorts are nothing */
void object_looping_sounds_unknown_00188980(
	void)
{
	if (unknown_004ed288)
	{
		unknown_0018bb80(0);
		memset(unknown_004ed288+OBJECT_LOOPING_SOUNDS_SHORTS_OFFSET, 0, OBJECT_LOOPING_SOUNDS_SHORT_COUNT*sizeof(short));
	}
}

/* 0x004e9ae8 and 0x0055e4d0, of game_voice.c: what the game engine keeps
(the kind of game that is played at 0xC14) and, for each kind, what it has */
extern byte *unknown_004e9ae8;
extern long unknown_0055e4d0[];

/* 0x00188cd0: two seconds before a game that is over ends (game.c), in a
kind of game that is played: every class of sound goes down to nothing in a
second, and the three of what is around to a fifth */
void sound_fade_for_game_end(
	void)
{
	if (unknown_0055e4d0[*(const long *)(unknown_004e9ae8+0xC14)])
	{
		sound_classes_set_decibels("", sound_gain_to_decibels(0.0f), 1.0f);
		sound_classes_set_decibels("ambient_nature", sound_gain_to_decibels(0.2f), 1.0f);
		sound_classes_set_decibels("ambient_machinery", sound_gain_to_decibels(0.2f), 1.0f);
		sound_classes_set_decibels("ambient_computers", sound_gain_to_decibels(0.2f), 1.0f);
	}
}

/* 0x00188d60: for a new map, in a kind of game that is played: every class
of sound is nothing at once and comes up whole in two seconds; what the
player says in the scripts is whole at once */
void sound_fade_in_for_new_map(
	void)
{
	if (unknown_0055e4d0[*(const long *)(unknown_004e9ae8+0xC14)])
	{
		sound_classes_set_decibels("", sound_gain_to_decibels(0.0f), 0.0f);
		sound_classes_set_decibels("", sound_gain_to_decibels(1.0f), 2.0f);
		sound_classes_set_decibels("scripted_dialog_player", sound_gain_to_decibels(1.0f), 0.0f);
	}
}

/* 0x00224250 to 0x002243a0: six functions of something that keeps the index
of a tag at 4 and where the tag is at 8. In the game it is in ecx (a class,
most likely). Which tag it is, and whether they are of this file, is not
known: they are here by their addresses */

/* 0x002243a0: the thing takes a tag */
void __attribute__((stdcall)) tag_user_set_tag(
	byte *user,
	long tag_index)
{
	*(long *)(user+4)= tag_index;
	*(unsigned long *)(user+8)= (unsigned long)global_tag_instances[tag_index&0xFFFF].address;
}

/* 0x00224250: what the tag has at 0x84 */
long tag_user_get_unknown84(
	const byte *user)
{
	const byte *tag= *(const byte *const *)(user+8);

	return *(const long *)(tag+0x84);
}

/* 0x00224260: one of the things of 0x38 bytes that the tag has at 0x88 */
byte *__attribute__((stdcall)) tag_user_get_unknown88(
	const byte *user,
	long index)
{
	const byte *tag= *(const byte *const *)(user+8);

	return *(byte *const *)(tag+0x88)+index*0x38;
}

/* 0x002242a0: what the tag has at 0x90 */
long tag_user_get_unknown90(
	const byte *user)
{
	const byte *tag= *(const byte *const *)(user+8);

	return *(const long *)(tag+0x90);
}

/* 0x00224330: the bit 7 of the first number of the tag */
long tag_user_get_flag7(
	const byte *user)
{
	const byte *tag= *(const byte *const *)(user+8);

	return (*(const unsigned long *)tag>>7)&1;
}

/* 0x00224340: its bit 8 */
long tag_user_get_flag8(
	const byte *user)
{
	const byte *tag= *(const byte *const *)(user+8);

	return (*(const unsigned long *)tag>>8)&1;
}

/* 0x00187420: the greater of two reals (0x74 and 0x78) of what
unknown_004ed284 has for a gamepad (0x94 bytes each); the second if either is
not a number */
real sound_unknown_00187420(
	long gamepad_index)
{
	const real *values= (const real *)(unknown_004ed284+gamepad_index*0x94+0x74);

	return values[0]>values[1] ? values[0] : values[1];
}
