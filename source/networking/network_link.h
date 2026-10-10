/*
NETWORK_LINK.H

What every channel of the game is of (unknown_00528000): it has a list of
them, each with the address that it talks with, so that what comes from an
address can be given to its channel. The name of the file is a guess, and so
are the names of its functions.
*/

#ifndef __NETWORK_LINK_H
#define __NETWORK_LINK_H

#include "cseries/cseries.h"
#include "networking/transport_security.h"

/* ---------- constants */

enum
{
	/* of it: how many channels its list has, and the list */
	NETWORK_LINK_CHANNEL_COUNT_OFFSET= 0x20,
	NETWORK_LINK_CHANNELS_OFFSET= 0x24,
	MAXIMUM_NETWORK_LINK_CHANNELS= 16,
	/* the kinds of channel that one can be looked for by: any; one that
	talks and has the bit 0x80 of its byte of 0x48; and one that talks and
	has the bit 0x40 */
	_network_link_channel_kind_any= 0,
	_network_link_channel_kind_bit80= 1,
	_network_link_channel_kind_bit40= 2,
	_network_link_channel_bit80= 0x80,
	_network_link_channel_bit40= 0x40
};

/* ---------- structures */

/* one of the list: the number of the channel; the number that the channel
gives what it talks with; a byte; and the address */
struct network_link_channel
{
	long channel_index;
	long identifier;
	boolean unknown8;
	byte pad[3];
	struct transport_address address;
};

/* ---------- prototypes/NETWORK_LINK.C */

boolean __attribute__((stdcall)) network_link_add_channel(byte *link, long channel_index, long identifier,
	const struct transport_address *address);
boolean network_link_remove_channel(byte *link, long channel_index);
long __attribute__((stdcall)) network_link_find_channel(const byte *link, long kind,
	const struct transport_address *address);
long network_link_channel_of_address(const byte *link, const struct transport_address *address, long kind);
void __attribute__((stdcall)) network_link_close_channels(byte *link);

#endif
