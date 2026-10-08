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
	NUMBER_OF_VOICE_CONTROLLER_LEVELS= 4
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

/* what each controller starts with */
enum
{
	_voice_controller_unknown2= 2,
	/* in a campaign */
	_voice_controller_unknown3
};

enum
{
	/* what the library is asked for, with a full voice */
	VOICE_ENGINE_FULL_UNKNOWN0= 15,
	VOICE_ENGINE_FULL_UNKNOWN8= 5,
	VOICE_ENGINE_UNKNOWN4= 4,
	VOICE_ENGINE_UNKNOWN14= 11,
	VOICE_ENGINE_UNKNOWN18= 32
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
	unsigned long unknownC;
	/* of the console's library; NULL if there is none */
	void *engine;
	/* -1 when the voice starts */
	real controller_levels[MAXIMUM_VOICE_CONTROLLERS][NUMBER_OF_VOICE_CONTROLLER_LEVELS];
	long controller_unknown54[MAXIMUM_VOICE_CONTROLLERS];
	byte unknown64[0x2C];
};

typedef char verify_voice_engine_parameters_size[
	sizeof(struct voice_engine_parameters) == (sizeof(void *) == 4 ? 0x24 : 0x28) ? 1 : -1];
#if defined(__i386__)
typedef char verify_voice_globals_size[
	sizeof(struct voice_globals) == 0x90 ? 1 : -1];
typedef char verify_voice_globals_controller_unknown54_offset[
	offsetof(struct voice_globals, controller_unknown54) == 0x54 ? 1 : -1];
#endif

/* ---------- prototypes/VOICE.C */

void voice_reset_controller_levels(struct voice_globals *voice);
void voice_reset_controllers(struct voice_globals *voice);
boolean voice_engine_parameters_new(struct voice_engine_parameters *parameters, const struct voice_globals *voice);

/* ---------- globals */

extern struct voice_globals voice_globals;

#endif
