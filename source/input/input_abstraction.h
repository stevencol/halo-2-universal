/*
INPUT_ABSTRACTION.H

What the game makes of the gamepads for each controller. Only a little of it
is worked out; the name of the file is a guess, from later games.
*/

#ifndef __INPUT_ABSTRACTION_H
#define __INPUT_ABSTRACTION_H

#include <stddef.h>

#include "cseries/cseries.h"
#include "input/input.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	INPUT_ABSTRACTION_PREFERENCES_BYTES_SIZE= 0x14
};

/* ---------- structures */

/* how a controller is set when nothing else says: two reals and twenty
bytes. What they are is not known */
struct input_abstraction_preferences
{
	real unknown0;
	real unknown4;
	byte unknown8[INPUT_ABSTRACTION_PREFERENCES_BYTES_SIZE];
};

/* 0x0051ea18 */
struct input_abstraction_globals
{
	struct input_abstraction_preferences preferences[MAXIMUM_GAMEPADS];
	byte unknown70[0x140];
	/* GetTickCount: when this started */
	unsigned long time;
	/* whether each gamepad was plugged in when this started */
	boolean gamepad_present[MAXIMUM_GAMEPADS];
	boolean initialized;
	byte pad[3];
};

typedef char verify_input_abstraction_preferences_size[
	sizeof(struct input_abstraction_preferences) == 0x1C ? 1 : -1];
typedef char verify_input_abstraction_globals_size[
	sizeof(struct input_abstraction_globals) == 0x1BC ? 1 : -1];
typedef char verify_input_abstraction_globals_time_offset[
	offsetof(struct input_abstraction_globals, time) == 0x1B0 ? 1 : -1];

/* ---------- prototypes/INPUT_ABSTRACTION.C */

void input_abstraction_initialize(void);
void input_abstraction_dispose(void);
void input_abstraction_preferences_set_defaults(byte *preferences);
void input_abstraction_handle_gamepad_changes(unsigned long gamepad_changes);
boolean input_abstraction_controller_unknown_00218510(long controller_index);

/* ---------- globals */

extern struct input_abstraction_globals input_abstraction_globals;

#endif
