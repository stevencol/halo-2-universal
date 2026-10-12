/*
SOUND_CLASSES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The name of the file and that of the function are guesses.
*/

#include "sound_definitions.h"

#include "cache/cache_files.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* in the globals tag: the block of what it says of the sound, which has
	first the reference to the tag of the classes */
	GAME_GLOBALS_SOUND_CLASSES_OFFSET= 0xC0
};

enum
{
	/* how many classes of sounds there are, and how many reals each has in
	sound_class_gains: the first three are nothing for a new map */
	SOUND_CLASSES_COUNT= 54,
	SOUND_CLASSES_GAIN_REALS= 4
};

/* ---------- globals */

/* 0x00502118, of sound_manager.c: what each class of sounds is heard at.
0x0050211c: a map is loaded */
extern float *sound_class_gains;
boolean unknown_0050211c;

/* ---------- code */

/* 0x00221810: a class of sounds. The globals tag is not asked whether it
says anything of the sound */
struct sound_class *sound_class_get(
	short class_index)
{
	const struct tag_block *block= (const struct tag_block *)((const byte *)global_game_globals+GAME_GLOBALS_SOUND_CLASSES_OFFSET);
	const struct tag_reference *reference= block->count ? (const struct tag_reference *)(uintptr_t)block->address : NULL;
	const struct sound_classes *classes=
		(const struct sound_classes *)(uintptr_t)global_tag_instances[reference->index&0xFFFF].address;

	return (struct sound_class *)(uintptr_t)classes->classes.address+class_index;
}

/* 0x00221850: what each class of sounds is heard at takes its memory of the
state of the game, and is nothing. The game has game_state_malloc written
here */
void sound_classes_initialize(
	void)
{
	float *gains= game_state_malloc(SOUND_CLASSES_COUNT*SOUND_CLASSES_GAIN_REALS*sizeof(float));

	memset(gains, 0, SOUND_CLASSES_COUNT*SOUND_CLASSES_GAIN_REALS*sizeof(float));
	sound_class_gains= gains;
}

/* 0x002218a0: for a new map, a map is loaded, and the first three reals of
each class are nothing; the fourth is left */
void sound_classes_initialize_for_new_map(
	void)
{
	float *gains= sound_class_gains;
	long index;

	unknown_0050211c= TRUE;
	for (index= 0; index<SOUND_CLASSES_COUNT; index++)
	{
		gains[1]= 0.0f;
		gains[0]= 0.0f;
		gains[2]= 0.0f;
		gains+= SOUND_CLASSES_GAIN_REALS;
	}
}

/* 0x002218e0: no map is loaded */
void sound_classes_dispose_from_old_map(
	void)
{
	unknown_0050211c= FALSE;
}

/* 0x002218f0: it has no memory */
void sound_classes_dispose(
	void)
{
	sound_class_gains= NULL;
}
