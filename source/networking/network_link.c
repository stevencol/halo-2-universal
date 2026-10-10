/*
NETWORK_LINK.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What every channel of the game is of, and its list of them by address. The
name of the file is a guess, and so are the names of its functions.
*/

#include "network_link.h"

#include "networking/network_channel.h"
#include "networking/network_observer.h"

#include <string.h>

/* ---------- prototypes */

/* 0x003cd19e, 0x003cd193, 0x003cd718, of the console's libraries: a socket
is shut down, and closed; and why the last thing asked of them was not done.
0x003312fa and 0x002d1d3e: memory is let go of, and why not */
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
	struct network_link_socket **sockets= (struct network_link_socket **)(link+NETWORK_LINK_SOCKETS_OFFSET);
	long index;

	for (index= 0; index<NUMBER_OF_NETWORK_LINK_SOCKETS; index++)
	{
		struct network_link_socket *socket= sockets[index];

		if (socket)
		{
			if (socket->socket!=NONE && unknown_004d8b18 && unknown_004d8b19)
			{
				if (socket->flags&_network_link_socket_connected_bit)
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
