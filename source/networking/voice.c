/*
VOICE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The voice of the players of the network game. The name of the file is a
guess, and so are the names of its functions.
*/

#include "voice.h"

#include <string.h>

/* ---------- globals */

/* 0x00476fc8 */
struct voice_globals voice_globals;

/* 0x00510c90, of sound_manager.c: what the game knows of the effects of the
processor of the console's sound. The first thing of it is what the
console's library needs of them for the voice; NULL if there is none */
extern byte *unknown_00510c90;

/* ---------- code */

/* the controller after one, and the first after NONE; NONE after the last.
The game has this in each loop over the controllers */
static inline __attribute__((always_inline)) long voice_next_controller(
	long controller_index)
{
	switch (controller_index)
	{
	case NONE:
		return 0;
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 3;
	}

	return NONE;
}

/* 0x000556a0: no level of any controller is known */
void voice_reset_controller_levels(
	struct voice_globals *voice)
{
	long controller_index;

	for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
	{
		voice->controller_levels[controller_index][3]= -1.0f;
		voice->controller_levels[controller_index][2]= -1.0f;
		voice->controller_levels[controller_index][1]= -1.0f;
		voice->controller_levels[controller_index][0]= -1.0f;
	}
}

/* 0x00055720: what each controller starts with, by what the voice is */
void voice_reset_controllers(
	struct voice_globals *voice)
{
	long unknown= voice->kind==_voice_kind_session ? _voice_controller_unknown3 : _voice_controller_unknown2;
	long controller_index;

	for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
	{
		voice->controller_unknown54[controller_index]= unknown;
	}
}

/* 0x00055780: what the console's library is asked to start the voice with;
FALSE, and nothing written, if what it needs is not there */
boolean voice_engine_parameters_new(
	struct voice_engine_parameters *parameters,
	const struct voice_globals *voice)
{
	void *unknown= *(void **)unknown_00510c90;
	long unknown0= 0;
	long unknown8= 0;

	if (!unknown)
	{
		return FALSE;
	}

	memset(parameters, 0, sizeof(*parameters));
	if (voice->kind==_voice_kind_full)
	{
		unknown0= VOICE_ENGINE_FULL_UNKNOWN0;
		unknown8= VOICE_ENGINE_FULL_UNKNOWN8;
	}
	parameters->unknown0= unknown0;
	parameters->unknown4= VOICE_ENGINE_UNKNOWN4;
	parameters->unknown8= unknown8;
	parameters->unknownC= 0;
	parameters->unknown10= unknown;
	parameters->unknown14= VOICE_ENGINE_UNKNOWN14;
	parameters->unknown18= VOICE_ENGINE_UNKNOWN18;
	parameters->unknown1C= 0;
	parameters->unknown20= 0;

	return TRUE;
}
