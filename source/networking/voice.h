/*
VOICE.H

The voice of the players of the network game, over the console's library for
it. The name of the file is a guess, and so are the names of its functions.
*/

#ifndef __VOICE_H
#define __VOICE_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* one voice for each controller */
	MAXIMUM_VOICE_CONTROLLERS= 4,
	NUMBER_OF_VOICE_CONTROLLER_LEVELS= 4,
	/* the players of other consoles that the engine can have a voice for */
	MAXIMUM_VOICE_REMOTE_TALKERS= 30,
	/* what of the flags of two players has to be the same for them to be
	the same player */
	VOICE_TALKER_FLAGS_MASK= 3
};

/* what the voice is told that it is, by how much of it the game has */
enum
{
	_voice_kind_none,
	/* of a campaign in a session of the network game */
	_voice_kind_session,
	/* of anything but a campaign */
	_voice_kind_full
};

/* what the library does with a voice: each controller has one of them, and
the library is told which of them the game uses */
enum
{
	_voice_mode_unknown0,
	_voice_mode_unknown1,
	/* what each controller starts with; not used in a campaign */
	_voice_mode_unknown2,
	/* what each controller starts with in a campaign */
	_voice_mode_unknown3,
	NUMBER_OF_VOICE_MODES,

	/* those that the game uses: all of them, and in a campaign */
	VOICE_MODES_FULL= 0xF,
	VOICE_MODES_SESSION= 0xB
};

enum
{
	/* what the library is asked for, with a full voice */
	VOICE_ENGINE_FULL_UNKNOWN0= 15,
	VOICE_ENGINE_FULL_UNKNOWN8= 5,
	VOICE_ENGINE_UNKNOWN4= 4,
	VOICE_ENGINE_UNKNOWN14= 11,
	VOICE_ENGINE_UNKNOWN18= 32,

	/* what the library is told when the game uses _voice_mode_unknown2 */
	VOICE_ENGINE_UNKNOWN_00332E7C_VALUE= 15
};

/* ---------- structures */

/* what the console's library starts the voice with */
struct voice_engine_parameters
{
	/* both 0 without a full voice */
	long unknown0;
	long unknown4;
	long unknown8;
	long unknownC;
	/* the first thing of unknown_00510c90 */
	void *unknown10;
	long unknown14;
	long unknown18;
	long unknown1C;
	long unknown20;
};

/* 0x00476fc8 */
struct voice_globals
{
	/* the functions of an object */
	void *unknown0;
	boolean initialized;
	byte pad5[3];
	long kind;
	/* a bit for each of the modes that the game uses */
	unsigned long modes;
	/* of the console's library; NULL if there is none */
	void *engine;
	/* -1 when the voice starts */
	real controller_levels[MAXIMUM_VOICE_CONTROLLERS][NUMBER_OF_VOICE_CONTROLLER_LEVELS];
	long controller_modes[MAXIMUM_VOICE_CONTROLLERS];
	/* the first thing that the library says of the controller was 0 when the
	voice started */
	boolean controller_unknown64[MAXIMUM_VOICE_CONTROLLERS];
	byte unknown68[0x20];
	/* FALSE when the voice starts */
	boolean controller_unknown88[MAXIMUM_VOICE_CONTROLLERS];
	boolean controller_unknown8C[MAXIMUM_VOICE_CONTROLLERS];
};

/* a player of another console, as the console's libraries know one: a
number of 64 bits and flags. The game makes it from a number of 32 bits,
with the sign, and no flags */
struct voice_talker
{
	long long identifier;
	unsigned long flags;
};

/* what the library says of a controller. Only the first thing of it is
looked at */
struct voice_controller_status
{
	long unknown0;
	long unknown4;
	long unknown8;
};

typedef char verify_voice_engine_parameters_size[
	sizeof(struct voice_engine_parameters) == (sizeof(void *) == 4 ? 0x24 : 0x28) ? 1 : -1];
#if defined(__i386__)
typedef char verify_voice_talker_size[
	sizeof(struct voice_talker) == 0xC ? 1 : -1];
typedef char verify_voice_globals_size[
	sizeof(struct voice_globals) == 0x90 ? 1 : -1];
typedef char verify_voice_globals_controller_modes_offset[
	offsetof(struct voice_globals, controller_modes) == 0x54 ? 1 : -1];
typedef char verify_voice_globals_controller_unknown88_offset[
	offsetof(struct voice_globals, controller_unknown88) == 0x88 ? 1 : -1];
#endif

/* ---------- prototypes/VOICE.C */

void voice_reset_controller_levels(struct voice_globals *voice);
void voice_reset_controllers(struct voice_globals *voice);
boolean voice_engine_parameters_new(struct voice_engine_parameters *parameters, const struct voice_globals *voice);
boolean voice_start(struct voice_globals *voice);
void voice_initialize(struct voice_globals *voice, long kind);
void voice_dispose(struct voice_globals *voice);
boolean voice_has_remote_talker(const struct voice_globals *voice, long identifier);
boolean voice_remote_talker_unknown_000551c0(const struct voice_globals *voice, long identifier);
void voice_update_controller_levels(struct voice_globals *voice, long controller_index, real *levels);
void voice_remote_talker_unknown_00055250(const struct voice_globals *voice, long identifier, long unknown4,
	void *unknown8);
void voice_remove_remote_talkers(struct voice_globals *voice);
boolean voice_remote_talker_set(const struct voice_globals *voice, long identifier, boolean has_voice);

/* ---------- globals */

extern struct voice_globals voice_globals;
extern void *voice_modes[NUMBER_OF_VOICE_MODES];

#endif
