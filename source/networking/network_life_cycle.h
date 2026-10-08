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
	UNKNOWN_00527508_SESSION_OFFSET= 0x30,

	/* one thing for each of the ten things that the network game can be
	doing: the first is that of doing nothing */
	NUMBER_OF_NETWORK_LIFE_CYCLE_STATES= 10,

	/* what network_life_cycle_unknown_000594f0 says when
	network_life_cycle_unknown_000594a0 goes on, and what
	network_life_cycle_unknown_00059570 says then */
	_network_life_cycle_unknown_000594f0_3= 3,
	_network_life_cycle_unknown_00059570_2= 2,
	_network_life_cycle_unknown_00059570_4= 4,

	/* the most that the number of a request of the second or the third kind
	can be, and the last of the first kind; and the most that the other
	three numbers of the first kind can be */
	MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_VALUE= 0x3FF,
	MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWN4= 15,
	MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWN8= 15,
	MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWNC= 63
};

/* something that is asked of the network game: its kind, and numbers that
are 0 but for one, in the second kind and in the third. What is asked with
each is not known */
struct network_life_cycle_request
{
	long kind;
	long unknown4;
	long unknown8;
	long unknownC;
	long unknown10;
	long unknown14;
	long unknown18;
};

typedef char verify_network_life_cycle_request_size[
	sizeof(struct network_life_cycle_request) == 0x1C ? 1 : -1];

/* the functions of what the network game can be doing: what is known of
them is how network_life_cycle_end calls them */
struct network_life_cycle_handler;

struct network_life_cycle_handler_functions
{
	void *unknown0;
	void (__attribute__((thiscall)) *enter)(struct network_life_cycle_handler *handler,
		struct network_life_cycle_handler *from, long unknown4, long unknown8);
	void (__attribute__((thiscall)) *exit)(struct network_life_cycle_handler *handler,
		struct network_life_cycle_handler *to);
};

struct network_life_cycle_handler
{
	const struct network_life_cycle_handler_functions *functions;
};

/* a function that takes its arguments off the stack */
#define NETWORK_LIFE_CYCLE_CALLBACK __attribute__((stdcall))

/* ---------- prototypes/NETWORK_LIFE_CYCLE.C */

boolean NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059890(long unknown0, long unknown4, long unknown8);

boolean network_life_cycle_in_state5(void);
boolean network_life_cycle_session_is_open(void);
boolean network_life_cycle_unknown_00063f00(void);
long network_life_cycle_get_state(void);
boolean network_life_cycle_local_peer_is_host(void);
boolean network_life_cycle_unknown_00059330(void);
boolean network_life_cycle_unknown_00059470(void);
long network_life_cycle_unknown_000594f0(void);
long network_life_cycle_unknown_00059500(void);
long network_life_cycle_unknown_00059570(void);
boolean network_life_cycle_get_session(byte **session);
boolean network_life_cycle_get_other_session(byte **session);
boolean network_life_cycle_unknown_000591e0(void);
boolean network_life_cycle_unknown_000591f0(void);
void network_life_cycle_unknown_00059280(void);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059360(boolean unknown);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_000593a0(boolean unknown);
void network_life_cycle_unknown_00059530(void);
long network_life_cycle_unknown_00059590(void);
void network_life_cycle_unknown_000595e0(void);
boolean network_life_cycle_unknown_00059610(void);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059200(long unknown0, long unknown4, long unknown8,
	boolean unknownC);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059230(long unknown0, long unknown4, long unknown8);
void network_life_cycle_end(void);
void network_life_cycle_update(void);
boolean NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_000594a0(long unknown0, long unknown4, long unknown8);
boolean network_life_cycle_get_current_session(byte **session);
boolean network_life_cycle_get_any_session(byte **session);
boolean network_life_cycle_get_state_session(byte **session);
boolean network_life_cycle_unknown_00059840(byte **session);
void network_life_cycle_unknown_000598f0(const byte *session);
void network_life_cycle_request_new2(struct network_life_cycle_request *request, long value);
void network_life_cycle_request_new3(struct network_life_cycle_request *request, long value);
void network_life_cycle_request_new4(struct network_life_cycle_request *request);
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_request_new1(long unknown4, long unknown8, long unknownC,
	long unknown10, struct network_life_cycle_request *request);

/* ---------- globals */

extern byte *network_life_cycle_session;
extern byte *network_life_cycle_other_session;
extern long unknown_00527e78;
extern long unknown_00527fa8;
extern byte unknown_00527500[UNKNOWN_00527500_SIZE];
extern byte unknown_005273f8[];
extern byte unknown_0047708c[];
extern byte unknown_0047712c[];
extern byte *unknown_00527360;
extern byte *unknown_00527400;
extern byte *unknown_00527fa0;
extern long unknown_005274fc;
extern boolean unknown_005274e1;
extern long unknown_00510518;
extern boolean unknown_0051051c;
extern boolean unknown_0051051d;
extern struct network_life_cycle_handler *network_life_cycle_handlers[NUMBER_OF_NETWORK_LIFE_CYCLE_STATES];
extern boolean unknown_0052737c;
extern boolean unknown_0052737d;
extern byte unknown_00510528[];

#endif
