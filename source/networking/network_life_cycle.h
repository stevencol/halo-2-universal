/*
NETWORK_LIFE_CYCLE.H

What the network game is doing now, and its sessions. The name of the file
is that of Halo 3's file for the same thing, and is a guess here, as are the
names of its functions.
*/

#ifndef __NETWORK_LIFE_CYCLE_H
#define __NETWORK_LIFE_CYCLE_H

#include "cseries/cseries.h"

/* ---------- constants */

/* what the network game is doing (unknown_00527334, of main_game.c): only
the numbers are known */
enum
{
	_network_life_cycle_state_none,
	_network_life_cycle_state1,
	_network_life_cycle_state2,
	_network_life_cycle_state3,
	_network_life_cycle_state4,
	_network_life_cycle_state5,
	_network_life_cycle_state6,
	_network_life_cycle_state7,
	_network_life_cycle_state8,
	_network_life_cycle_state9
};

enum
{
	/* what network_life_cycle_unknown_00059570 says in the seventh state */
	_network_life_cycle_unknown_00059570_5= 5,

	/* what unknown_00527e78 is when unknown_00527fa8 is looked at in its
	place */
	_unknown_00527e78_2= 2
};

/* ---------- prototypes/NETWORK_LIFE_CYCLE.C */

boolean network_life_cycle_in_state5(void);
long network_life_cycle_get_state(void);
boolean network_life_cycle_local_peer_is_host(void);
boolean network_life_cycle_unknown_00059330(void);
boolean network_life_cycle_unknown_00059470(void);
long network_life_cycle_unknown_000594f0(void);
long network_life_cycle_unknown_00059500(void);
long network_life_cycle_unknown_00059570(void);
boolean network_life_cycle_get_session(byte **session);
boolean network_life_cycle_get_other_session(byte **session);

/* ---------- globals */

extern byte *network_life_cycle_session;
extern byte *network_life_cycle_other_session;
extern long unknown_00527e78;
extern long unknown_00527fa8;
extern byte unknown_00527500[];

#endif
