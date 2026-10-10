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

#include "networking/network_heap.h"

#include <string.h>

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

/* the place in its ring of what is kept of the message of a number, or NONE:
as the packets of network_connection.c */
static inline __attribute__((always_inline)) long network_message_queue_window_place(
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

/* 0x00094bf0: one of the things starts over. If it is someone's, the heap
of the network takes back the messages that it keeps to send, and the ones
that came and are there (of the ones to send, it is asked to even if there
is nothing of one). Then it has no messages of either, and no bytes of
them */
void network_message_queue_reset(
	byte *queue)
{
	struct network_connection_window *sent=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET);
	struct network_connection_window *received=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET);

	if (queue[NETWORK_MESSAGE_QUEUE_USED_OFFSET])
	{
		long sequence;

		if (sent->newest!=sent->oldest)
		{
			for (sequence= sent->oldest+1; sequence<=sent->newest; sequence++)
			{
				struct network_message_queue_message *message=
					(struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+
					network_message_queue_window_place(sent, sequence);
				struct network_heap *heap= unknown_004d87f8;
				void *data;
				long size;

				*(long *)(queue+NETWORK_MESSAGE_QUEUE_SENT_BYTES_OFFSET)-= message->size;
				data= message->data;
				heap->allocator->functions->size(heap->allocator, data, &size);
				heap= unknown_004d87f8;
				heap->allocator->functions->free(heap->allocator, data, NONE);
				if (data)
				{
					heap->count--;
				}
				message->data= NULL;
			}
		}
		if (received->newest!=received->oldest)
		{
			for (sequence= received->oldest+1; sequence<=received->newest; sequence++)
			{
				struct network_message_queue_received *message=
					(struct network_message_queue_received *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_OFFSET)+
					network_message_queue_window_place(received, sequence);

				if (message->data)
				{
					*(long *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET)-= message->size;
					network_heap_free(message->data);
					message->data= NULL;
				}
			}
		}
	}
	queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET]= FALSE;
	sent->newest= 0;
	sent->oldest= 0;
	sent->start= 0;
	sent->count= 0;
	sent->enabled= TRUE;
	received->newest= 0;
	received->oldest= 0;
	received->start= 0;
	received->count= 0;
	received->enabled= TRUE;
	*(long *)(queue+NETWORK_MESSAGE_QUEUE_SENT_BYTES_OFFSET)= 0;
	*(long *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET)= 0;
}

/* 0x00095b70: what one of the things keeps of the message that came with a
number, or NULL */
struct network_message_queue_received *network_message_queue_get_received(
	long sequence,
	byte *queue)
{
	long place= network_message_queue_window_place(
		(const struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET), sequence);

	return place==NONE ? NULL : (struct network_message_queue_received *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_OFFSET)+place;
}

/* 0x00095bc0: a piece of a message came, with a number, a size, a byte and
a kind: one of the things keeps a copy of it, in memory of the heap of the
network. Whether it is well: it is with one from before the ones that are
kept, and with one that is there already; it is not if there is no room for
its number, or no memory. The messages that the thing knows of go on up to
that one, as far as the ring has room, each new one with nothing */
boolean __attribute__((stdcall)) network_message_queue_add_received(
	long sequence,
	long size,
	byte *queue,
	boolean unknown,
	word type,
	const void *data)
{
	struct network_connection_window *received=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET);
	struct network_message_queue_received *message;
	void *memory;

	if (sequence<=received->oldest)
	{
		return TRUE;
	}
	while (received->newest<sequence && received->newest-received->oldest<received->size)
	{
		long next= (long)((unsigned long)received->newest+1);

		if (next>received->newest)
		{
			received->count= (long)((unsigned long)received->count+(unsigned long)next-(unsigned long)received->newest);
			received->newest= next;
		}
		message= (struct network_message_queue_received *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_OFFSET)+
			network_message_queue_window_place(received, next);
		memset(message, 0, sizeof(*message));
	}
	message= network_message_queue_get_received(sequence, queue);
	if (!message)
	{
		return FALSE;
	}
	if (message->flags&_network_message_queue_received_bit)
	{
		return TRUE;
	}
	memory= network_heap_allocate_block(size);
	if (!memory)
	{
		return FALSE;
	}
	message->flags|= _network_message_queue_received_bit;
	if (unknown)
	{
		message->flags|= _network_message_queue_received_unknown_bit;
	}
	else
	{
		message->flags&= ~_network_message_queue_received_unknown_bit;
	}
	message->data= memory;
	message->type= type;
	message->size= (byte)size;
	memcpy(memory, data, size);
	*(long *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET)+= size;

	return TRUE;
}
