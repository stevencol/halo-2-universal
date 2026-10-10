/*
NETWORK_MESSAGE_QUEUE.H

What a channel of the game has of its own to send messages that have to
come, and to put together the ones that come to it: the things of
unknown_004d87dc. The name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __NETWORK_MESSAGE_QUEUE_H
#define __NETWORK_MESSAGE_QUEUE_H

#include "cseries/cseries.h"
#include "networking/bitstream.h"
#include "networking/network_connection.h"

/* ---------- constants */

enum
{
	/* of one of them: a byte that says that it is someone's, and one that it
	could not go on; what it knows of the messages that it has to send, and
	what it keeps of each; the reason that a channel stops with when its
	thing could not go on; the bits that it needs in a packet at the least;
	and the bit of a message that is to be sent now */
	NETWORK_MESSAGE_QUEUE_USED_OFFSET= 0x4,
	NETWORK_MESSAGE_QUEUE_FULL_OFFSET= 0x5,
	NETWORK_MESSAGE_QUEUE_SENT_WINDOW_OFFSET= 0x10,
	NETWORK_MESSAGE_QUEUE_SENT_OFFSET= 0x2C,
	_network_message_queue_closed_full= 0xC,
	NETWORK_MESSAGE_QUEUE_MINIMUM_BITS= 2,
	_network_message_queue_message_pending_bit= 0x4,
	/* more: what it knows of the messages that came, and what it keeps of
	each; the bytes of the messages that it keeps to send, and of the ones
	that came; and two bits of a message that came: it is there, and a byte
	that came with it */
	NETWORK_MESSAGE_QUEUE_RECEIVED_WINDOW_OFFSET= 0x182C,
	NETWORK_MESSAGE_QUEUE_RECEIVED_OFFSET= 0x1848,
	NETWORK_MESSAGE_QUEUE_SENT_BYTES_OFFSET= 0x2848,
	NETWORK_MESSAGE_QUEUE_RECEIVED_BYTES_OFFSET= 0x284C,
	_network_message_queue_received_bit= 0x1,
	_network_message_queue_received_sized_bit= 0x2,
	/* more bits of a message to send: it came there; it was sent, in a
	packet; and it has a number of bits of its own */
	_network_message_queue_message_delivered_bit= 0x1,
	_network_message_queue_message_sent_bit= 0x2,
	_network_message_queue_message_sized_bit= 0x8,
	/* what a packet has of a message: how far its number is from the one
	before it, less one, in so many bits, if it is no more than so far; or
	else the low bits of its number, which is taken for the one nearest the
	last that came, and is not to be further than so many from the ones that
	are kept; and its bits, less one, in so many, or else it has the most */
	NETWORK_MESSAGE_QUEUE_DISTANCE_BITS= 4,
	MAXIMUM_NETWORK_MESSAGE_QUEUE_DISTANCE= 0x10,
	NETWORK_MESSAGE_QUEUE_SEQUENCE_BITS= 10,
	NETWORK_MESSAGE_QUEUE_SEQUENCE_RANGE= 0x400,
	NETWORK_MESSAGE_QUEUE_SEQUENCE_REACH= 0x180,
	NETWORK_MESSAGE_QUEUE_BIT_COUNT_BITS= 8,
	MAXIMUM_NETWORK_MESSAGE_QUEUE_MESSAGE_BITS= 0x100,
	/* what reading says of a message whose number is from the one before
	it, with none before it */
	_network_message_queue_read_bad= 3,
	/* where one of them has the kinds of message; and the most bytes of
	what the function of a kind writes of a message */
	NETWORK_MESSAGE_QUEUE_TYPES_OFFSET= 0xC,
	MAXIMUM_NETWORK_MESSAGE_QUEUE_DATA_SIZE= 0xFFFF
};

/* ---------- structures */

/* what is kept of a message that is to be sent: its bits, its size in
bytes and in bits, where it is, and the number of the packet that it was
sent in, or NONE */
struct network_message_queue_message
{
	byte flags;
	byte size;
	word bit_count;
	void *data;
	long packet_sequence;
};

/* what is kept of a message that came: its bits, its size in bytes and in
bits, and where it is */
struct network_message_queue_received
{
	byte flags;
	byte size;
	word bit_count;
	void *data;
};

/* a message as it goes from one console to the other, in pieces: what the
function of its kind writes of it, after its kind and its size, and before
all that a checksum of it all */
struct network_message_queue_packed
{
	unsigned long checksum;
	byte data[MAXIMUM_NETWORK_MESSAGE_QUEUE_DATA_SIZE];
};

/* ---------- prototypes/NETWORK_MESSAGE_QUEUE.C */

void network_message_queue_reset(byte *queue);
struct network_message_queue_received *network_message_queue_get_received(long sequence, byte *queue);
boolean __attribute__((stdcall)) network_message_queue_add_received(long sequence, long size, byte *queue,
	boolean sized, word bit_count, const void *data);
boolean __attribute__((stdcall)) network_message_queue_write(byte *queue, long sequence, struct bitstream *stream,
	long bits, long rest);
long __attribute__((stdcall)) network_message_queue_read(byte *queue, const long *sequence_in,
	struct bitstream *stream);
void __attribute__((stdcall)) network_message_queue_packet_result(byte *queue, long sequence, boolean delivered);
void __attribute__((stdcall)) network_message_queue_send(byte *queue, long type, long size, const void *data);
boolean __attribute__((stdcall)) network_message_queue_receive(byte *queue, long *type, long *size, void *data);

boolean __attribute__((stdcall)) network_message_queue_closes(const byte *queue, long *reason);
boolean __attribute__((stdcall)) network_message_queue_has_something(const byte *queue, boolean *urgent);
long __attribute__((stdcall)) network_message_queue_minimum_bits(const byte *queue, long available, long left);

#endif
