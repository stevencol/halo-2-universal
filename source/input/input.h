/*
INPUT.H

The gamepads of the console. As Halo 1's input_xbox.c, with no keyboard and
no mouse; the names are those of Halo 1's input.h where it has them, and
guesses where it does not.
*/

#ifndef __INPUT_H
#define __INPUT_H

#include <stdint.h>

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	MAXIMUM_GAMEPADS= 4,
	NUMBER_OF_GAMEPAD_ANALOG_BUTTONS= 8,
	NUMBER_OF_GAMEPAD_DIGITAL_BUTTONS= 8,
	NUMBER_OF_GAMEPAD_THUMBSTICK_AXES= 4,

	/* a frame of the gamepads is never longer than this, in milliseconds */
	INPUT_MAXIMUM_FRAME_MILLISECONDS= 100,

	/* an analog button is down while it is pressed more than a level, which
	follows the button: this much under it while it is down, and this much
	over it while it is not */
	GAMEPAD_ANALOG_BUTTON_DOWN_MARGIN= 0x20,
	GAMEPAD_ANALOG_BUTTON_UP_MARGIN= 0x40,

	/* what a thumbstick has to move for it to count, and what is left of
	its range to each side: it goes from -32768 to 32767 */
	GAMEPAD_THUMBSTICK_DEAD_ZONE= 9000,
	GAMEPAD_THUMBSTICK_RANGE= 23767,
	GAMEPAD_THUMBSTICK_NEGATIVE_RANGE= 23768,

	NUMBER_OF_INPUT_UNIT_DRIVES= 10,
	INPUT_HARD_DISK_DRIVE_LETTER= 'u'
};

/* the kinds of gamepad that the console tells apart */
enum
{
	_gamepad_type_unknown,
	_gamepad_type_1,
	_gamepad_type_2
};

/* ---------- structures */

struct gamepad_state
{
	byte analog_buttons[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	/* the level over which each counts as down */
	byte analog_button_thresholds[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	/* how long each button has been down: in frames, and in milliseconds */
	byte analog_button_frames_down[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	byte digital_button_frames_down[NUMBER_OF_GAMEPAD_DIGITAL_BUTTONS];
	word analog_button_msec_down[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	word digital_button_msec_down[NUMBER_OF_GAMEPAD_DIGITAL_BUTTONS];
	/* left x and y, right x and y */
	short thumbsticks[NUMBER_OF_GAMEPAD_THUMBSTICK_AXES];
};

struct gamepad_rumble
{
	word left;
	word right;
};

/* 0x004e61b8 */
struct input_globals
{
	boolean initialized;
	/* the game gets a gamepad with nothing pressed */
	boolean suppressed;
	/* the gamepads do not rumble */
	boolean rumble_disabled;
	byte pad3;
	/* GetTickCount: of the last update */
	unsigned long update_time;
	/* bit n: the drive n is there. The first is the hard disk */
	word unit_mask;
	/* the letter of each drive but the first */
	char unit_drive_letters[NUMBER_OF_INPUT_UNIT_DRIVES];
	/* of each gamepad that is plugged in; 0 if it is not */
	uint32_t gamepad_handles[MAXIMUM_GAMEPADS];
	struct gamepad_state gamepad_states[MAXIMUM_GAMEPADS];
	long gamepad_types[MAXIMUM_GAMEPADS];
	/* what the game gets when the gamepads are suppressed */
	struct gamepad_state suppressed_state;
	struct gamepad_rumble rumbles[MAXIMUM_GAMEPADS];
	/* GetTickCount: when a memory unit was last put in or taken out */
	unsigned long memory_unit_change_time;
};

/* what the console is given to make a gamepad rumble: its header, which the
console fills, and the speeds of the two motors */
#pragma pack(push, 1)
struct input_feedback
{
	uint32_t status;
	byte reserved[0x3E];
	word left_motor_speed;
	word right_motor_speed;
};
#pragma pack(pop)

/* what the console tells of the devices of a kind */
struct xdevice_type
{
	uint32_t unknown;
};

typedef char verify_gamepad_state_size[
	sizeof(struct gamepad_state) == 0x48 ? 1 : -1];
typedef char verify_input_globals_size[
	sizeof(struct input_globals) == 0x1B0 ? 1 : -1];
typedef char verify_input_feedback_size[
	sizeof(struct input_feedback) == 0x46 ? 1 : -1];

/* ---------- prototypes/INPUT_XBOX.C */

boolean input_initialize(void);
void input_dispose(void);
void input_update(void);
const struct gamepad_state *input_get_gamepad_state(short gamepad_index);
long input_get_gamepad_type(short gamepad_index);
boolean input_get_unit_drive_letter(long unit_index, char *letter);
void input_set_gamepad_rumbler_state(short gamepad_index, long left, long right);
void input_update_devices(void);
void input_update_gamepads(long elapsed_msec);
void input_flush_rumble(void);

/* ---------- globals */

extern struct input_globals input_globals;
extern struct input_feedback input_feedbacks[MAXIMUM_GAMEPADS];

#endif
