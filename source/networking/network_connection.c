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

/* the place in its ring of what is kept of the packet of a number, or NONE
if the number is not one of the ones that there are. The game has this
written wherever it looks for one; with no packets it starts from before the
first place */
static inline __attribute__((always_inline)) long network_connection_window_place(
	const struct network_connection_window *window,
	long sequence)
{
	if (sequence>window->oldest && sequence<=window->newest)
	{
		return (long)((unsigned long)sequence-(unsigned long)window->oldest-1+
			(unsigned long)(window->count!=0 ? window->start : NONE))%window->size;
	}

	return NONE;
}

/* 0x000966d0: what one of the things keeps of the packet that it sent with
a number, or NULL */
struct network_connection_sent_packet *network_connection_get_sent_packet(
	long sequence,
	byte *connection)
{
	long place= network_connection_window_place(
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET), sequence);

	return place==NONE ? NULL : (struct network_connection_sent_packet *)(connection+NETWORK_CONNECTION_SENT_OFFSET)+place;
}

/* 0x00096810: the packet that one of the things sent with a number was of a
size, which is added to the bytes that it has sent. The game has
network_connection_get_sent_packet written here, and does not look whether
there is such a packet */
void network_connection_set_sent_size(
	long sequence,
	byte *connection,
	long size)
{
	network_connection_get_sent_packet(sequence, connection)->size= size;
	*(long *)(connection+NETWORK_CONNECTION_SENT_BYTES_OFFSET)+= size;
}

/* 0x00096860: the packet of a number came, as one of the things is told;
with NONE nothing is done. The packets that it knows of go on up to that
one, one at a time, each new one with nothing in its word; and the word of
that one has the bit that says that it came */
void network_connection_received(
	long sequence,
	byte *connection)
{
	struct network_connection_window *window=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	word *received= (word *)(connection+NETWORK_CONNECTION_RECEIVED_OFFSET);
	word *packet;

	if (sequence==NONE)
	{
		return;
	}
	while (sequence>window->newest)
	{
		long next= (long)((unsigned long)window->newest+1);

		if (next>window->newest)
		{
			window->count= (long)((unsigned long)window->count+(unsigned long)next-(unsigned long)window->newest);
			window->newest= next;
		}
		received[network_connection_window_place(window, window->newest)]= 0;
	}
	packet= &received[network_connection_window_place(window, sequence)];
	if (!(*packet&_network_connection_received_bit))
	{
		*packet|= _network_connection_received_bit;
	}
}

/* 0x00096c50: one of the things that is in use for packets that come does
not keep the first ones that came any more: while the one after the last
that it does not keep came, that is the last that it does not keep, and the
ring starts one place on */
void network_connection_advance_received(
	byte *connection)
{
	struct network_connection_window *window=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	const word *received= (const word *)(connection+NETWORK_CONNECTION_RECEIVED_OFFSET);

	if (!window->enabled)
	{
		return;
	}
	while (window->newest!=window->oldest)
	{
		long next= (long)((unsigned long)window->oldest+1);
		long count= (long)((unsigned long)next-(unsigned long)window->oldest);

		if (!(received[network_connection_window_place(window, next)]&_network_connection_received_bit))
		{
			break;
		}
		if (count>0)
		{
			if (window->count>=count)
			{
				window->start= (window->start+count)%window->capacity;
				window->count-= count;
			}
			window->oldest= next;
		}
	}
}
