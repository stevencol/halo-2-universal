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
#include "cache/texture_cache.h"

#include <math.h>
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
	UNIT_SEAT_SIZE= 0xB0,

	/* of the preferences of a controller: which button of the gamepad each of
	the sixteen is, how the sticks are, and two bytes of looking up and down */
	INPUT_ABSTRACTION_PREFERENCES_BUTTONS_OFFSET= 0x8,
	INPUT_ABSTRACTION_PREFERENCES_STICKS_OFFSET= 0x18
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

/* a real, no less than -1 and no more than 1 */
static inline __attribute__((always_inline)) real input_abstraction_pin(
	real value)
{
	if (-1.0f>value)
	{
		return -1.0f;
	}

	return value>1.0f ? 1.0f : value;
}

/* -1 for a real under 0, and 1 for the rest */
static inline __attribute__((always_inline)) real input_abstraction_sign(
	real value)
{
	return (real)(0.0f>value ? -1 : 1);
}

/* what a stick says, from -1 to 1 each way: stretched so that it gets to 1
all the way round and not only along its two axes. Gives its angle */
static inline __attribute__((always_inline)) long double input_abstraction_stick(
	real x,
	real y,
	real *stick_x,
	real *stick_y)
{
	long double angle= atan2l((long double)y, (long double)x);
	real sine= (real)fabsl(sinl(angle));
	long double cosine= fabsl(cosl(angle));
	real largest= (long double)sine>cosine ? sine : (real)cosine;
	real scale= 1.0f/largest;

	*stick_x= input_abstraction_pin(x*scale*(1.0f/32767.0f));
	*stick_y= input_abstraction_pin(y*scale*(1.0f/32767.0f));

	return angle;
}

/* a stick that only goes four ways: within some angle of a diagonal, the
lesser of its two axes shrinks to nothing as the stick gets to the diagonal;
out of it, only the greater axis is left. With the length that it had */
static inline __attribute__((always_inline)) void input_abstraction_stick_four_ways(
	real *stick_x,
	real *stick_y,
	long double away,
	real away_real,
	long double limit,
	long double turned,
	real turned_real)
{
	real x= *stick_x;
	real y= *stick_y;
	real length= (real)sqrtl((long double)y*(long double)y+(long double)x*(long double)x);

	if (limit>away)
	{
		if ((long double)0.785398185f>turned || turned_real>2.35619450f)
		{
			*stick_x= input_abstraction_sign(x)*length;
			*stick_y= (1.0f-away_real*1.63702226f)*input_abstraction_sign(y)*length;
		}
		else
		{
			*stick_y= input_abstraction_sign(y)*length;
			*stick_x= (1.0f-away_real*1.63702226f)*input_abstraction_sign(x)*length;
		}
	}
	else if (fabsl((long double)x)>fabsl((long double)y))
	{
		*stick_x= input_abstraction_sign(x)*length;
		*stick_y= 0.0f;
	}
	else
	{
		*stick_y= input_abstraction_sign(y)*length;
		*stick_x= 0.0f;
	}
	*stick_x= input_abstraction_pin(*stick_x);
	*stick_y= input_abstraction_pin(*stick_y);
}

/* 0x00217c40: what each controller says, each time, from its gamepad: for a
gamepad that is not plugged in, only that. The sixteen buttons, in the order
that the controller's preferences have them: for how many frames and for how
long each has been down. The two triggers, from 0 to 1. Where two of the
buttons are in that order (the 1, and the 6, or the 7 if the eighth is the
6). And the two sticks (input_abstraction_stick), as five reals: by the
number at 0x18 of the preferences, which stick moves and which looks, and
for the numbers 2 and 3 they only go four ways (the left one within 35
degrees of a diagonal and the right one within 10). The looking up and down
is the other way round with the byte at 0x1A, or with the one at 0x1B when
input_abstraction_controller_unknown_00218510 says so */
void input_abstraction_update(
	void)
{
	static const real diagonals[4]= { 0.785398185f, 2.35619450f, -0.785398185f, -2.35619450f };
	long controller_index;

	for (controller_index= 0; controller_index!=NONE; controller_index= input_abstraction_next_controller(controller_index))
	{
		struct input_abstraction_state *state= &input_abstraction_globals.states[controller_index];
		const byte *preferences= (const byte *)&input_abstraction_globals.preferences[controller_index];
		const struct gamepad_state *gamepad;
		long double left_angle;
		long double right_angle;
		real left_angle_real;
		double left_angle_double;
		real right_angle_real;
		boolean inverted;
		real vertical;
		real left_x;
		real left_y;
		real right_x;
		real right_y;
		long button;
		long index;
		short mode;

		if (!input_globals.gamepad_handles[(short)controller_index])
		{
			input_abstraction_globals.gamepad_present[controller_index]= FALSE;
			continue;
		}
		gamepad= input_globals.suppressed ? &input_globals.suppressed_state : &input_globals.gamepad_states[(short)controller_index];
		left_angle= input_abstraction_stick((real)gamepad->thumbsticks[0], (real)gamepad->thumbsticks[1], &left_x, &left_y);
		left_angle_double= (double)left_angle;
		left_angle_real= (real)left_angle;
		right_angle= input_abstraction_stick((real)gamepad->thumbsticks[2], (real)gamepad->thumbsticks[3], &right_x, &right_y);
		right_angle_real= (real)right_angle;
		for (index= 0; index<INPUT_ABSTRACTION_BUTTON_COUNT; index++)
		{
			byte which= preferences[INPUT_ABSTRACTION_PREFERENCES_BUTTONS_OFFSET+index];

			state->frames_down[index]= gamepad->analog_button_frames_down[which];
			state->msec_down[index]= gamepad->analog_button_msec_down[which];
		}
		state->triggers[0]= (real)gamepad->analog_buttons[6]*(1.0f/255.0f);
		state->triggers[1]= (real)gamepad->analog_buttons[7]*(1.0f/255.0f);
		state->unknown4E= FALSE;
		button= 6;
		if (preferences[INPUT_ABSTRACTION_PREFERENCES_BUTTONS_OFFSET+7]==6)
		{
			button= 7;
			state->unknown4E= TRUE;
		}
		state->unknown4C= 6;
		for (index= 0; index<INPUT_ABSTRACTION_BUTTON_COUNT; index++)
		{
			if (preferences[INPUT_ABSTRACTION_PREFERENCES_BUTTONS_OFFSET+index]==button)
			{
				state->unknown4C= (byte)index;
				break;
			}
		}
		state->unknown4D= 4;
		for (index= 0; index<INPUT_ABSTRACTION_BUTTON_COUNT; index++)
		{
			if (preferences[INPUT_ABSTRACTION_PREFERENCES_BUTTONS_OFFSET+index]==1)
			{
				state->unknown4D= (byte)index;
				break;
			}
		}

		mode= *(const short *)(preferences+INPUT_ABSTRACTION_PREFERENCES_STICKS_OFFSET);
		if (mode==2 || mode==3)
		{
			long left_diagonal= (0.0f>left_x ? 1 : 0)|(0.0f>left_y ? 2 : 0);
			long right_diagonal= (0.0f>right_x ? 1 : 0)|(0.0f>right_y ? 2 : 0);
			long double left_away= fabsl((long double)left_angle_real-(long double)diagonals[left_diagonal]);
			real right_away_real= right_angle_real-diagonals[right_diagonal];
			long double left_turned= fabsl((long double)left_angle_double);
			long double right_away= fabsl((long double)right_away_real);
			long double right_turned= fabsl(right_angle);

			input_abstraction_stick_four_ways(&left_x, &left_y, left_away, (real)left_away, (long double)0.610865235f,
				left_turned, (real)left_turned);
			input_abstraction_stick_four_ways(&right_x, &right_y, right_away, (real)right_away, (long double)0.174532920f,
				right_turned, (real)right_turned);
		}

		inverted= preferences[INPUT_ABSTRACTION_PREFERENCES_STICKS_OFFSET+2];
		if (!inverted && preferences[INPUT_ABSTRACTION_PREFERENCES_STICKS_OFFSET+3])
		{
			inverted= input_abstraction_controller_unknown_00218510(controller_index);
		}
		switch (mode)
		{
		case 1:
			state->sticks[2]= 0.0f-left_x;
			vertical= left_y;
			state->sticks[1]= 0.0f-right_x;
			state->sticks[0]= right_y;
			break;
		case 2:
			state->sticks[0]= left_y;
			state->sticks[2]= 0.0f-left_x;
			state->sticks[1]= 0.0f-right_x;
			vertical= right_y;
			break;
		case 3:
			state->sticks[1]= 0.0f-left_x;
			vertical= left_y;
			state->sticks[2]= 0.0f-right_x;
			state->sticks[0]= right_y;
			break;
		default:
			state->sticks[0]= left_y;
			state->sticks[1]= 0.0f-left_x;
			state->sticks[2]= 0.0f-right_x;
			vertical= right_y;
			break;
		}
		state->sticks[4]= vertical;
		state->sticks[3]= (inverted ? -1.0f : 1.0f)*vertical;
		if (!input_abstraction_globals.gamepad_present[controller_index])
		{
			input_abstraction_globals.gamepad_present[controller_index]= TRUE;
		}
	}
}

/* 0x004e9194: a number of the same part as unknown_004e9188 */
long unknown_004e9194;

/* 0x00155f60: whether both unknown_004e9194 and unknown_004e9188 are set.
It is far before the others of this file in the game */
long unknown_004e9194_and_unknown_004e9188(
	void)
{
	return unknown_004e9194 && unknown_004e9188;
}

/* 0x004e91a0, 0x004e925c and 0x004e9264: a texture, the permanent memory it
was in, and its size */
void *unknown_004e91a0;
void *unknown_004e925c;
long unknown_004e9264;

/* 0x001565e0: with unknown_004e9188, the permanent memory is given back to
the texture cache, and they are cleared */
void input_abstraction_unknown_001565e0(
	void)
{
	if (unknown_004e9188 && unknown_004e925c)
	{
		texture_cache_return_memory(unknown_004e925c);
		unknown_004e925c= NULL;
		unknown_004e91a0= NULL;
		unknown_004e9264= 0;
	}
}
