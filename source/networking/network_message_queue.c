/*
NETWORK_MESSAGE_QUEUE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a channel of the game has of its own to send messages that have to
come, and to put together the ones that come to it: the things of
unknown_004d87dc. The name of the file is a guess, and so are the names of
its functions.
*/

#include "network_message_queue.h"

/* ---------- code */

/* 0x000814c0: whether the channel of one of the things is to stop, and why:
it is, with 0xC for a reason, if the thing is someone's and could not go on.
It is the second function of the thing, which the table at 0x00450db8 has,
as the two below are, and Ghidra's list has none of them; this one is away
from the rest in the executable, next to network_connection_closes */
boolean __attribute__((stdcall)) network_message_queue_closes(
	const byte *queue,
	long *reason)
{
	if (queue[NETWORK_MESSAGE_QUEUE_USED_OFFSET] && queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET])
	{
		if (reason)
		{
			*reason= _network_message_queue_closed_full;
		}

		return TRUE;
	}

	return FALSE;
}

/* 0x00094df0: whether one of the things has something to send: one of the
messages that it keeps has the bit that says that it is to be sent now; and
then that is urgent. The third function. The game has the looking for what
is kept of a message written here, as network_connection_get_sent_packet has
it for a packet */
boolean __attribute__((stdcall)) network_message_queue_has_something(
	const byte *queue,
	boolean *urgent)
{
	const struct network_connection_window *window=
		(const struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET);
	long oldest= window->oldest;
	long newest= window->newest;
	long sequence;

	if (newest==oldest)
	{
		return FALSE;
	}
	for (sequence= oldest+1; sequence<=newest; sequence++)
	{
		const struct network_message_queue_message *message= NULL;

		if (sequence>oldest && sequence<=window->newest)
		{
			long place= (long)((unsigned long)sequence-(unsigned long)oldest-1+
				(unsigned long)(window->count!=0 ? window->start : NONE))%window->size;

			if (place!=NONE)
			{
				message= (const struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+place;
			}
		}
		if (message->flags&_network_message_queue_message_pending_bit)
		{
			*urgent= TRUE;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00094e80: how many bits one of the things needs in a packet, at the
least: two. The fourth function */
long __attribute__((stdcall)) network_message_queue_minimum_bits(
	const byte *queue,
	long available,
	long left)
{
	return NETWORK_MESSAGE_QUEUE_MINIMUM_BITS;
}
