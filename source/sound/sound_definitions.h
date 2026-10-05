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
#include "memory/data.h"
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

enum
{
	NUMBER_OF_SOUND_CHANNEL_TYPES= 3,
	MAXIMUM_SOUND_CHANNELS= 128,
	MAXIMUM_SOUND_SOURCES= 384,
	SOUND_SOURCE_DATUM_SIZE= 0xBC
};

/* the flags of a sound class */
enum
{
	_sound_class_unknown1_bit= 1,
	_sound_class_unknown3_bit= 3,
	/* of the second flags */
	_sound_class_internal_unknown11_bit= 11,
	/* of the second flags: its sounds go on when the game is paused */
	_sound_class_internal_not_pausable_bit= 0,

	NUMBER_OF_SOUND_CLASSES= 54
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
	/* its pitch ranges: the first, and how many */
	short first_pitch_range_index;
	byte pitch_range_count;
};

/* a range of pitches of a sound: each has its own recordings */
struct sound_pitch_range
{
	byte unknown0[8];
	short first_permutation_index;
	short permutation_count;
};

/* a recording of a sound */
struct sound_permutation
{
	byte unknown0[0xC];
	short first_chunk_index;
	short chunk_count;
};

/* a piece of the samples of a recording: where it is in the map */
struct sound_chunk
{
	byte unknown0[0xC];
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
	struct tag_block unknown8[3];
	struct tag_block pitch_ranges; /* sound_pitch_range */
	struct tag_block permutations; /* sound_permutation */
	struct tag_block unknown30[2];
	struct tag_block chunks; /* sound_chunk */
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
	/* where it is in the structure bsp; NONE if it is out of it */
	long leaf_index;
	short cluster_index;
	boolean valid;
	byte unknown7[0x29];
	real_point3d position;
	byte unknown3C[0xC];
};

/* 0x0044a0c8: how much of each thing the sound has */
struct sound_configuration
{
	short unknown0;
	short unknown2;
	short unknown4;
	/* of each of the three types */
	short channel_counts[NUMBER_OF_SOUND_CHANNEL_TYPES];
	short maximum_voices;
};

/* what plays a sound */
struct sound_channel
{
	long unknown0;
	long unknown4;
	char type;
	byte unknown9;
	byte unknownA;
	byte unknownB;
	short unknownC;
	short unknownE;
	/* how far the sound has played, in seconds it seems */
	real unknown10;
	byte unknown14[4];
	/* the chunk of each permutation that plays */
	short unknown18;
	short unknown1A;
	/* the permutations that play: a pointer, or 0 */
	long unknown1C;
	long unknown20;
};

typedef char verify_sound_pitch_range_size[
	sizeof(struct sound_pitch_range) == 0xC ? 1 : -1];
typedef char verify_sound_permutation_size[
	sizeof(struct sound_permutation) == 0x10 ? 1 : -1];
typedef char verify_sound_cache_file_gestalt_chunks_offset[
	offsetof(struct sound_cache_file_gestalt, chunks) == 0x40 ? 1 : -1];
typedef char verify_sound_channel_size[
	sizeof(struct sound_channel) == 0x24 ? 1 : -1];
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
void sound_initialize(void);
void sound_dispose(void);
void sound_initialize_for_new_map(void);
void sound_dispose_from_old_map(void);
void sound_initialize_for_new_structure_bsp(void);
boolean sound_available(void);
void sound_enable(long enable);
void sound_idle(void);
void sound_load_environment(void);
boolean sound_source_request_data(byte *source);
void sound_unknown_00125ef0(const byte *unknown);
void sound_predict(long sound_index);
void __attribute__((stdcall)) sound_reset(unsigned long flags);

/* ---------- globals */

extern struct sound_cache_file_gestalt *global_sound_cache_file_gestalt;
extern byte *game_sound_globals;
extern long sound_initialize_count;
extern struct sound_channel *sound_channels;
extern struct data_array *sound_sources;
extern struct data_array *looping_sounds;
extern struct data_array *sound_voices;
extern struct data_array *sound_effects;
extern struct data_array *unknown_0051ebd8;
extern byte *unknown_0051ebe4;
extern long unknown_0047f0e0;
extern byte *unknown_004ed288;
extern void *unknown_0051ebdc;
extern const struct sound_configuration sound_configuration;

#endif
