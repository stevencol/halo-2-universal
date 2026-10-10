/*
NETWORK_CONNECTION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a channel of the game has of its own to keep count of the packets that
it sends and of their answers: the things of unknown_004d87d8. The name of
the file is a guess, and so are the names of its functions.
*/

#include "network_connection.h"

/* ---------- globals */

/* 0x004cf6f8, 0x004cf6fc: by how many bits a new time is taken into the
smoothed one, and a new difference into how much it changes. 0x004cf700,
0x004cf704: what that is taken times for the wait, and the least that it
gives. 0x004cf708: the least wait. 0x004cf71c, 0x004cf720: what a packet
with no answer adds to the loss, and the most that that is. 0x004cf724: what
an answer takes from it */
long unknown_004cf6f8;
long unknown_004cf6fc;
long unknown_004cf700;
long unknown_004cf704;
long unknown_004cf708;
long unknown_004cf71c;
long unknown_004cf720;
long unknown_004cf724;

/* ---------- code */

/* 0x00096d80: one of the things of unknown_004d87d8 is told what became of
a packet. With no answer, its loss is more, up to the most. With an answer
that took a time: the three numbers of it are kept, the most of one and the
least time; the smoothed time goes toward the new one by a part of the
difference, and how much it changes toward the size of the difference, each
by a shift; the wait for an answer is the smoothed time and so many times
how much it changes, no less than two numbers; and the loss is less, down to
nothing. The game adds, takes and multiplies as the processor does, and
shifts by the low five bits */
void __attribute__((stdcall)) network_connection_update_times(
	long event,
	byte *connection,
	long time,
	long unknown2,
	long unknown)
{
	if (event==_network_connection_event1 || event==_network_connection_event4 || event==_network_connection_event5)
	{
		long smoothed= *(const long *)(connection+NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET);
		long deviation= *(const long *)(connection+NETWORK_CONNECTION_TIME_DEVIATION_OFFSET);
		long difference= (long)((unsigned long)time-(unsigned long)smoothed);
		long size= (long)(difference<0 ? 0u-(unsigned long)difference : (unsigned long)difference);
		long timeout;
		long loss;

		*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN_OFFSET)= unknown;
		if (unknown2>*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET))
		{
			*(long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET)= unknown2;
		}
		*(long *)(connection+NETWORK_CONNECTION_LAST_TIME_OFFSET)= time;
		if (*(const long *)(connection+NETWORK_CONNECTION_LEAST_TIME_OFFSET)>time)
		{
			*(long *)(connection+NETWORK_CONNECTION_LEAST_TIME_OFFSET)= time;
		}
		*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN2_OFFSET)= unknown2;
		smoothed= (long)((unsigned long)(difference>>(unknown_004cf6f8&31))+(unsigned long)smoothed);
		*(long *)(connection+NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET)= smoothed;
		deviation= (long)((unsigned long)((long)((unsigned long)size-(unsigned long)deviation)>>(unknown_004cf6fc&31))+
			(unsigned long)deviation);
		*(long *)(connection+NETWORK_CONNECTION_TIME_DEVIATION_OFFSET)= deviation;
		timeout= (long)((unsigned long)deviation*(unsigned long)unknown_004cf700);
		if (timeout<=unknown_004cf704)
		{
			timeout= unknown_004cf704;
		}
		timeout= (long)((unsigned long)timeout+(unsigned long)smoothed);
		if (timeout<=unknown_004cf708)
		{
			timeout= unknown_004cf708;
		}
		*(long *)(connection+NETWORK_CONNECTION_TIMEOUT_OFFSET)= timeout;
		loss= (long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)-(unsigned long)unknown_004cf724);
		*(long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)= loss>0 ? loss : 0;
	}
	else if (event==_network_connection_event_lost)
	{
		long loss= (long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)+(unsigned long)unknown_004cf71c);

		*(long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)= loss>unknown_004cf720 ? unknown_004cf720 : loss;
	}
}
