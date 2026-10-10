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
	_network_message_queue_message_pending_bit= 0x4
};

/* ---------- structures */

/* what is kept of a message that is to be sent: its bits, its size, a
number, where it is, and one more number */
struct network_message_queue_message
{
	byte flags;
	byte size;
	word unknown2;
	void *data;
	long unknown8;
};

/* ---------- prototypes/NETWORK_MESSAGE_QUEUE.C */

boolean __attribute__((stdcall)) network_message_queue_closes(const byte *queue, long *reason);
boolean __attribute__((stdcall)) network_message_queue_has_something(const byte *queue, boolean *urgent);
long __attribute__((stdcall)) network_message_queue_minimum_bits(const byte *queue, long available, long left);

#endif
