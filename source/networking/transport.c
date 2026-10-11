/*
TRANSPORT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The network of the console: started when its cable is there and stopped when
it is not. The name of the file is a guess, and so are the names of its
functions.
*/

#include "transport.h"

#include "memory/data.h"
#include "networking/network_session.h"
#include "networking/transport_qos.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0036c47d, of the console's libraries: what the console says of its
cable. 0x003cd576, 0x003cd67b: the network of the console is started, and
stopped. 0x003cd58d, 0x003cd688: and its sockets. 0x003a0c39, 0x003a0d36:
and the service of games by the network. Each of the ones that start says
nothing if it did */
unsigned long XNetGetEthernetLinkStatus(void);
long __attribute__((stdcall)) XNetStartup(const struct transport_startup_parameters *parameters);
long XNetCleanup(void);
long __attribute__((stdcall)) WSAStartup(long version, byte *data);
long WSACleanup(void);
long __attribute__((stdcall)) XOnlineStartup(const long *parameters);
long XOnlineCleanup(void);

/* ---------- globals */

/* 0x004d8b1a: the network is started as far as the cable goes: it is there.
0x0055e704: what the console last said of its cable. 0x004d8b1c: how many
things are told that the network started or stopped. 0x004d8b20,
0x004d8b40: what each has for when it starts, and for when it stops.
0x004d8b80: and what each asked to be given */
boolean unknown_004d8b1a;
unsigned long unknown_0055e704;
long unknown_004d8b1c;
transport_user_proc unknown_004d8b20[MAXIMUM_TRANSPORT_USERS];
transport_user_proc unknown_004d8b40[MAXIMUM_TRANSPORT_USERS];
long unknown_004d8b80[MAXIMUM_TRANSPORT_USERS];

/* ---------- code */

/* 0x0008d7e0: the cable of the console is looked at, if the network of the
console is there: when it is put in, the network is started
(transport_startup), and when it is taken out, stopped (transport_shutdown).
While it is in, the console is asked what it is known by in the network
until it says (transport_security_get_address) */
void transport_global_update(
	void)
{
	boolean linked= unknown_004d8b18;

	if (unknown_004d8b18)
	{
		unsigned long status= XNetGetEthernetLinkStatus();

		if (unknown_0055e704!=status)
		{
			unknown_0055e704= status;
		}
		if (!(status&_transport_link_active_bit))
		{
			linked= FALSE;
		}
		if (unknown_004d8b1a!=linked)
		{
			unknown_004d8b1a= linked;
			if (linked)
			{
				transport_startup();
			}
			else
			{
				transport_shutdown();
			}
		}
		if (unknown_004d8b1a && !unknown_004cf791)
		{
			transport_security_get_address();
		}
	}
}

/* 0x0008d840: the network of the console is started, if it was not: the
network itself, with how much of each thing it is to have; its sockets; and
the service of games. If one cannot be, the ones before it are stopped. Then
the game has a network: what the console is known by is asked for, the table
of the attempts to measure is good for use, and each thing that asked to be
told is */
void transport_startup(
	void)
{
	struct transport_startup_parameters parameters;
	byte data[TRANSPORT_WINSOCK_DATA_SIZE];
	long online;
	long index;

	if (unknown_004d8b19)
	{
		return;
	}
	memset(data, 0, sizeof(data));
	memset(&parameters, 0, sizeof(parameters));
	parameters.size= sizeof(parameters);
	parameters.flags= 0;
	parameters.unknown2[0]= 12;
	parameters.unknown2[1]= 8;
	parameters.unknown2[2]= 4;
	parameters.unknown2[3]= 8;
	parameters.unknown2[4]= 24;
	parameters.unknown2[5]= 8;
	parameters.unknown2[6]= 8;
	parameters.unknown2[7]= 8;
	parameters.unknown2[8]= 40;
	parameters.unknown2[9]= 255;
	XNetGetEthernetLinkStatus();
	if (XNetStartup(&parameters))
	{
		return;
	}
	if (!WSAStartup(TRANSPORT_WINSOCK_VERSION, data))
	{
		online= 0;
		if (!XOnlineStartup(&online))
		{
			unknown_004d8b19= TRUE;
			transport_security_get_unknown7cc();
			transport_security_get_address();
			if (!unknown_004cf8d4)
			{
				data_make_valid(transport_qos_attempts);
				unknown_004cf8d4= TRUE;
			}
			for (index= 0; index<unknown_004d8b1c; index++)
			{
				if (unknown_004d8b20[index])
				{
					unknown_004d8b20[index](unknown_004d8b80[index]);
				}
			}

			return;
		}
		WSACleanup();
	}
	XNetCleanup();
}

/* 0x0008d970: the network of the console is stopped, if it was started:
each thing that asked to be told is; the measuring is done with
(transport_qos_dispose); the keys of the sessions that the console has are
taken from it; and the service of games, the sockets and the network itself
are stopped */
void transport_shutdown(
	void)
{
	long index;

	if (!unknown_004d8b19)
	{
		return;
	}
	for (index= 0; index<unknown_004d8b1c; index++)
	{
		if (unknown_004d8b40[index])
		{
			unknown_004d8b40[index](unknown_004d8b80[index]);
		}
	}
	transport_qos_dispose();
	for (index= 0; index<MAXIMUM_NETWORK_SESSION_KEYS; index++)
	{
		struct network_session_key *key= &unknown_004cf7d4[index];

		if (key->valid)
		{
			XNetUnregisterKey(key->identifier);
			key->valid= FALSE;
		}
	}
	unknown_004d8b19= FALSE;
	XOnlineCleanup();
	WSACleanup();
	XNetCleanup();
}
