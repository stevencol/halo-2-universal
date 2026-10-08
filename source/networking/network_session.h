/*
NETWORK_SESSION.H

A session of the network game: the consoles that play together, and their
players. Only a little of it is known. The name of the file is a guess, and
so are the names of its functions.
*/

#ifndef __NETWORK_SESSION_H
#define __NETWORK_SESSION_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* in a session: its state, 0 for none; which console this one is and
	which one runs the session; and more that is only read */
	NETWORK_SESSION_UNKNOWN18_OFFSET= 0x18,
	NETWORK_SESSION_HOST_PEER_OFFSET= 0x50,
	NETWORK_SESSION_UNKNOWN49A8_OFFSET= 0x49A8,
	NETWORK_SESSION_UNKNOWN49AC_OFFSET= 0x49AC,
	NETWORK_SESSION_UNKNOWN49F8_OFFSET= 0x49F8,
	NETWORK_SESSION_LOCAL_PEER_OFFSET= 0x72D8,
	NETWORK_SESSION_STATE_OFFSET= 0x741C,
	NETWORK_SESSION_UNKNOWN7420_OFFSET= 0x7420
};

/* the states of a session. From the third one to the last, the session
says who is in it */
enum
{
	_network_session_state_none,
	_network_session_state1,
	_network_session_state2,
	_network_session_state3,
	_network_session_state4,
	_network_session_state5,
	_network_session_state6,
	_network_session_state7,
	_network_session_state8
};

/* ---------- prototypes/NETWORK_SESSION.C */

boolean network_session_unknown_00058d20(const byte *session);
boolean network_session_local_peer_is_host(const byte *session);
boolean network_session_established(const byte *session);
boolean network_session_unknown_00058d90(const byte *session);

#endif
