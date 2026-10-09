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
