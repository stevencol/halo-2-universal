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
	NETWORK_MESSAGE_GATEWAY_PENDING_OFFSET= 0x14,
	NETWORK_MESSAGE_GATEWAY_ADDRESS_OFFSET= 0x618,
	NETWORK_MESSAGE_GATEWAY_STREAM_OFFSET= 0x62C,
	/* what a stream that is done with is */
	_bitstream_unknownC_finished= 2
};

/* ---------- prototypes/NETWORK_MESSAGE_GATEWAY.C */

void network_message_gateway_flush(byte *gateway);
void network_message_gateway_flush_address(const struct transport_address *address, byte *gateway);

#endif
