/*
TRANSPORT.H

The network of the console: started when its cable is there and stopped when
it is not, with what is told of each. The name of the file is a guess, and
so are the names of its functions.
*/

#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* how many things can be told that the network started or stopped; and
	the bit of what the console says of its cable that says that it is
	there */
	MAXIMUM_TRANSPORT_USERS= 8,
	_transport_link_active_bit= 0x1,
	/* the version of the sockets that the game asks for, and the size of
	what the libraries say of them */
	TRANSPORT_WINSOCK_VERSION= 2,
	TRANSPORT_WINSOCK_DATA_SIZE= 0x190
};

/* ---------- structures */

/* what the network of the console is started with: its size, its bits, and
ten numbers of how much of each thing it is to have */
struct transport_startup_parameters
{
	byte size;
	byte flags;
	byte unknown2[10];
};

/* what is told that the network started or stopped is given what it asked
to be given */
typedef void (__attribute__((stdcall)) *transport_user_proc)(long context);

/* ---------- prototypes/TRANSPORT.C */

void transport_initialize(void);
void transport_dispose(void);
void transport_unknown_0008d730(void);
void __attribute__((stdcall)) transport_register(transport_user_proc startup, transport_user_proc shutdown,
	transport_user_proc unknown, long context);
boolean transport_available(void);
void transport_global_update(void);
void transport_startup(void);
void transport_shutdown(void);

/* ---------- globals */

extern boolean unknown_004d8b1a;
extern long unknown_004d8b1c;
extern transport_user_proc unknown_004d8b20[MAXIMUM_TRANSPORT_USERS];
extern transport_user_proc unknown_004d8b40[MAXIMUM_TRANSPORT_USERS];
extern transport_user_proc unknown_004d8b60[MAXIMUM_TRANSPORT_USERS];
extern long unknown_004d8b80[MAXIMUM_TRANSPORT_USERS];

#endif
