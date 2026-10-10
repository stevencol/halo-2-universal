/*
NETWORK_LINK.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What every channel of the game is of, and its list of them by address. The
name of the file is a guess, and so are the names of its functions.
*/

#include "network_link.h"

#include "networking/game_voice.h"
#include "networking/network_channel.h"
#include "networking/network_observer.h"

#include <string.h>

/* ---------- prototypes */

/* 0x003cd19e, 0x003cd193, 0x003cd718, of the console's libraries: a socket
is shut down, and closed; and why the last thing asked of them was not done.
0x003312fa and 0x002d1d3e: memory is let go of, and why not */
/* 0x000b5110: bytes are sent by a socket to an address; how many were, or
less than nothing */
short __attribute__((stdcall)) unknown_000b5110(const struct transport_address *address,
	struct transport_endpoint *socket, const byte *bytes, long size);

/* 0x000b5060: bytes that came for a socket are taken, with the address that
they came from; how many, and not more than nothing if there are none */
short __attribute__((stdcall)) unknown_000b5060(struct transport_endpoint *socket, byte *bytes, long size,
	struct transport_address *address);

int __attribute__((stdcall)) shutdown(long socket, long how);
int __attribute__((stdcall)) closesocket(long socket);
int WSAGetLastError(void);
int VirtualFree(void *address, unsigned long size, unsigned long type);
unsigned long GetLastError(void);

/* ---------- globals */

/* 0x004d8b18, 0x004d8b19, of network_session.c: the network of the console
is there, and so is the one of the game */
extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

/* ---------- code */

/* 0x00092d10: the list has a channel, with the number that it gives what it
talks with and the address; whether it does. It does already if the channel
of that address, of the kind that this one is, is this one. If it is another,
that one is let go of (network_channel_release), but not if the two have the
same number at 0x4C: then the list does not take this one. Nor does it when
it is full */
boolean __attribute__((stdcall)) network_link_add_channel(
	byte *link,
	long channel_index,
	long identifier,
	const struct transport_address *address)
{
	struct network_link_channel *channels= (struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET);
	byte *channel= unknown_004d87d4+channel_index*NETWORK_CHANNEL_SIZE;
	long kind= _network_link_channel_kind_any;
	long other= NONE;
	long found;

	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state_closed)
	{
		if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit80)
		{
			kind= _network_link_channel_kind_bit80;
		}
		else if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit40)
		{
			kind= _network_link_channel_kind_bit40;
		}
	}
	found= network_link_find_channel(link, kind, address);
	if (found!=NONE)
	{
		other= channels[found].channel_index;
	}
	if (other==channel_index)
	{
		return TRUE;
	}
	if (other!=NONE)
	{
		byte *other_channel= unknown_004d87d4+other*NETWORK_CHANNEL_SIZE;

		if (*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET)==
			*(const long *)(other_channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET))
		{
			return FALSE;
		}
		network_channel_release(other_channel);
	}
	if (*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)>=MAXIMUM_NETWORK_LINK_CHANNELS)
	{
		return FALSE;
	}
	channels[*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)].channel_index= channel_index;
	channels[*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)].identifier= identifier;
	channels[*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)].unknown8= FALSE;
	channels[*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)].address= *address;
	(*(long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET))++;

	return TRUE;
}

/* 0x00092e00: the list does not have a channel any more; whether it had it.
The last of the list takes its place */
boolean network_link_remove_channel(
	byte *link,
	long channel_index)
{
	struct network_link_channel *channels= (struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET);
	long count= *(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET);
	long index;

	for (index= 0; index<*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET); index++)
	{
		if (channels[index].channel_index==channel_index)
		{
			*(long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)= count-1;
			if (index<count-1)
			{
				channels[index]= channels[count-1];
			}

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00092e50: where in the list the first channel of an address and a kind
is, or NONE. The address has to have a size and be the same in size and
bytes, as in transport_address_equal without the port; and the channel has
to talk (a state after the second) and have the bit of the kind, if a kind
is asked for */
long __attribute__((stdcall)) network_link_find_channel(
	const byte *link,
	long kind,
	const struct transport_address *address)
{
	const struct network_link_channel *channels= (const struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET);
	long count= *(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET);
	long index;

	for (index= 0; index<count; index++)
	{
		const struct transport_address *other= &channels[index].address;

		if (address->address_size>0 && address->address_size==other->address_size &&
			memcmp(address, other, address->address_size)==0)
		{
			const byte *channel= unknown_004d87d4+channels[index].channel_index*NETWORK_CHANNEL_SIZE;

			if (kind==_network_link_channel_kind_bit80)
			{
				if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state_closed &&
					(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit80))
				{
					return index;
				}
			}
			else if (kind==_network_link_channel_kind_bit40)
			{
				if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state_closed &&
					(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit40))
				{
					return index;
				}
			}
			else
			{
				return index;
			}
		}
	}

	return NONE;
}

/* 0x00092ef0: the number of the channel of an address and a kind, or NONE */
long network_link_channel_of_address(
	const byte *link,
	const struct transport_address *address,
	long kind)
{
	long found= network_link_find_channel(link, kind, address);

	return found!=NONE ? ((const struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET))[found].channel_index : NONE;
}

/* 0x00092f10: every channel of the list stops (network_channel_close, with
1), and the list has none */
void __attribute__((stdcall)) network_link_close_channels(
	byte *link)
{
	const struct network_link_channel *channels= (const struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET);
	long index;

	for (index= 0; index<*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET); index++)
	{
		network_channel_close(unknown_004d87d4+channels[index].channel_index*NETWORK_CHANNEL_SIZE, 1);
	}
	*(long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET)= 0;
}

/* 0x00092c70: its four sockets are closed, and it does not have them any
more. One that the libraries have is shut down first if its bit says so, and
closed, if the network is there; and the memory of each is let go of */
void __attribute__((stdcall)) network_link_close_sockets(
	byte *link)
{
	struct transport_endpoint **sockets= (struct transport_endpoint **)(link+NETWORK_LINK_SOCKETS_OFFSET);
	long index;

	for (index= 0; index<NUMBER_OF_NETWORK_LINK_SOCKETS; index++)
	{
		struct transport_endpoint *socket= sockets[index];

		if (socket)
		{
			if (socket->socket!=NONE && unknown_004d8b18 && unknown_004d8b19)
			{
				if (socket->flags&_transport_endpoint_connected_bit)
				{
					if (shutdown(socket->socket, 2))
					{
						WSAGetLastError();
					}
				}
				if (closesocket(socket->socket))
				{
					WSAGetLastError();
				}
			}
			socket->socket= NONE;
			socket->flags= 0;
			if (!VirtualFree(socket, 0, 0x8000))
			{
				GetLastError();
			}
			sockets[index]= NULL;
		}
	}
	link[NETWORK_LINK_OPEN_OFFSET]= FALSE;
}

/* 0x00093090: every channel of the list goes on (network_channel_update);
and one whose byte in the list says so stops, with 9 for a reason, if it
talks */
void __attribute__((stdcall)) network_link_update_channels(
	byte *link)
{
	struct network_link_channel *channels= (struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET);
	long index;

	for (index= 0; index<*(const long *)(link+NETWORK_LINK_CHANNEL_COUNT_OFFSET); index++)
	{
		byte *channel= unknown_004d87d4+channels[index].channel_index*NETWORK_CHANNEL_SIZE;

		network_channel_update(channel);
		if (channels[index].unknown8)
		{
			if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state_closed)
			{
				network_channel_close(channel, _network_channel_closed_unknown9);
			}
			channels[index].unknown8= FALSE;
		}
	}
}

/* 0x00093590: the bytes that are sent for a packet, and how many: its own
after their size in two bytes, and then the ones of the voice; but only its
own, as they are, if it is of the third kind */
void network_link_packet_pack(
	const struct network_link_packet *packet,
	byte *bytes,
	long *size_out)
{
	if (packet->kind!=_network_link_packet_kind3)
	{
		*(word *)bytes= (word)packet->size;
		memcpy(bytes+sizeof(word), packet->data, (unsigned long)packet->size);
		memcpy(bytes+sizeof(word)+packet->size, packet->voice, (unsigned long)packet->voice_size);
		*size_out= packet->voice_size+packet->size+(long)sizeof(word);
	}
	else
	{
		memcpy(bytes, packet->data, (unsigned long)packet->size);
		*size_out= packet->size;
	}
}

/* 0x00093610: a packet of a kind gets the bytes that came for it; whether
they are good. What is after the two bytes of the size and the bytes that it
says is the voice; neither can be more than a packet has room for. The third
kind has only its own bytes */
boolean __attribute__((stdcall)) network_link_packet_unpack(
	long size,
	struct network_link_packet *packet,
	const byte *bytes)
{
	if (packet->kind!=_network_link_packet_kind3)
	{
		if (size<(long)sizeof(word))
		{
			return FALSE;
		}
		packet->size= *(const word *)bytes;
		packet->voice_size= size-packet->size-(long)sizeof(word);
		if (packet->size<0 || (unsigned long)packet->size>NETWORK_LINK_PACKET_DATA_SIZE || packet->voice_size<0 ||
			(unsigned long)packet->voice_size>NETWORK_LINK_PACKET_VOICE_SIZE)
		{
			return FALSE;
		}
		memcpy(packet->data, bytes+sizeof(word), (unsigned long)packet->size);
		memcpy(packet->voice, bytes+sizeof(word)+packet->size, (unsigned long)packet->voice_size);
	}
	else
	{
		if ((unsigned long)size>NETWORK_LINK_PACKET_DATA_SIZE)
		{
			return FALSE;
		}
		packet->size= size;
		memcpy(packet->data, bytes, (unsigned long)size);
	}

	return TRUE;
}

/* 0x000936c0: how many bytes a packet is on the wire, for the counts of
what is sent and read: its own, made a multiple of eight, the ones of the
voice, and what the network puts before them */
long network_link_packet_wire_size(
	const struct network_link_packet *packet)
{
	long size= packet->size;

	if (size%8>0)
	{
		size+= 8-size%8;
	}

	return packet->voice_size+(packet->kind!=_network_link_packet_kind3 ? NETWORK_LINK_PACKET_WIRE_SIZE :
		NETWORK_LINK_PACKET_KIND3_WIRE_SIZE)+size;
}

/* 0x00093730: bytes are sent by one of its sockets to an address, at the
port of that socket. If they could not be, the channel of that address and
kind has its byte in the list, so that it stops (network_link_update_channels) */
void __attribute__((stdcall)) network_link_send_bytes(
	byte *link,
	long kind,
	const struct transport_address *address,
	long size,
	const byte *bytes)
{
	struct transport_address to= *address;
	struct transport_endpoint *socket;

	switch (kind)
	{
	case _network_link_packet_kind0:
		to.port= NETWORK_LINK_SOCKET0_PORT;
		break;
	case _network_link_packet_kind1:
		to.port= NETWORK_LINK_SOCKET1_PORT;
		break;
	case _network_link_packet_kind2:
		to.port= NETWORK_LINK_SOCKET2_PORT;
		break;
	case _network_link_packet_kind3:
		to.port= NETWORK_LINK_SOCKET3_PORT;
		break;
	}
	socket= ((struct transport_endpoint **)(link+NETWORK_LINK_SOCKETS_OFFSET))[kind];
	if (socket)
	{
		long sent= unknown_000b5110(&to, socket, bytes, size);

		if (sent!=size && sent==NONE)
		{
			long found= network_link_find_channel(link, kind, address);

			if (found!=NONE)
			{
				((struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET))[found].unknown8= TRUE;
			}
		}
	}
}

/* one more packet of a size in one of the counts */
static inline __attribute__((always_inline)) void network_link_rate_add(
	struct network_rate *rate,
	long size)
{
	network_rate_update(rate);
	rate->total_count++;
	rate->total_size+= size;
	rate->count++;
	rate->size+= (unsigned long)size;
}

/* 0x000932f0: a packet is sent: its bytes are made (network_link_packet_pack)
and, if they are some and not too many, sent by the socket of its kind. One
that does not go to this same console is counted, with its own size and with
the one that it has on the wire */
void __attribute__((stdcall)) network_link_send_packet(
	const struct network_link_packet *packet,
	byte *link)
{
	byte bytes[0x1000];
	long size;

	network_link_packet_pack(packet, bytes, &size);
	if (!(packet->address.address_size==sizeof(packet->address.address.ipv4) &&
		packet->address.address.ipv4==NETWORK_LINK_LOOPBACK_ADDRESS))
	{
		network_link_rate_add((struct network_rate *)(link+NETWORK_LINK_SENT_RATE_OFFSET), packet->size);
		network_link_rate_add((struct network_rate *)(link+NETWORK_LINK_SENT_WIRE_RATE_OFFSET),
			network_link_packet_wire_size(packet));
	}
	if (size<=MAXIMUM_NETWORK_LINK_SENT_SIZE && size>0)
	{
		network_link_send_bytes(link, packet->kind, &packet->address, size, bytes);
	}
}

/* 0x00093100: what a stream wrote is sent to an address, as a packet of the
third kind; what it is on the wire goes where it is told, if anywhere, and
nothing if it is more than a packet can have */
void __attribute__((stdcall)) network_link_send_to_address(
	const struct transport_address *address,
	const struct bitstream *stream,
	byte *link,
	long *wire_size_out)
{
	struct network_link_packet packet;
	long wire_size= 0;

	memset(&packet, 0, sizeof(packet));
	packet.address= *address;
	packet.kind= _network_link_packet_kind3;
	packet.size= stream->size;
	if ((unsigned long)packet.size<=NETWORK_LINK_PACKET_DATA_SIZE)
	{
		memcpy(packet.data, stream->data, (unsigned long)packet.size);
		network_link_send_packet(&packet, link);
		wire_size= network_link_packet_wire_size(&packet);
	}
	if (wire_size_out)
	{
		*wire_size_out= wire_size;
	}
}

/* 0x000931a0: what a stream wrote, if there is one, is sent to what a
channel talks with, with what the voice has for it; what it is on the wire
goes where it is told, if anywhere, and nothing if it was not sent. A channel
that is not in use or is new sends nothing. One that talks with this same
console sends by the second socket if it has the bit 0x40 of its byte of
0x48, and by the third if it has the bit 0x80; the others by the first */
void __attribute__((stdcall)) network_link_send_to_channel(
	long channel_index,
	const struct bitstream *stream,
	byte *link,
	long voice_size,
	const byte *voice,
	long *wire_size_out)
{
	const byte *channel= unknown_004d87d4+channel_index*NETWORK_CHANNEL_SIZE;
	struct network_link_packet packet;
	long wire_size= 0;
	long state;

	memset(&packet, 0, sizeof(packet));
	state= *(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET);
	if (state!=_network_channel_state_none && state!=_network_channel_state_new)
	{
		packet.address= *(const struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET);
		if (packet.address.address_size==sizeof(packet.address.address.ipv4) &&
			packet.address.address.ipv4==NETWORK_LINK_LOOPBACK_ADDRESS)
		{
			if (state<=_network_channel_state_closed)
			{
				goto done;
			}
			if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit40)
			{
				packet.kind= _network_link_packet_kind1;
			}
			else if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_link_channel_bit80)
			{
				packet.kind= _network_link_packet_kind2;
			}
			else
			{
				goto done;
			}
		}
		else
		{
			packet.kind= _network_link_packet_kind0;
		}
		if (stream)
		{
			packet.size= stream->size;
			if ((unsigned long)packet.size>NETWORK_LINK_PACKET_DATA_SIZE)
			{
				goto done;
			}
			memcpy(packet.data, stream->data, (unsigned long)packet.size);
		}
		if (voice_size>0)
		{
			packet.voice_size= voice_size;
			if ((unsigned long)voice_size>NETWORK_LINK_PACKET_VOICE_SIZE)
			{
				goto done;
			}
			memcpy(packet.voice, voice, (unsigned long)voice_size);
		}
		network_link_send_packet(&packet, link);
		wire_size= network_link_packet_wire_size(&packet);
	}
done:
	if (wire_size_out)
	{
		*wire_size_out= wire_size;
	}
}

/* 0x000933f0: a packet that came is given to who it is for. One that did not
come from this same console is counted, with its own size and with the one
that it has on the wire. One of the third kind goes, as a stream that reads
it, to what takes the ones that come by no channel. The others go to the
channel of their address and kind, if there is one: it reads the packet
(network_channel_read_packet), and the voice takes what is of it, with the
channel's time of when voice last came */
void __attribute__((stdcall)) network_link_receive_packet(
	const struct network_link_packet *packet,
	byte *link)
{
	long wire_size= network_link_packet_wire_size(packet);
	struct bitstream stream;

	if (!(packet->address.address_size==sizeof(packet->address.address.ipv4) &&
		packet->address.address.ipv4==NETWORK_LINK_LOOPBACK_ADDRESS))
	{
		network_link_rate_add((struct network_rate *)(link+NETWORK_LINK_RECEIVED_RATE_OFFSET), packet->size);
		network_link_rate_add((struct network_rate *)(link+NETWORK_LINK_RECEIVED_WIRE_RATE_OFFSET), wire_size);
	}
	if (packet->kind>_network_link_packet_kind2)
	{
		struct network_link_receiver *receiver;

		stream.data= (unsigned long *)packet->data;
		stream.size= packet->size;
		stream.unknown8= 1;
		stream.unknownC= 0;
		stream.position= 0;
		stream.position_count= 0;
		stream.error= FALSE;
		receiver= *(struct network_link_receiver **)(link+NETWORK_LINK_RECEIVER_OFFSET);
		if (receiver)
		{
			receiver->functions->receive(receiver, &packet->address, &stream);
		}
	}
	else
	{
		long found= network_link_find_channel(link, packet->kind, &packet->address);
		long channel_index;
		byte *channel;

		if (found==NONE)
		{
			return;
		}
		channel_index= ((const struct network_link_channel *)(link+NETWORK_LINK_CHANNELS_OFFSET))[found].channel_index;
		if (channel_index==NONE)
		{
			return;
		}
		channel= unknown_004d87d4+channel_index*NETWORK_CHANNEL_SIZE;
		if (packet->size>0)
		{
			stream.data= (unsigned long *)packet->data;
			stream.size= packet->size;
			stream.unknown8= 1;
			stream.unknownC= 0;
			stream.position= 0;
			stream.position_count= 0;
			stream.error= FALSE;
			network_channel_read_packet(&stream, channel, wire_size, FALSE);
		}
		if (packet->voice_size>0)
		{
			game_voice_read_packet(packet->voice, packet->voice_size);
			network_channel_set_time(_network_channel_time_voice_received, channel);
		}
	}
}

/* 0x00092f80: all that came for its sockets is read: while one of the four
has bytes from an address that is one (transport_address_valid), they are
made a packet of the kind of that socket (network_link_packet_unpack) and,
if they are good, given to who they are for (network_link_receive_packet) */
void __attribute__((stdcall)) network_link_receive(
	byte *link)
{
	struct transport_endpoint **sockets= (struct transport_endpoint **)(link+NETWORK_LINK_SOCKETS_OFFSET);
	boolean received;

	do
	{
		struct transport_address address;
		byte bytes[0x1000];
		long kind= 0;
		long size= 0;
		long index;

		received= FALSE;
		for (index= 0; index<NUMBER_OF_NETWORK_LINK_SOCKETS && !received; index++)
		{
			if (sockets[index])
			{
				long read= unknown_000b5060(sockets[index], bytes, sizeof(bytes), &address);

				if (read>0 && transport_address_valid(&address))
				{
					size= read;
					kind= index;
					received= TRUE;
				}
			}
		}
		if (received)
		{
			struct network_link_packet packet;

			memset(&packet, 0, sizeof(packet));
			packet.kind= kind;
			packet.address= address;
			if (network_link_packet_unpack(size, &packet, bytes))
			{
				network_link_receive_packet(&packet, link);
			}
		}
	} while (received);
}
