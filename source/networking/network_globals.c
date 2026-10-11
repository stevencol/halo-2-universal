/*
NETWORK_GLOBALS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game does of the network each time around. The name of the file is
a guess, and so are the names of its functions.
*/

#include "network_globals.h"

#include "game/game.h"
#include "main/main_game.h"
#include "networking/game_voice.h"
#include "networking/network_blob.h"
#include "networking/network_link.h"
#include "networking/network_memory.h"
#include "networking/network_message_gateway.h"
#include "networking/network_observer.h"
#include "networking/network_session.h"
#include "networking/simulation_entity_database.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

/* ---------- prototypes */

/* 0x003314b0, of the console's libraries: the time since the console
started, in thousandths of a second */
unsigned long GetTickCount(void);

/* 0x0008d7e0: something of the network of the console is looked at, before
all else. The others are what the network does each time around, in order;
none of them is decompiled yet */
void unknown_0008d7e0(void);
void unknown_0007f070(void);
void unknown_000b2ea0(void);
void unknown_000b3200(void);
void unknown_00063d80(void);
void unknown_0006b4a0(void);
void unknown_00074aa0(void);
void unknown_000755d0(void);
void unknown_00080390(void);
void unknown_000b3de0(void);

/* ---------- globals */

/* 0x004d8ba0, 0x00510548, 0x0051054c, 0x00510560, of main.c: there is a
network; the time of the network is taken now, and which it is; and what
every channel is of */
extern boolean unknown_004d8ba0;
extern boolean unknown_00510548;
extern unsigned long unknown_0051054c;
extern long unknown_00510560;

/* 0x00510568, 0x00510570, 0x00510574: what sends the messages that go by no
channel, the observer, and the three sessions */
byte *unknown_00510568;
byte *unknown_00510570;
byte *unknown_00510574;

/* ---------- code */

/* 0x0008df50: what the network does each time around, after what came is
read: the network of the console is looked at; and, if there is a network,
ten things in order, of which the voice (game_voice_update) and the tasks of
the blocks (network_blob_tasks_update) are known. While it does them, the
time of the network is the one of when it started */
void network_update(
	void)
{
	unknown_0008d7e0();
	if (unknown_004d8ba0)
	{
		unknown_00510548= TRUE;
		unknown_0051054c= GetTickCount();
		unknown_0007f070();
		unknown_000b2ea0();
		unknown_000b3200();
		unknown_00063d80();
		unknown_0006b4a0();
		game_voice_update();
		unknown_00074aa0();
		unknown_000755d0();
		unknown_00080390();
		network_blob_tasks_update();
		unknown_000b3de0();
		unknown_00510548= FALSE;
		unknown_0051054c= GetTickCount();
	}
}

/* 0x0008dfc0: what the network sends, at the end of a time around: the
three sessions and the observer go on; the game in the network, if it is in
its fourth state and a game is on, sends what changed of the things that the
consoles share (simulation_entity_database_flush_updates), unless a byte
says not to; the channels go on (network_link_update_channels); and the
packet of the messages that go by no channel is sent
(network_message_gateway_flush). The game looks at the state twice, as two
things that it asks of it: that it is the fourth or the fifth, and that it
is not the third or the fifth */
void network_send(
	void)
{
	long index;

	if (!unknown_004d8ba0)
	{
		return;
	}
	unknown_00510548= TRUE;
	unknown_0051054c= GetTickCount();
	for (index= 0; index<NUMBER_OF_NETWORK_SESSIONS; index++)
	{
		network_session_update(unknown_00510574+index*NETWORK_SESSION_SIZE);
	}
	network_observer_update(unknown_00510570);
	if (!unknown_004cf770.unknown1[1])
	{
		const byte *world= unknown_004cf770.unknownC;
		long state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);

		if (state!=0 && main_game_globals && main_game_globals->active &&
			(state==_simulation_world_state4 || state==_simulation_world_state5) &&
			state!=_simulation_world_state3 && state!=_simulation_world_state5)
		{
			simulation_entity_database_flush_updates((struct simulation_entity_database *)
				(*(byte *const *)(world+SIMULATION_WORLD_UNKNOWN4_OFFSET)+NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWN2098_OFFSET));
		}
	}
	network_link_update_channels((byte *)unknown_00510560);
	network_message_gateway_flush(unknown_00510568);
	unknown_00510548= FALSE;
	unknown_0051054c= GetTickCount();
}

/* 0x0008e090: the sockets of the network are opened, if there is a network
and what every channel is of. What it is given is not looked at */
void __attribute__((stdcall)) network_open_sockets(
	long unknown)
{
	(void)unknown;
	if (unknown_004d8ba0)
	{
		if (unknown_00510560)
		{
			network_link_open_sockets((byte *)unknown_00510560);
		}
	}
}

/* 0x0008e0b0: the sockets of the network are closed, if there is a network:
every channel stops first; and a byte of the observer is clear. What it is
given is not looked at */
void __attribute__((stdcall)) network_close_sockets(
	long unknown)
{
	(void)unknown;
	if (unknown_004d8ba0)
	{
		if (unknown_00510560)
		{
			network_link_close_channels((byte *)unknown_00510560);
			network_link_close_sockets((byte *)unknown_00510560);
		}
		if (unknown_00510570)
		{
			unknown_00510570[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET]= FALSE;
		}
	}
}
