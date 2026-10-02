/*
SOUND_CLASSES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The name of the file and that of the function are guesses.
*/

#include "sound_definitions.h"

#include "cache/cache_files.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	/* in the globals tag: the block of what it says of the sound, which has
	first the reference to the tag of the classes */
	GAME_GLOBALS_SOUND_CLASSES_OFFSET= 0xC0
};

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
