/*
NETWORK_MESSAGE_GATEWAY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What sends the messages of the network game that do not go by a channel: it
gathers the ones for an address of the game in one packet, and sends it. The
name of the file is a guess (it is the one that Halo 3 gives what does this),
and so are the names of its functions.
*/

#include "network_message_gateway.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x00093100: what a stream wrote is sent to an address of the game, with
something; how much was sent goes where it is told, if anywhere */
void __attribute__((stdcall)) unknown_00093100(const struct transport_address *address, const struct bitstream *stream,
	long unknown8, long *sent);

/* ---------- code */

/* 0x0007b330: the packet with messages that has not been sent, if there is
one, is: one bit more ends what was written, which is as many bytes as its
bits take, and as many more as it takes for them to be so many at a time
(the number of 0x8 of the stream) */
void network_message_gateway_flush(
	byte *gateway)
{
	if (gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET])
	{
		struct bitstream *stream= (struct bitstream *)(gateway+NETWORK_MESSAGE_GATEWAY_STREAM_OFFSET);
		/* the game adds as the processor does */
		long position= (long)((unsigned long)stream->position+1);
		long size= (long)((unsigned long)position+7)/8;
		long over;

		stream->position= position;
		stream->size= size;
		over= size%stream->unknown8;
		if (over!=0)
		{
			stream->size= (long)((unsigned long)stream->unknown8-(unsigned long)over+(unsigned long)size);
		}
		stream->unknownC= _bitstream_unknownC_finished;
		unknown_00093100((const struct transport_address *)(gateway+NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET), stream,
			*(const long *)(gateway+NETWORK_MESSAGE_GATEWAY_UNKNOWN8_OFFSET), NULL);
		gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET]= FALSE;
	}
}

/* 0x0007b390: the packet with messages that has not been sent is, if it is
for an address of the game: one that is as long as its own, and the same */
void network_message_gateway_flush_address(
	const struct transport_address *address,
	byte *gateway)
{
	if (gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET])
	{
		const struct transport_address *pending=
			(const struct transport_address *)(gateway+NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET);

		if (pending->address_size>0 && pending->address_size==address->address_size &&
			memcmp(pending, address, pending->address_size)==0)
		{
			network_message_gateway_flush(gateway);
		}
	}
}

/* 0x000937e0: a message starts: its kind, in 8 bits, and its size, in 16.
The game has bitstream_write_integer_checked written here twice, and has
this and the next one far from the functions of the gateway */
void __attribute__((stdcall)) network_message_header_write(
	struct bitstream *stream,
	unsigned long type,
	unsigned long size)
{
	bitstream_write_integer_checked(type, NETWORK_MESSAGE_TYPE_BITS, stream);
	bitstream_write_integer_checked(size, NETWORK_MESSAGE_SIZE_BITS, stream);
}

/* 0x00093860: the kind and the size of the next message of a stream;
whether they are good: the stream has them whole, there is such a kind, and
the size is one that a message of that kind can be of */
boolean __attribute__((stdcall)) network_message_header_read(
	long *type,
	struct bitstream *stream,
	const struct network_message_type *types,
	long *size)
{
	long message_size;

	*type= (long)bitstream_read_integer(stream, NETWORK_MESSAGE_TYPE_BITS);
	message_size= (long)bitstream_read_integer(stream, NETWORK_MESSAGE_SIZE_BITS);
	*size= message_size;
	if (!bitstream_overflowed(stream) && *type>=0 && *type<NUMBER_OF_NETWORK_MESSAGE_TYPES)
	{
		const struct network_message_type *message_type= &types[*type];

		if (message_type->valid && message_size>=message_type->minimum_size && message_size<=message_type->maximum_size)
		{
			return !bitstream_overflowed(stream);
		}
	}

	return FALSE;
}

/* 0x0007b140: a message of a kind and a size is to go to an address of the
game; whether it is in the packet. If the packet that has not been sent is
for another address, it is sent first. With no packet, one starts, of 0x518
bytes, for the address. The message goes in after a bit that says that there
is one more: its kind and size, and what the function of its kind writes of
it. If it does not fit, it is taken back; the packet is sent, and the
message goes in a new one; and one that does not fit in a new packet does
not go. The game has several functions of the stream written here */
boolean __attribute__((stdcall)) network_message_gateway_send(
	byte *gateway,
	const struct transport_address *address,
	long type,
	long size,
	const void *data)
{
	struct bitstream *stream= (struct bitstream *)(gateway+NETWORK_MESSAGE_GATEWAY_STREAM_OFFSET);

	if (gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET] &&
		!transport_address_equal(address,
		(const struct transport_address *)(gateway+NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET), FALSE))
	{
		network_message_gateway_flush(gateway);
	}
	for (;;)
	{
		const struct network_message_type *types;
		boolean started= FALSE;

		if (!gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET])
		{
			stream->size= NETWORK_MESSAGE_GATEWAY_PACKET_SIZE;
			stream->data= (unsigned long *)(gateway+NETWORK_MESSAGE_GATEWAY_PACKET_OFFSET);
			stream->unknown8= 1;
			stream->unknownC= _bitstream_writing;
			memset(stream->data, 0, stream->size);
			stream->position= 0;
			stream->position_count= 0;
			stream->error= FALSE;
			stream->unknown2C= 0;
			stream->unknown30= 0;
			*(struct transport_address *)(gateway+NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET)= *address;
			gateway[NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET]= TRUE;
			started= TRUE;
		}
		stream->positions[stream->position_count]= stream->position;
		stream->position_count++;
		bitstream_write_bool(stream, TRUE);
		types= *(const struct network_message_type **)(gateway+NETWORK_MESSAGE_GATEWAY_TYPES_OFFSET);
		network_message_header_write(stream, type, size);
		types[type].encode(stream, size, data);
		/* the game adds as the processor does: there has to be room for the
		bit that ends the packet too */
		if ((long)((unsigned long)stream->position+1)<=(long)((unsigned long)stream->size<<3))
		{
			stream->position_count--;

			return TRUE;
		}
		bitstream_pop_position(stream, TRUE);
		if (started)
		{
			return FALSE;
		}
		network_message_gateway_flush(gateway);
	}
}

/* 0x000938e0: a message of a kind that came from an address of the game is
given to what it is for, by something */
void __attribute__((stdcall)) unknown_000938e0(long type, const void *data, void *unknown10,
	const struct transport_address *address);

/* 0x0007afd0: a packet that came from an address of the game is read;
whether it was good. It is not if it starts with the word of a build for
testing. While a bit says that there is one more message: its kind and size
are read, and what the function of its kind reads of it; and if the gateway
has something at 0x10, the message is given to what it is for
(unknown_000938e0). A message that is not good ends it, with the ones before
given already. The game has bitstream_begin_reading and bitstream_read_bool
written here */
boolean __attribute__((stdcall)) network_message_gateway_receive(
	byte *gateway,
	const struct transport_address *address,
	struct bitstream *stream)
{
	byte data[MAXIMUM_NETWORK_MESSAGE_SIZE];

	bitstream_begin_reading(stream);
	if (bitstream_overflowed(stream))
	{
		return FALSE;
	}
	while (bitstream_read_bool(stream))
	{
		const struct network_message_type *types=
			*(const struct network_message_type **)(gateway+NETWORK_MESSAGE_GATEWAY_TYPES_OFFSET);
		long type= NONE;
		long size= 0;
		void *unknown10;

		if (!network_message_header_read(&type, stream, types, &size))
		{
			return FALSE;
		}
		memset(data, 0, size);
		if (!types[type].decode(stream, size, data))
		{
			return FALSE;
		}
		unknown10= *(void **)(gateway+NETWORK_MESSAGE_GATEWAY_UNKNOWN10_OFFSET);
		if (unknown10)
		{
			unknown_000938e0(type, data, unknown10, address);
		}
	}
	stream->unknownC= _bitstream_unknownC_read;

	return TRUE;
}