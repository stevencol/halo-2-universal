/*
USER_INTERFACE.H

What the rest of the game asks of the interface. Worked out from
halo2ship.exe; the name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __USER_INTERFACE_H
#define __USER_INTERFACE_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of one of the two things that a part of the interface has: what it
	is at, which is good at these three; and which thing it is */
	USER_INTERFACE_SCREEN_STATE_OFFSET= 0x70,
	USER_INTERFACE_SCREEN_IDENTIFIER_OFFSET= 0x610,
	_user_interface_screen_state7= 7,
	_user_interface_screen_state8,
	_user_interface_screen_state_f0= 0xF0,
	/* where a part of the interface has its two things */
	USER_INTERFACE_CHANNEL_SCREENS_OFFSET= 0x8,
	NUMBER_OF_USER_INTERFACE_CHANNEL_SCREENS= 2,
	/* the three things that user_interface_unknown_00149318 looks for */
	NUMBER_OF_USER_INTERFACE_UNKNOWN_00149318_SCREENS= 3
};

/* ---------- prototypes/USER_INTERFACE.C */

boolean user_interface_unknown_00147da5(long voice_index);
boolean user_interface_unknown_0014a224(void);
boolean user_interface_screen_unknown_00149278(const byte *screen);
boolean user_interface_unknown_001492d6(long unknown, long unknown2, long identifier);
boolean user_interface_unknown_00149318(void);

#endif
