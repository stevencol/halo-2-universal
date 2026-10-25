/*
INPUT_XBOX.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's input_xbox.c where it has them.
*/

#include "input.h"
#include "input_abstraction.h"

#include "game/game.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* what XInitDevices is told to make room for */
struct xdevice_preallocation
{
	struct xdevice_type *type;
	uint32_t count;
};

/* XINPUT_STATE */
#pragma pack(push, 1)
struct xinput_state
{
	uint32_t packet_number;
	word digital_buttons;
	byte analog_buttons[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	short thumbsticks[NUMBER_OF_GAMEPAD_THUMBSTICK_AXES];
};
#pragma pack(pop)

/* XINPUT_CAPABILITIES: only its first byte is looked at */
struct xinput_capabilities
{
	byte subtype;
	byte unknown1[0x1B];
};

/* 0x003314b0 */
long GetTickCount(void);

/* 0x004098c0 */
void XInitDevices(uint32_t count, struct xdevice_preallocation *preallocations);

/* 0x004098c5 */
int XGetDeviceChanges(struct xdevice_type *type, uint32_t *insertions, uint32_t *removals);

/* 0x00409932 */
uint32_t XInputOpen(struct xdevice_type *type, uint32_t port, uint32_t slot, void *polling_parameters);

/* 0x00409988 */
void XInputClose(uint32_t device);

/* 0x00409994 */
uint32_t XInputGetCapabilities(uint32_t device, struct xinput_capabilities *capabilities);

/* 0x00409b6c */
long XInputGetState(uint32_t device, struct xinput_state *state);

/* 0x00409bdf */
uint32_t XInputSetState(uint32_t device, struct input_feedback *feedback);

/* ---------- constants */

enum
{
	XDEVICE_PREALLOCATION_COUNT= 4,
	/* ERROR_IO_PENDING: the console is still at what it was last told */
	INPUT_FEEDBACK_STATUS_PENDING= 0x3E5,

	/* how input_abstraction_handle_gamepad_changes is told of the gamepads that
	were plugged in */
	INPUT_GAMEPAD_INSERTION_SHIFT= 12
};

/* ---------- globals */

/* 0x004e61b8 */
struct input_globals input_globals;

/* 0x004e60a0 */
struct input_feedback input_feedbacks[MAXIMUM_GAMEPADS];

/* 0x004086f8, 0x004088b0, 0x004088bc, 0x004087a8: of the console's library:
the kinds of device */
struct xdevice_type xdevice_type_gamepad;
struct xdevice_type xdevice_type_voice_microphone;
struct xdevice_type xdevice_type_voice_headphone;
struct xdevice_type xdevice_type_memory_unit;

/* 0x0054e8e0: one for each gamepad; of its player's profile, most likely.
The first flags say that there is one, and the second whether its gamepad
rumbles */
byte unknown_0054e8e0[MAXIMUM_GAMEPADS][UNKNOWN_0054E8E0_SIZE];

/* 0x00440c38, 0x00440c40: where each analog button is in what the console
tells of a gamepad, and the bit of each digital button */
static const byte gamepad_analog_button_indices[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS]= { 0, 1, 2, 3, 4, 5, 6, 7 };
static const byte gamepad_digital_button_masks[NUMBER_OF_GAMEPAD_DIGITAL_BUTTONS]=
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/* ---------- code */

/* 0x001248b0 */
boolean input_initialize(
	void)
{
	struct xdevice_preallocation preallocations[3];

	preallocations[0].type= &xdevice_type_gamepad;
	preallocations[0].count= XDEVICE_PREALLOCATION_COUNT;
	preallocations[1].type= &xdevice_type_voice_microphone;
	preallocations[1].count= XDEVICE_PREALLOCATION_COUNT;
	preallocations[2].type= &xdevice_type_voice_headphone;
	preallocations[2].count= XDEVICE_PREALLOCATION_COUNT;
	XInitDevices(3, preallocations);

	memset(&input_globals, 0, sizeof(input_globals));
	input_globals.unit_mask|= 1;
	input_globals.initialized= TRUE;
	input_update();

	return input_globals.initialized;
}

/* 0x00124920 */
void input_dispose(
	void)
{
	long gamepad_index;

	for (gamepad_index= 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index])
		{
			XInputClose(input_globals.gamepad_handles[gamepad_index]);
			input_globals.gamepad_handles[gamepad_index]= 0;
		}
	}
}

/* 0x00124950: once a frame */
void input_update(
	void)
{
	if (input_globals.initialized)
	{
		long time= GetTickCount();
		long elapsed_msec= time-(long)input_globals.update_time;

		if (elapsed_msec<0)
		{
			elapsed_msec= 0;
		}
		else if (elapsed_msec>INPUT_MAXIMUM_FRAME_MILLISECONDS)
		{
			elapsed_msec= INPUT_MAXIMUM_FRAME_MILLISECONDS;
		}
		input_globals.suppressed= FALSE;
		input_globals.update_time= (unsigned long)time;
		input_update_devices();
		input_update_gamepads(elapsed_msec);
		input_flush_rumble();
	}
}

/* 0x001249a0: NULL if the gamepad is not plugged in */
const struct gamepad_state *input_get_gamepad_state(
	short gamepad_index)
{
	if (!input_globals.gamepad_handles[gamepad_index])
	{
		return NULL;
	}
	if (input_globals.suppressed)
	{
		return &input_globals.suppressed_state;
	}

	return &input_globals.gamepad_states[gamepad_index];
}

/* 0x001249d0 */
long input_get_gamepad_type(
	short gamepad_index)
{
	if (input_globals.gamepad_handles[gamepad_index])
	{
		return input_globals.gamepad_types[gamepad_index];
	}

	return _gamepad_type_unknown;
}

/* 0x001249f0: the letter of a drive, the hard disk or a memory unit, and
whether it is there. What sets the mask and the letters is not decompiled */
boolean input_get_unit_drive_letter(
	long unit_index,
	char *letter)
{
	unsigned long mask= 1UL<<(unit_index&31);

	if (!(mask&input_globals.unit_mask))
	{
		*letter= 0;
	}
	else if (unit_index)
	{
		*letter= ((const char *)&input_globals.unit_mask)[unit_index+1];
	}
	else
	{
		*letter= INPUT_HARD_DISK_DRIVE_LETTER;
	}

	return (mask&input_globals.unit_mask)!=0;
}

/* 0x00124a40: the speeds of the two motors of a gamepad; none if its
player does not want it to rumble */
void input_set_gamepad_rumbler_state(
	short gamepad_index,
	long left,
	long right)
{
	const byte *unknown= unknown_0054e8e0[gamepad_index];
	unsigned long flags;
	unsigned long flags2;

	memcpy(&flags, unknown, sizeof(flags));
	memcpy(&flags2, unknown+UNKNOWN_0054E8E0_FLAGS2_OFFSET, sizeof(flags2));
	if (((flags>>4)&1) && ((flags2>>1)&1))
	{
		input_globals.rumbles[gamepad_index].left= 0;
		input_globals.rumbles[gamepad_index].right= 0;
	}
	else
	{
		input_globals.rumbles[gamepad_index].left= (word)left;
		input_globals.rumbles[gamepad_index].right= (word)right;
	}
}

/* 0x00124aa0: opens the gamepads that were plugged in and closes those that
were unplugged, and looks at whether a memory unit was put in or taken out */
void input_update_devices(
	void)
{
	unsigned long gamepad_changes= 0;
	uint32_t insertions;
	uint32_t removals;

	if (XGetDeviceChanges(&xdevice_type_gamepad, &insertions, &removals))
	{
		long gamepad_index;

		for (gamepad_index= 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
		{
			uint32_t mask= 1U<<gamepad_index;

			if ((removals&mask) && input_globals.gamepad_handles[gamepad_index])
			{
				XInputClose(input_globals.gamepad_handles[gamepad_index]);
				input_globals.gamepad_handles[gamepad_index]= 0;
			}
			if (insertions&mask)
			{
				struct xinput_capabilities capabilities;
				uint32_t handle;

				input_globals.gamepad_types[gamepad_index]= _gamepad_type_unknown;
				handle= XInputOpen(&xdevice_type_gamepad, (uint32_t)gamepad_index, 0, NULL);
				input_globals.gamepad_handles[gamepad_index]= handle;
				if (handle && XInputGetCapabilities(handle, &capabilities)==0)
				{
					if (capabilities.subtype==_gamepad_type_1)
					{
						input_globals.gamepad_types[gamepad_index]= _gamepad_type_1;
					}
					else if (capabilities.subtype==_gamepad_type_2)
					{
						input_globals.gamepad_types[gamepad_index]= _gamepad_type_2;
					}
				}
			}
		}
		gamepad_changes= (removals&((1U<<MAXIMUM_GAMEPADS)-1))|
			((insertions&((1U<<MAXIMUM_GAMEPADS)-1))<<INPUT_GAMEPAD_INSERTION_SHIFT);
	}

	if (XGetDeviceChanges(&xdevice_type_memory_unit, &insertions, &removals))
	{
		input_globals.memory_unit_change_time= (unsigned long)GetTickCount();
	}
	input_abstraction_handle_gamepad_changes(gamepad_changes);
}

/* what a thumbstick says, with what is near its middle taken as its middle.
In Halo 1, fix_dead_zone; the game has this four times */
static inline __attribute__((always_inline)) short gamepad_thumbstick_fix_dead_zone(
	short value)
{
	long fixed= 0;

	if (value>GAMEPAD_THUMBSTICK_DEAD_ZONE)
	{
		fixed= ((value-GAMEPAD_THUMBSTICK_DEAD_ZONE)*0x7FFF)/GAMEPAD_THUMBSTICK_RANGE;
	}
	if (value<-GAMEPAD_THUMBSTICK_DEAD_ZONE)
	{
		fixed= ((-GAMEPAD_THUMBSTICK_DEAD_ZONE-value)*0x8000)/-GAMEPAD_THUMBSTICK_NEGATIVE_RANGE;
	}

	return (short)fixed;
}

/* 0x00124c00: reads the gamepads: what each button and thumbstick says, and
how long each button has been down */
void input_update_gamepads(
	long elapsed_msec)
{
	long gamepad_index;

	for (gamepad_index= 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
	{
		struct gamepad_state *state= &input_globals.gamepad_states[gamepad_index];
		struct xinput_state xstate;
		long index;

		if (!input_globals.gamepad_handles[gamepad_index] ||
			XInputGetState(input_globals.gamepad_handles[gamepad_index], &xstate)<0)
		{
			continue;
		}

		for (index= 0; index<NUMBER_OF_GAMEPAD_ANALOG_BUTTONS; index++)
		{
			/* by what it said the last time */
			boolean down= state->analog_buttons[index]>state->analog_button_thresholds[index];
			long msec= 0;
			long frames= 0;
			byte value;

			state->analog_buttons[index]= xstate.analog_buttons[gamepad_analog_button_indices[index]];
			if (down)
			{
				frames= state->analog_button_frames_down[index]+1;
				if (frames>UNSIGNED_CHAR_MAX)
				{
					frames= UNSIGNED_CHAR_MAX;
				}
				msec= state->analog_button_msec_down[index]+elapsed_msec;
				if (msec>UNSIGNED_SHORT_MAX)
				{
					msec= UNSIGNED_SHORT_MAX;
				}
			}
			state->analog_button_frames_down[index]= (byte)frames;
			state->analog_button_msec_down[index]= (word)msec;

			value= state->analog_buttons[index];
			if (down)
			{
				value= value<GAMEPAD_ANALOG_BUTTON_DOWN_MARGIN ? 0 : value-GAMEPAD_ANALOG_BUTTON_DOWN_MARGIN;
				if (value>state->analog_button_thresholds[index])
				{
					state->analog_button_thresholds[index]= value;
				}
			}
			else
			{
				value= value>UNSIGNED_CHAR_MAX-GAMEPAD_ANALOG_BUTTON_UP_MARGIN ? UNSIGNED_CHAR_MAX :
					value+GAMEPAD_ANALOG_BUTTON_UP_MARGIN;
				if (value<state->analog_button_thresholds[index])
				{
					state->analog_button_thresholds[index]= value;
				}
			}
		}

		for (index= 0; index<NUMBER_OF_GAMEPAD_DIGITAL_BUTTONS; index++)
		{
			boolean down= (word)(gamepad_digital_button_masks[index]&xstate.digital_buttons)!=0;
			long msec= 0;
			long frames= 0;

			if (down)
			{
				frames= state->digital_button_frames_down[index]+1;
				if (frames>UNSIGNED_CHAR_MAX)
				{
					frames= UNSIGNED_CHAR_MAX;
				}
				msec= state->digital_button_msec_down[index]+elapsed_msec;
				if (msec>UNSIGNED_SHORT_MAX)
				{
					msec= UNSIGNED_SHORT_MAX;
				}
			}
			state->digital_button_frames_down[index]= (byte)frames;
			state->digital_button_msec_down[index]= (word)msec;
		}

		for (index= 0; index<NUMBER_OF_GAMEPAD_THUMBSTICK_AXES; index++)
		{
			state->thumbsticks[index]= gamepad_thumbstick_fix_dead_zone(xstate.thumbsticks[index]);
		}
	}
}

/* 0x00124ee0: tells each gamepad how to rumble, if the console is done with
what it was last told: not at all when there is no game or it is paused */
void input_flush_rumble(
	void)
{
	boolean quiet= input_globals.rumble_disabled || input_globals.suppressed;
	long gamepad_index;

	if (!main_game_globals || !main_game_globals->active ||
		(game_time_globals->initialized && game_time_globals->paused))
	{
		quiet= TRUE;
	}

	for (gamepad_index= 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
	{
		struct input_feedback *feedback= &input_feedbacks[gamepad_index];
		uint32_t handle= input_globals.gamepad_handles[gamepad_index];

		if (handle)
		{
			if (feedback->status==0)
			{
				feedback->left_motor_speed= quiet ? 0 : input_globals.rumbles[gamepad_index].left;
				feedback->right_motor_speed= quiet ? 0 : input_globals.rumbles[gamepad_index].right;
				XInputSetState(handle, feedback);
			}
			else if (feedback->status!=INPUT_FEEDBACK_STATUS_PENDING)
			{
				feedback->status= 0;
			}
		}
	}
}

/* 0x0023e400: whether any gamepad that is there has a button (a byte at
0x10 of its state, or of the state of the suppressed ones) */
boolean input_unknown_0023e400(
	long index)
{
	boolean result= FALSE;
	long slot;

	for (slot= 0; slot<4; slot++)
	{
		if (input_globals.gamepad_handles[slot])
		{
			const byte *state= input_globals.suppressed ? (const byte *)&input_globals.suppressed_state : (const byte *)&input_globals.gamepad_states[slot];

			if (state[0x10+index])
			{
				result= TRUE;
			}
		}
	}

	return result;
}
