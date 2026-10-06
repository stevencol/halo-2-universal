/*
INPUT_ABSTRACTION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The name of the file and those of its functions are guesses.
*/

#include "input_abstraction.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "memory/data.h"
#include "objects/objects.h"

#include <stdint.h>
#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x003314b0 */
long GetTickCount(void);

/* 0x001565a0: stops something that unknown_004e9188 says is going on, and
goes to the main menu if its flag 5 says so */
void unknown_001565a0(void);

/* ---------- constants */

enum
{
	/* how long after this starts, or after the first gamepad is plugged in, a
	change of the gamepads stops what unknown_001565a0 stops: milliseconds */
	INPUT_ABSTRACTION_GAMEPAD_CHANGE_MILLISECONDS= 2000,
	/* the bits of the gamepads that were plugged in, of what
	input_update_devices tells */
	INPUT_ABSTRACTION_GAMEPAD_INSERTIONS_MASK= 0xFFF000,

	/* of a player, of an object and of the tag of a unit */
	PLAYER_DATUM_SIZE= 0x21C,
	PLAYER_DATUM_UNIT_INDEX_OFFSET= 0x2C,
	OBJECT_PARENT_INDEX_OFFSET= 0x14,
	UNIT_SEAT_INDEX_OFFSET= 0x1FC,
	UNIT_DEFINITION_SEATS_OFFSET= 0x1CC,
	UNIT_DEFINITION_UNKNOWN1F0_OFFSET= 0x1F0,
	UNIT_SEAT_SIZE= 0xB0
};

/* ---------- globals */

/* 0x0051ea18 */
struct input_abstraction_globals input_abstraction_globals;

/* 0x0055e77c: GetTickCount: when a gamepad was first plugged in; 0 if none
was */
unsigned long unknown_0055e77c;

/* 0x004e9188 and 0x004e9190: of something else, which unknown_001565a0
stops: whether it is going on, and its flags */
boolean unknown_004e9188;
unsigned long unknown_004e9190;

/* ---------- code */

/* the controller after one, or NONE after the last */
static inline __attribute__((always_inline)) long input_abstraction_next_controller(
	long controller_index)
{
	return (controller_index>=0 && controller_index<MAXIMUM_GAMEPADS-1) ? controller_index+1 : NONE;
}

/* 0x00217bc0: all starts as nothing, with each controller as
input_abstraction_preferences_set_defaults leaves it, whether its gamepad is
plugged in, and the time */
void input_abstraction_initialize(
	void)
{
	long controller_index;

	memset(&input_abstraction_globals, 0, sizeof(input_abstraction_globals));
	for (controller_index= 0; controller_index!=NONE; controller_index= input_abstraction_next_controller(controller_index))
	{
		input_abstraction_preferences_set_defaults((byte *)&input_abstraction_globals.preferences[controller_index]);
		input_abstraction_globals.gamepad_present[controller_index]= input_globals.gamepad_handles[(short)controller_index]!=0;
	}
	input_abstraction_globals.time= (unsigned long)GetTickCount();
	input_abstraction_globals.initialized= TRUE;
}

/* 0x00217c20 */
void input_abstraction_dispose(
	void)
{
	memset(&input_abstraction_globals, 0, sizeof(input_abstraction_globals));
}

/* 0x00218420: how a controller is set when nothing else says: two reals,
120 and 60, and twenty bytes */
void input_abstraction_preferences_set_defaults(
	byte *preferences)
{
	static const byte defaults[INPUT_ABSTRACTION_PREFERENCES_BYTES_SIZE]=
	{
		0, 4, 2, 3, 1, 5, 6, 7, 12, 13, 14, 15, 10, 11, 0, 1, 0, 0, 0, 0
	};

	((real *)preferences)[0]= 120.0f;
	((real *)preferences)[1]= 60.0f;
	memcpy(preferences+offsetof(struct input_abstraction_preferences, unknown8), defaults, sizeof(defaults));
}

/* 0x002184a0: input_update_devices tells of the gamepads that were plugged
in and unplugged. When any has changed, two seconds or more after this
started or after the first gamepad was plugged in, what unknown_004e9188
says is going on is stopped (unknown_001565a0), unless it has one of the
flags 8 and 10. And the time of the first gamepad that is plugged in is
kept. The changes are taken in esi */
void input_abstraction_handle_gamepad_changes(
	unsigned long gamepad_changes)
{
	if (!input_abstraction_globals.initialized)
	{
		return;
	}
	if (gamepad_changes &&
		((unsigned long)GetTickCount()-input_abstraction_globals.time>=INPUT_ABSTRACTION_GAMEPAD_CHANGE_MILLISECONDS ||
		(unknown_0055e77c && (unsigned long)GetTickCount()-unknown_0055e77c>=INPUT_ABSTRACTION_GAMEPAD_CHANGE_MILLISECONDS)) &&
		unknown_004e9188 && !(unknown_004e9190&0x500))
	{
		unknown_001565a0();
	}
	if ((gamepad_changes&INPUT_ABSTRACTION_GAMEPAD_INSERTIONS_MASK) && !unknown_0055e77c)
	{
		unknown_0055e77c= (unsigned long)GetTickCount();
	}
}

/* 0x00218510: whether the unit of the player of a controller is in a seat
with the flag 2, of something whose tag has a 3 or a 5 at 0x1F0. The
controller is taken in eax */
boolean input_abstraction_controller_unknown_00218510(
	long controller_index)
{
	long player_index= controller_index!=NONE ? players_globals->unknown1C[controller_index] : NONE;
	const struct object_header_datum *headers;
	const byte *definition;
	const byte *unit;
	long parent_index;
	long unit_index;
	short seat_index;
	short kind;

	if (player_index==NONE)
	{
		return FALSE;
	}
	unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*PLAYER_DATUM_SIZE+PLAYER_DATUM_UNIT_INDEX_OFFSET);
	if (unit_index==NONE)
	{
		return FALSE;
	}
	headers= (const struct object_header_datum *)object_header_data->data;
	unit= (const byte *)headers[unit_index&0xFFFF].datum;
	parent_index= *(const long *)(unit+OBJECT_PARENT_INDEX_OFFSET);
	if (parent_index==NONE)
	{
		return FALSE;
	}
	seat_index= *(const short *)(unit+UNIT_SEAT_INDEX_OFFSET);
	if (seat_index==NONE)
	{
		return FALSE;
	}
	definition= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)headers[parent_index&0xFFFF].datum&0xFFFF].address;
	kind= *(const short *)(definition+UNIT_DEFINITION_UNKNOWN1F0_OFFSET);
	if (kind!=3 && kind!=5)
	{
		return FALSE;
	}

	return (*(const unsigned long *)(*(const byte *const *)(definition+UNIT_DEFINITION_SEATS_OFFSET)+seat_index*UNIT_SEAT_SIZE)>>2)&1;
}
