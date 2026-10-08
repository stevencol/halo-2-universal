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
	_unknown_00527e78_2= 2,

	/* where three things of the network game have a session: what
	unknown_00527400 and unknown_00527fa0 point to, and unknown_00527500,
	which is of the sixth state */
	UNKNOWN_00527400_SESSION_OFFSET= 0x30,
	UNKNOWN_00527FA0_SESSION_OFFSET= 0x38,
	UNKNOWN_00527500_SIZE= 0x10,
	UNKNOWN_00527500_UNKNOWN8_OFFSET= 0x8,
	UNKNOWN_00527508_SESSION_OFFSET= 0x30
};

/* a function that takes its arguments off the stack */
#define NETWORK_LIFE_CYCLE_CALLBACK __attribute__((stdcall))

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
void network_life_cycle_unknown_000591e0(void);
void network_life_cycle_unknown_000591f0(void);
void network_life_cycle_unknown_00059280(void);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059360(boolean unknown);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_000593a0(boolean unknown);
void network_life_cycle_unknown_00059530(void);
long network_life_cycle_unknown_00059590(void);
void network_life_cycle_unknown_000595e0(void);
boolean network_life_cycle_unknown_00059610(void);

/* ---------- globals */

extern byte *network_life_cycle_session;
extern byte *network_life_cycle_other_session;
extern long unknown_00527e78;
extern long unknown_00527fa8;
extern byte unknown_00527500[UNKNOWN_00527500_SIZE];
extern byte *unknown_00527400;
extern byte *unknown_00527fa0;
extern long unknown_005274fc;
extern boolean unknown_005274e1;
extern long unknown_00510518;
extern boolean unknown_0051051c;
extern boolean unknown_0051051d;

#endif
