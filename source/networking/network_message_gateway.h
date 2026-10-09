/*
NETWORK_MESSAGE_GATEWAY.H

What sends the messages of the network game that do not go by a channel: it
gathers the ones for an address of the game in one packet, and sends it. The
name of the file is a guess (it is the one that Halo 3 gives what does this),
and so are the names of its functions.
*/

#ifndef __NETWORK_MESSAGE_GATEWAY_H
#define __NETWORK_MESSAGE_GATEWAY_H

#include "cseries/cseries.h"
#include "networking/bitstream.h"
#include "networking/transport_security.h"

/* ---------- constants */

enum
{
	/* what the packet is sent with; whether there is a packet with messages
	that has not been sent; the address of the game that it is for; and
	what writes it, bit by bit */
	NETWORK_MESSAGE_GATEWAY_UNKNOWN8_OFFSET= 0x8,
	NETWORK_MESSAGE_GATEWAY_TYPES_OFFSET= 0xC,
	NETWORK_MESSAGE_GATEWAY_UNKNOWN10_OFFSET= 0x10,
	NETWORK_MESSAGE_GATEWAY_PACKET_OFFSET= 0x15,
	NETWORK_MESSAGE_GATEWAY_PACKET_SIZE= 0x518,
	NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET= 0x14,
	NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET= 0x618,
	NETWORK_MESSAGE_GATEWAY_STREAM_OFFSET= 0x62C,
	/* how many kinds of message there are, and in how many bits a message
	says its kind and its size */
	NUMBER_OF_NETWORK_MESSAGE_TYPES= 0x2D,
	NETWORK_MESSAGE_TYPE_BITS= 8,
	NETWORK_MESSAGE_SIZE_BITS= 16,
	/* what a stream that is done with is */
	_bitstream_unknownC_finished= 2,
	/* what a stream that was read to the end is, and the most that a message
	is when it is read */
	_bitstream_unknownC_read= 5,
	MAXIMUM_NETWORK_MESSAGE_SIZE= 0x10000
};

/* ---------- structures */

/* a kind of message: whether there is such a kind, the sizes that one can
be of, and the two functions that write one and read one */
struct network_message_type
{
	boolean valid;
	long unknown4;
	long unknown8;
	long minimum_size;
	long maximum_size;
	void (__attribute__((stdcall)) *encode)(struct bitstream *stream, long size, const void *data);
	boolean (__attribute__((stdcall)) *decode)(struct bitstream *stream, long size, void *data);
	long unknown1C;
};

typedef char verify_network_message_type_size[sizeof(struct network_message_type) == 0x20 || sizeof(void *) != 4 ? 1 : -1];

/* ---------- prototypes/NETWORK_MESSAGE_GATEWAY.C */

void network_message_gateway_flush(byte *gateway);
void network_message_gateway_flush_address(const struct transport_address *address, byte *gateway);
void __attribute__((stdcall)) network_message_header_write(struct bitstream *stream, unsigned long type,
	unsigned long size);
boolean __attribute__((stdcall)) network_message_gateway_send(byte *gateway, const struct transport_address *address,
	long type, long size, const void *data);
boolean __attribute__((stdcall)) network_message_gateway_receive(byte *gateway, const struct transport_address *address,
	struct bitstream *stream);
boolean __attribute__((stdcall)) network_message_header_read(long *type, struct bitstream *stream,
	const struct network_message_type *types, long *size);

#endif
