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
	_network_link_channel_bit40= 0x40,
	/* more of it: a byte that says that it has its sockets, and the four of
	them. The bit of a socket that says that it is to be shut down before
	it is closed; and the reason that a channel of the list stops with when
	its byte says so */
	NETWORK_LINK_OPEN_OFFSET= 0x8,
	NETWORK_LINK_SOCKETS_OFFSET= 0xC,
	NUMBER_OF_NETWORK_LINK_SOCKETS= 4,
	_network_link_socket_connected_bit= 0x1,
	_network_channel_closed_unknown9= 9,
	/* of a packet: what its bytes and its voice can be; the kind whose
	bytes are sent as they are, with no voice; and what the network puts
	before a packet on the wire, for that kind and for the others */
	NETWORK_LINK_PACKET_DATA_SIZE= 0x600,
	NETWORK_LINK_PACKET_VOICE_SIZE= 0x200,
	_network_link_packet_kind3= 3,
	NETWORK_LINK_PACKET_KIND3_WIRE_SIZE= 0x2C,
	NETWORK_LINK_PACKET_WIRE_SIZE= 0x2D
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

/* one of its sockets, in memory of its own: the socket of the console's
libraries, or NONE, and its bits */
struct network_link_socket
{
	long socket;
	word flags;
};

/* a packet that it sends or has read: its kind, which is the socket that
it is of; a number; the address that it goes to or came from; and its bytes
and the ones of the voice, each after their size */
struct network_link_packet
{
	long kind;
	long unknown4;
	struct transport_address address;
	long size;
	byte data[NETWORK_LINK_PACKET_DATA_SIZE];
	long voice_size;
	byte voice[NETWORK_LINK_PACKET_VOICE_SIZE];
};

typedef char verify_network_link_packet_size[sizeof(struct network_link_packet) == 0x824 ? 1 : -1];

/* ---------- prototypes/NETWORK_LINK.C */

boolean __attribute__((stdcall)) network_link_add_channel(byte *link, long channel_index, long identifier,
	const struct transport_address *address);
boolean network_link_remove_channel(byte *link, long channel_index);
long __attribute__((stdcall)) network_link_find_channel(const byte *link, long kind,
	const struct transport_address *address);
long network_link_channel_of_address(const byte *link, const struct transport_address *address, long kind);
void __attribute__((stdcall)) network_link_close_channels(byte *link);
void __attribute__((stdcall)) network_link_close_sockets(byte *link);
void __attribute__((stdcall)) network_link_update_channels(byte *link);
void network_link_packet_pack(const struct network_link_packet *packet, byte *bytes, long *size_out);
boolean __attribute__((stdcall)) network_link_packet_unpack(long size, struct network_link_packet *packet,
	const byte *bytes);
long network_link_packet_wire_size(const struct network_link_packet *packet);

#endif
