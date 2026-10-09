/*
NETWORK_CHANNEL.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The channels of the game: what this console talks with another by, once the
observer has an address for it. The name of the file is a guess, and so are
the names of its functions.
*/

#include "network_channel.h"

#include "networking/network_message_gateway.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x00092e00: what a channel is of does not have its number in its list any
more; whether it had it */
boolean unknown_00092e00(byte *unknown0, long unknown44);

/* 0x00094bf0: one of the things of unknown_004d87dc is done with */
void unknown_00094bf0(byte *unknown14);

/* ---------- code */

/* what a channel's own thing of unknown_004d87d8 is, if the channel is in
its fifth state and has one */
static inline __attribute__((always_inline)) const byte *network_channel_unknown10(
	const byte *channel)
{
	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
		(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8))
	{
		return unknown_004d87d8+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*NETWORK_CHANNEL_UNKNOWN10_SIZE;
	}

	return NULL;
}

/* 0x000880b0: whether the bits that a console asks to be talked with are
ones that go together: none past the first eight; not the first two at once,
nor the seventh and the eighth; not the fourth with the third; one of the
fifth and the sixth only with the fourth; and the sixth only with the fifth */
boolean network_channel_flags_valid(
	unsigned long flags)
{
	boolean valid= !(flags&~0xFFu) && !((flags&0x1) && (flags&0x2)) && !((flags&0x40) && (flags&0x80));

	if (flags&0x4)
	{
		valid= valid && !(flags&0x8);
	}
	if (!(flags&0x8))
	{
		valid= valid && !(flags&0x30);
	}
	if (!(flags&0x10))
	{
		valid= valid && !(flags&0x20);
	}

	return valid;
}

/* 0x00088650: a channel stops talking, for a reason. If it was in its fifth
state, and it is not the other console that said so, the other console is
told (a message of the seventh kind, with the two numbers of what it talked
with and the reason). What the channel has at 0x3C is told; what the channel
is of does not have its number any more; the address that it talked with is
kept as the last one; and the channel is in its second state, with the
reason and with no numbers of what it talks with */
void network_channel_close(
	byte *channel,
	long reason)
{
	struct network_channel_unknown3c *unknown3c;

	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
		reason!=_network_channel_closed_by_other)
	{
		struct network_channel_closed_message message;

		message.unknown50= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET);
		message.unknown4c= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET);
		message.reason= reason;
		network_message_gateway_send(*(byte **)(channel+NETWORK_CHANNEL_GATEWAY_OFFSET),
			(const struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET), _network_message_connect_closed,
			sizeof(message), &message);
	}
	unknown3c= *(struct network_channel_unknown3c **)(channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
	if (unknown3c)
	{
		unknown3c->closed(unknown3c->unknown4);
	}
	unknown_00092e00(*(byte **)(channel+NETWORK_CHANNEL_UNKNOWN0_OFFSET),
		*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET));
	memcpy(channel+NETWORK_CHANNEL_LAST_ADDRESS_OFFSET, channel+NETWORK_CHANNEL_ADDRESS_OFFSET,
		sizeof(struct transport_address));
	*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state_closed;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN58_OFFSET)= reason;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET)= NONE;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET)= NONE;
}

/* 0x000886e0: a channel is not in use any more. One that is past its second
state stops talking first (network_channel_close, with 3); no observer has
it; and its things of unknown_004d87dc and unknown_004d87d8 are no one's */
void __attribute__((stdcall)) network_channel_release(
	byte *channel)
{
	long index;

	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state_closed)
	{
		network_channel_close(channel, _network_channel_closed_released);
	}
	*(long *)(channel+NETWORK_CHANNEL_OBSERVER_OFFSET)= 0;
	index= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET);
	if (index!=NONE)
	{
		byte *unknown14= unknown_004d87dc+index*NETWORK_CHANNEL_UNKNOWN14_SIZE;

		unknown_00094bf0(unknown14);
		unknown14[NETWORK_CHANNEL_UNKNOWN14_USED_OFFSET]= FALSE;
		*(long *)(unknown14+NETWORK_CHANNEL_UNKNOWN14_OWNER_OFFSET)= 0;
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)= NONE;
	}
	index= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET);
	if (index!=NONE)
	{
		byte *unknown10= unknown_004d87d8+index*NETWORK_CHANNEL_UNKNOWN10_SIZE;

		unknown10[NETWORK_CHANNEL_UNKNOWN10_USED_OFFSET]= FALSE;
		*(long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_OWNER_OFFSET)= 0;
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)= NONE;
	}
	*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state_none;
}

/* 0x00088d20: the six times of a channel are now */
void network_channel_reset_times(
	byte *channel)
{
	struct network_channel_time *times= (struct network_channel_time *)(channel+NETWORK_CHANNEL_TIMES_OFFSET);
	long index;

	for (index= 0; index<NUMBER_OF_NETWORK_CHANNEL_TIMES; index++)
	{
		times[index].time= network_time_since(0);
		times[index].frame= unknown_004e6398;
	}
}

/* 0x00088d70: one of the times of a channel is now */
void network_channel_set_time(
	long index,
	byte *channel)
{
	struct network_channel_time *times= (struct network_channel_time *)(channel+NETWORK_CHANNEL_TIMES_OFFSET);

	times[index].time= network_time_since(0);
	times[index].frame= unknown_004e6398;
}

/* 0x00088fd0: a number of what a channel has of its own, and nothing if it
has none: the observer takes it for the bytes that it sent and were not
answered */
long network_channel_unknown_00088fd0(
	const byte *channel)
{
	const byte *unknown10= network_channel_unknown10(channel);

	return unknown10 ? *(const long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN48_OFFSET) : 0;
}

/* 0x00089000: by how much one number of what a channel has of its own is
more than another, and nothing if it has none. The game adds and takes away
as the processor does */
long network_channel_unknown_00089000(
	const byte *channel)
{
	const byte *unknown10= network_channel_unknown10(channel);

	return unknown10 ? (long)(*(const unsigned long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN30_OFFSET)-
		*(const unsigned long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN44_OFFSET)) : 0;
}

/* 0x00089030: by how much one number of what a channel has of its own is
more than another, and 0x80 more; nothing if it has none */
long network_channel_unknown_00089030(
	const byte *channel)
{
	const byte *unknown10= network_channel_unknown10(channel);

	return unknown10 ? (long)(*(const unsigned long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN94C_OFFSET)-
		*(const unsigned long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN30_OFFSET)+0x80) : 0;
}

/* 0x00089070: how much a channel in its fifth state that has one of the
things of unknown_004d87dc has waiting to be sent, in units of four: by how
much one number of that is more than another, and 0x200 more, times 32;
nothing if it has none */
long network_channel_unknown_00089070(
	const byte *channel)
{
	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
		(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10))
	{
		const byte *unknown14= unknown_004d87dc+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)*
			NETWORK_CHANNEL_UNKNOWN14_SIZE;

		return (long)((*(const unsigned long *)(unknown14+NETWORK_CHANNEL_UNKNOWN14_UNKNOWN1C_OFFSET)-
			*(const unsigned long *)(unknown14+NETWORK_CHANNEL_UNKNOWN14_UNKNOWN18_OFFSET)+0x200)<<5);
	}

	return 0;
}

/* 0x00095580: a message of a kind and a size is put in what one of the
things of unknown_004d87dc has to send */
void __attribute__((stdcall)) unknown_00095580(byte *unknown14, long type, long size, const void *data);

/* 0x00089180: a channel says that it talks with the other console (a
message of the sixth kind, with the two numbers of what it talks with): with
one of the things of unknown_004d87dc, by it, and only with a byte; with
none, to the address, by what sends the messages that go by no channel */
void __attribute__((stdcall)) network_channel_send_established(
	byte *channel,
	boolean established)
{
	struct network_channel_established_message message;

	message.unknown50= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET);
	message.unknown4c= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET);
	if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10)
	{
		if (established)
		{
			unknown_00095580(unknown_004d87dc+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)*
				NETWORK_CHANNEL_UNKNOWN14_SIZE, _network_message_connect_established, sizeof(message), &message);
		}
	}
	else
	{
		network_message_gateway_send(*(byte **)(channel+NETWORK_CHANNEL_GATEWAY_OFFSET),
			(const struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET),
			_network_message_connect_established, sizeof(message), &message);
	}
}

/* 0x00088360: a channel is given the number of what it talks with, which
asked to be talked with. One in its third state is in the fourth, since
now, with that number and its six times now; and in the fifth if it has
none of the things of unknown_004d87dc. In any state it says that it talks
(network_channel_send_established), with whether it was in the third */
void __attribute__((stdcall)) network_channel_accept(
	byte *channel,
	long identifier)
{
	boolean established= FALSE;

	if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state_connecting)
	{
		*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state4;
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN94_OFFSET)= network_time_since(0);
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET)= identifier;
		established= TRUE;
		network_channel_reset_times(channel);
		if (!(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10))
		{
			*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state5;
		}
	}
	network_channel_send_established(channel, established);
}

/* 0x000890b0: what is done each time with a channel that is trying to talk
with another console. After the time that it has for it (the third number
of what it has at 0xC) it stops, with 4 for a reason. If not, when it is
time for one more try, and it has not made as many as the second number
says, it asks again to be talked with (a message of the fourth kind, with
its number and its bits), and the next try is after the first number */
void network_channel_update_connecting(
	byte *channel)
{
	const struct network_channel_tries *tries;

	if (network_time_since(*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN88_OFFSET))>=
		(*(const struct network_channel_tries **)(channel+NETWORK_CHANNEL_TRIES_OFFSET))->timeout)
	{
		network_channel_close(channel, _network_channel_closed_timeout);

		return;
	}
	if (network_time_since(*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN8C_OFFSET))<0)
	{
		return;
	}
	tries= *(const struct network_channel_tries **)(channel+NETWORK_CHANNEL_TRIES_OFFSET);
	if (*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN90_OFFSET)<tries->count)
	{
		struct network_channel_request_message message;

		message.unknown4c= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET);
		message.flags= *(const unsigned long *)(channel+NETWORK_CHANNEL_UNKNOWN48_OFFSET);
		network_message_gateway_send(*(byte **)(channel+NETWORK_CHANNEL_GATEWAY_OFFSET),
			(const struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET), _network_message_connect_request,
			sizeof(message), &message);
		/* the game adds as the processor does */
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN8C_OFFSET)= (long)((unsigned long)(*(const struct network_channel_tries **)
			(channel+NETWORK_CHANNEL_TRIES_OFFSET))->period+(unsigned long)network_time_since(0));
	}
}
