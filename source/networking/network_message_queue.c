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

#include "memory/crc.h"
#include "networking/network_heap.h"
#include "networking/network_message_gateway.h"

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

/* 0x00095bc0: a message came, with a number, a size in bytes, whether its
bits were said, and how many they are: one of the things keeps a copy of it,
in memory of the heap of the network. Whether it is well: it is with one from before the ones that are
kept, and with one that is there already; it is not if there is no room for
its number, or no memory. The messages that the thing knows of go on up to
that one, as far as the ring has room, each new one with nothing */
boolean __attribute__((stdcall)) network_message_queue_add_received(
	long sequence,
	long size,
	byte *queue,
	boolean sized,
	word bit_count,
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
	if (sized)
	{
		message->flags|= _network_message_queue_received_sized_bit;
	}
	else
	{
		message->flags&= ~_network_message_queue_received_sized_bit;
	}
	message->data= memory;
	message->bit_count= bit_count;
	message->size= (byte)size;
	memcpy(memory, data, size);
	*(long *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET)+= size;

	return TRUE;
}

/* 0x00094e90: one of the things writes its own in a packet of a number: the
messages that it keeps that are to be sent now, from the first, each after
keeping the position of the stream. Nothing with a packet of no number.

A message starts with two bits that say how its number comes: set and set,
it is the one after the message before it; set and clear, how far it is from
that one, less one, in four bits, if that is no more than 0x10; clear and
set, the low ten bits of the number, for the first one and for one further
than that. Then a bit that says whether its bits are said, and if so how
many, less one, in eight; and the bits of the message.

If after that the stream has room for what is to come after and two bits
more, the message is sent and not to be sent now, in that packet. If not,
the stream goes back to where it was before the message, no more are
written, and TRUE is what is said; and if it was the first one, the thing
could not go on. At the end come two bits clear.

The fifth function, with ones that Ghidra's list has not; the game has
bitstream_write_bool and bitstream_write_integer_checked written here */
boolean __attribute__((stdcall)) network_message_queue_write(
	byte *queue,
	long sequence,
	struct bitstream *stream,
	long bits,
	long rest)
{
	const struct network_connection_window *sent=
		(const struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET);
	boolean result= FALSE;
	long last= NONE;
	long current;

	if (sequence==NONE)
	{
		return FALSE;
	}
	if (sent->newest!=sent->oldest)
	{
		for (current= sent->oldest+1; current<=sent->newest; current++)
		{
			struct network_message_queue_message *message=
				(struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+
				network_message_queue_window_place(sent, current);
			long distance;

			if (!(message->flags&_network_message_queue_message_pending_bit))
			{
				continue;
			}
			stream->positions[stream->position_count]= stream->position;
			stream->position_count++;
			distance= (long)((unsigned long)current-(unsigned long)last);
			if (last!=NONE && distance<=MAXIMUM_NETWORK_MESSAGE_QUEUE_DISTANCE)
			{
				bitstream_write_bool(stream, TRUE);
				if (distance==1)
				{
					bitstream_write_bool(stream, TRUE);
				}
				else
				{
					bitstream_write_bool(stream, FALSE);
					bitstream_write_integer_checked((unsigned long)distance-1, NETWORK_MESSAGE_QUEUE_DISTANCE_BITS, stream);
				}
			}
			else
			{
				bitstream_write_bool(stream, FALSE);
				bitstream_write_bool(stream, TRUE);
				bitstream_write_integer_checked((unsigned long)current&(NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE-1),
					NETWORK_MESSAGE_QUEUE_SEQUENCE_BITS, stream);
			}
			if (message->flags&_network_message_queue_message_sized_bit)
			{
				bitstream_write_bool(stream, TRUE);
				bitstream_write_integer_checked((unsigned long)message->bit_count-1, NETWORK_MESSAGE_QUEUE_BIT_COUNT_BITS,
					stream);
			}
			else
			{
				bitstream_write_bool(stream, FALSE);
			}
			bitstream_write_bits(stream, message->data, message->bit_count);
			if ((long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)<(long)((unsigned long)rest+2))
			{
				if (last==NONE)
				{
					queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET]= TRUE;
				}
				result= TRUE;
				bitstream_pop_position(stream, TRUE);
				break;
			}
			stream->position_count--;
			message->flags= (byte)((message->flags&~_network_message_queue_message_pending_bit)|
				_network_message_queue_message_sent_bit);
			message->packet_sequence= sequence;
			last= current;
		}
	}
	bitstream_write_bool(stream, FALSE);
	bitstream_write_bool(stream, FALSE);

	return result;
}

/* 0x000951c0: one of the things reads its own of a packet that came: what
network_message_queue_write writes, message after message up to the two bits
clear. Nothing with a packet of no number. The sixth function, and what it
says is what network_connection_read does: well (0); that the packet is not
one to read (2), with a message whose number is further than 0x180 after the
last that came or before the first that is kept, or that the thing cannot
keep (network_message_queue_add_received); or 3, with a message whose number
is from the one before it and none before it.

A number that comes by its low ten bits is the one nearest the last that
came that has them. A message whose bits are not said has 0x100. In a stream
that is only looked at the messages are read and nothing is done with them */
long __attribute__((stdcall)) network_message_queue_read(
	byte *queue,
	const long *sequence_in,
	struct bitstream *stream)
{
	const struct network_connection_window *received=
		(const struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET);
	byte data[MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS/8];
	long last= NONE;

	if (*sequence_in==NONE)
	{
		return _network_connection_read_good;
	}
	for (;;)
	{
		boolean first= bitstream_read_bool(stream);
		boolean second= bitstream_read_bool(stream);
		long sequence= NONE;
		long bit_count;
		boolean sized;
		long size;

		if (first)
		{
			long distance= 0;

			if (!second)
			{
				distance= (long)bitstream_read_integer(stream, NETWORK_MESSAGE_QUEUE_DISTANCE_BITS);
			}
			if (stream->unknownC!=_network_connection_stream_look_only)
			{
				if (last==NONE)
				{
					return _network_message_queue_read_bad;
				}
				sequence= (long)((unsigned long)last+(unsigned long)distance+1);
			}
		}
		else
		{
			long difference;

			if (!second)
			{
				return _network_connection_read_good;
			}
			difference= (long)bitstream_read_integer(stream, NETWORK_MESSAGE_QUEUE_SEQUENCE_BITS);
			if (stream->unknownC!=_network_connection_stream_look_only)
			{
				difference= (long)((unsigned long)difference-
					(unsigned long)(received->newest&(NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE-1)));
				if (difference>NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE/2)
				{
					difference-= NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE;
				}
				else if (difference<=-NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE/2)
				{
					difference+= NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE;
				}
				sequence= (long)((unsigned long)received->newest+(unsigned long)difference);
				if ((long)((unsigned long)sequence-(unsigned long)received->newest)>NETWORK_MESSAGE_QUEUE_SEQUENCE_REACH ||
					(long)((unsigned long)received->oldest-(unsigned long)sequence)>NETWORK_MESSAGE_QUEUE_SEQUENCE_REACH)
				{
					return _network_connection_read_no;
				}
			}
		}
		last= sequence;
		sized= bitstream_read_bool(stream);
		if (sized)
		{
			bit_count= (long)(bitstream_read_integer(stream, NETWORK_MESSAGE_QUEUE_BIT_COUNT_BITS)+1);
			size= (bit_count+7)/8;
		}
		else
		{
			bit_count= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS;
			size= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS/8;
		}
		bitstream_read_bits(stream, data, bit_count);
		if (stream->unknownC==_network_connection_stream_look_only)
		{
			continue;
		}
		if (!network_message_queue_add_received(sequence, size, queue, sized, (word)bit_count, data))
		{
			return _network_connection_read_no;
		}
	}
}

/* 0x00095410: one of the things is told what became of the packet of a
number: each message that it keeps that was sent in that packet is in none
now, and it came there, or else it is to be sent now again. Then the first
messages that it keeps are not kept any more, while they are ones that came
there: the heap of the network takes each back (as
network_message_queue_reset does), its size is taken from the bytes of the
ones to send, and the ring starts one place on (the game has
network_connection_window_advance written here). The eighth function */
void __attribute__((stdcall)) network_message_queue_packet_result(
	byte *queue,
	long sequence,
	boolean delivered)
{
	struct network_connection_window *sent=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET);
	long current;

	if (sent->newest!=sent->oldest)
	{
		for (current= sent->oldest+1; current<=sent->newest; current++)
		{
			struct network_message_queue_message *message=
				(struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+
				network_message_queue_window_place(sent, current);

			if (message->packet_sequence==sequence)
			{
				message->packet_sequence= NONE;
				message->flags|= delivered ? _network_message_queue_message_delivered_bit :
					_network_message_queue_message_pending_bit;
			}
		}
	}
	while (sent->newest!=sent->oldest)
	{
		long next= sent->oldest+1;
		struct network_message_queue_message *message=
			(struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+
			network_message_queue_window_place(sent, next);
		struct network_heap *heap;
		void *data;
		long size;

		if (!(message->flags&_network_message_queue_message_delivered_bit))
		{
			break;
		}
		*(long *)(queue+NETWORK_MESSAGE_QUEUE_SENT_BYTES_OFFSET)-= message->size;
		data= message->data;
		heap= unknown_004d87f8;
		heap->allocator->functions->size(heap->allocator, data, &size);
		heap= unknown_004d87f8;
		heap->allocator->functions->free(heap->allocator, data, NONE);
		if (data)
		{
			heap->count--;
		}
		message->data= NULL;
		network_connection_window_advance(sent, next);
	}
}

/* 0x00095580: a message of a kind and a size is put in what one of the
things has to send. Its kind and size (network_message_header_write) and
what the function of its kind writes of it go in a stream of 0xFFFF bytes,
all zeros before; a checksum of it all goes before, which is counted with
itself as 0xFFFFFFFF first. That is cut in pieces of 0x20 bytes, and each
piece is one more message that the thing keeps to send, to be sent now: a
copy of it, in memory of the heap of the network. The last piece is the one
that has its number of bits; the others have 0x100.

Before each piece the thing is asked whether it cannot send, as its second
function says, and then no more is done. With no room for one more in the
ring, or no memory, the thing could not go on: that is noted, and it is
asked again.

The game has a stream started for writing and ended, and
network_heap_allocate, written here */
void __attribute__((stdcall)) network_message_queue_send(
	byte *queue,
	long type,
	long size,
	const void *data)
{
	struct network_connection_window *sent=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET);
	const struct network_message_type *types;
	struct network_message_queue_packed packed;
	struct bitstream stream;
	const byte *source;
	long remaining;

	stream.data= (unsigned long *)packed.data;
	stream.size= MAXIMUM_NETWORK_MESSAGE_QUEUE_DATA_SIZE;
	stream.unknown8= 1;
	stream.unknownC= _bitstream_writing;
	memset(packed.data, 0, sizeof(packed.data));
	stream.position= 0;
	stream.position_count= 0;
	stream.error= FALSE;
	stream.unknown2C= 0;
	stream.unknown30= 0;
	types= *(const struct network_message_type **)(queue+NETWORK_MESSAGE_QUEUE_TYPES_OFFSET);
	network_message_header_write(&stream, type, size);
	types[type].encode(&stream, size, data);
	remaining= stream.position+32;
	packed.checksum= 0xFFFFFFFF;
	crc_checksum_buffer(&packed, (remaining+7)/8, &packed.checksum);
	source= (const byte *)&packed;
	for (;;)
	{
		struct network_message_queue_message *message;
		long bit_count;
		boolean last;
		long piece;
		void *memory;
		long next;

		if ((*(const struct network_connection_functions **)queue)->cannot_send(queue, 0))
		{
			return;
		}
		if (sent->newest-sent->oldest>=sent->size)
		{
			queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET]= TRUE;
			continue;
		}
		if (remaining<=MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS)
		{
			bit_count= remaining;
			last= TRUE;
			piece= (remaining+7)/8;
		}
		else
		{
			bit_count= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS;
			last= FALSE;
			piece= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS/8;
		}
		memory= network_heap_allocate(piece);
		if (!memory)
		{
			queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET]= TRUE;
			continue;
		}
		next= sent->newest+1;
		if (next>sent->newest)
		{
			sent->count+= next-sent->newest;
			sent->newest= next;
		}
		message= (struct network_message_queue_message *)(queue+NETWORK_MESSAGE_QUEUE_SENT_OFFSET)+
			network_message_queue_window_place(sent, next);
		memcpy(memory, source, piece);
		memset(message, 0, sizeof(*message));
		message->size= (byte)piece;
		message->packet_sequence= NONE;
		message->flags= (byte)(_network_message_queue_message_pending_bit|
			(last ? _network_message_queue_message_sized_bit : 0));
		message->bit_count= (word)bit_count;
		message->data= memory;
		remaining-= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS;
		*(long *)(queue+NETWORK_MESSAGE_QUEUE_SENT_BYTES_OFFSET)+= piece;
		source+= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS/8;
		if (last)
		{
			return;
		}
	}
}

/* 0x00095840: the next message that one of the things has whole, with its
kind and its size; whether there is one. There is if, of the messages that
came, from the first that is kept on and with none missing, one has its
number of bits: the last piece.

The pieces up to that one are put together, each of 0x20 bytes but the last;
the heap of the network takes each back, its size is taken from the bytes of
the ones that came, and the ring starts after it. What they make starts with
a checksum, which has to be the one of it all (counted with itself as
0xFFFFFFFF); after it, read as a stream, come the kind and the size
(network_message_header_read) and what the function of the kind reads, into
the message, all zeros before. If the checksum is another, or the kind or
the size are not good, or the function says so, the thing could not go on,
and there is none.

The game has bitstream_begin_reading and network_connection_window_advance
written here */
boolean __attribute__((stdcall)) network_message_queue_receive(
	byte *queue,
	long *type,
	long *size,
	void *data)
{
	struct network_connection_window *received=
		(struct network_connection_window *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET);
	const struct network_message_type *types;
	struct network_message_queue_packed packed;
	struct bitstream stream;
	unsigned long checksum;
	boolean whole= FALSE;
	boolean last= FALSE;
	long sequence;
	long total= 0;

	if (received->newest==received->oldest)
	{
		return FALSE;
	}
	for (sequence= received->oldest+1; sequence<=received->newest; sequence++)
	{
		const struct network_message_queue_received *message= network_message_queue_get_received(sequence, queue);

		if (!message || !(message->flags&_network_message_queue_received_bit))
		{
			break;
		}
		if (message->flags&_network_message_queue_received_sized_bit)
		{
			whole= TRUE;
		}
	}
	if (!whole)
	{
		return FALSE;
	}
	stream.position= 0;
	stream.error= FALSE;
	stream.position_count= 0;
	stream.unknownC= 0;
	stream.unknown8= 1;
	stream.data= (unsigned long *)packed.data;
	stream.size= MAXIMUM_NETWORK_MESSAGE_QUEUE_DATA_SIZE;
	for (sequence= received->oldest+1; sequence<=received->newest && !last; sequence++)
	{
		struct network_message_queue_received *message=
			(struct network_message_queue_received *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_OFFSET)+
			network_message_queue_window_place(received, sequence);
		struct network_heap *heap;
		void *memory;
		long piece;
		long heap_size;

		if (message->flags&_network_message_queue_received_sized_bit)
		{
			piece= (message->bit_count+7)/8;
			last= TRUE;
		}
		else
		{
			piece= MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS/8;
		}
		memcpy((byte *)&packed+total, message->data, piece);
		total+= piece;
		memory= message->data;
		heap= unknown_004d87f8;
		heap->allocator->functions->size(heap->allocator, memory, &heap_size);
		heap= unknown_004d87f8;
		heap->allocator->functions->free(heap->allocator, memory, NONE);
		if (memory)
		{
			heap->count--;
		}
		*(long *)(queue+NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET)-= message->size;
		network_connection_window_advance(received, sequence);
	}
	checksum= packed.checksum;
	packed.checksum= 0xFFFFFFFF;
	crc_checksum_buffer(&packed, total, &packed.checksum);
	if (checksum==packed.checksum)
	{
		stream.size= total-(long)sizeof(packed.checksum);
		stream.data= (unsigned long *)packed.data;
		bitstream_begin_reading(&stream);
		types= *(const struct network_message_type **)(queue+NETWORK_MESSAGE_QUEUE_TYPES_OFFSET);
		if (network_message_header_read(type, &stream, types, size))
		{
			memset(data, 0, *size);
			if (types[*type].decode(&stream, *size, data))
			{
				return TRUE;
			}
		}
	}
	queue[NETWORK_MESSAGE_QUEUE_FULL_OFFSET]= TRUE;

	return FALSE;
}
