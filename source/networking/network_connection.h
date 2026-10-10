/*
NETWORK_CONNECTION.H

What a channel of the game has of its own to keep count of the packets that
it sends and of their answers: the things of unknown_004d87d8. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __NETWORK_CONNECTION_H
#define __NETWORK_CONNECTION_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of one of them: the most that a number of what was answered has been;
	the three numbers of the last answer; the least that an answer has
	taken; how long an answer takes, smoothed, and how much that changes;
	how long is waited for one; and a number that goes up with each packet
	that is not answered and down with each that is */
	NETWORK_CONNECTION_UNKNOWN94C_OFFSET= 0x94C,
	NETWORK_CONNECTION_LAST_UNKNOWN_OFFSET= 0x95C,
	NETWORK_CONNECTION_LAST_TIME_OFFSET= 0x960,
	NETWORK_CONNECTION_LAST_UNKNOWN2_OFFSET= 0x964,
	NETWORK_CONNECTION_LEAST_TIME_OFFSET= 0x968,
	NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET= 0x96C,
	NETWORK_CONNECTION_TIME_DEVIATION_OFFSET= 0x970,
	NETWORK_CONNECTION_TIMEOUT_OFFSET= 0x974,
	NETWORK_CONNECTION_LOSS_OFFSET= 0x978,
	/* the kinds of thing that can happen to a packet, as a channel sees
	them: 1, 4 and 5 are answers, and 3 is that there was none */
	_network_connection_event1= 1,
	_network_connection_event_lost= 3,
	_network_connection_event4= 4,
	_network_connection_event5= 5
};

/* ---------- prototypes/NETWORK_CONNECTION.C */

void __attribute__((stdcall)) network_connection_update_times(long event, byte *connection, long time, long unknown2,
	long unknown);

#endif
