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

/* 0x00092d10: what a channel is of has, for the channel's number in its
list, the number that the channel gives what it talks with and the address;
whether it could. 0x00095cf0: one of the things of unknown_004d87d8 starts
over. 0x000820f0, 0x00082150: one of the things of unknown_004d87d8, and of
unknown_004d87dc, that is no one's is taken; which, or NONE */
boolean __attribute__((stdcall)) unknown_00092d10(byte *unknown0, long unknown44, long identifier,
	const struct transport_address *address);
void unknown_00095cf0(byte *unknown10);

/* ---------- globals */

/* 0x004d87d0: how many channels of the game there are. 0x004d8ba0, of
main.c: whether there are any */
long unknown_004d87d0;
extern boolean unknown_004d8ba0;

/* 0x00528000, 0x00528b28, 0x00529188, 0x004cf6d4: what every channel of the
game is of; what sends its messages that go by no channel, which is the
gateway; one more thing that it is given; and its tries at talking with
another console */
byte unknown_00528000[0xB28];
byte unknown_00528b28[0x660];
byte unknown_00529188[4];
struct network_channel_tries unknown_004cf6d4;

/* 0x00088220: a channel starts to talk with an address of the game: it is in
its third state, with the address and a byte; unless the address is this
console's own, it has not the seventh and eighth of its bits; and it gives
what it talks with a new number, the next one of what it is of. If that
cannot have it (unknown_00092d10), the channel stops, with 2 for a reason.
If not: with the byte, it starts to try, since now, and makes its first try
(network_channel_update_connecting); its six times are now; and its things
of unknown_004d87d8 and unknown_004d87dc start over */
void __attribute__((stdcall)) network_channel_connect(
	const struct transport_address *address,
	byte *channel,
	boolean unknown)
{
	byte *unknown0;
	long identifier;
	long index;

	*(struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET)= *address;
	*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state_connecting;
	channel[NETWORK_CHANNEL_UNKNOWN84_OFFSET]= unknown;
	if (!(address->address_size==sizeof(address->address.ipv4) && address->address.ipv4==NETWORK_OBSERVER_LOOPBACK_ADDRESS))
	{
		*(unsigned long *)(channel+NETWORK_CHANNEL_UNKNOWN48_OFFSET)&= ~(unsigned long)_network_channel_unknown48_bitsC0;
	}
	unknown0= *(byte **)(channel+NETWORK_CHANNEL_UNKNOWN0_OFFSET);
	identifier= *(const long *)(unknown0+NETWORK_CHANNEL_UNKNOWN0_NEXT_IDENTIFIER_OFFSET);
	/* the game adds as the processor does */
	*(long *)(unknown0+NETWORK_CHANNEL_UNKNOWN0_NEXT_IDENTIFIER_OFFSET)= (long)((unsigned long)identifier+1);
	if ((long)((unsigned long)identifier+1)==NONE)
	{
		*(long *)(unknown0+NETWORK_CHANNEL_UNKNOWN0_NEXT_IDENTIFIER_OFFSET)= 0;
	}
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET)= identifier;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET)= NONE;
	if (!unknown_00092d10(*(byte **)(channel+NETWORK_CHANNEL_UNKNOWN0_OFFSET),
		*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET), identifier,
		(const struct transport_address *)(channel+NETWORK_CHANNEL_ADDRESS_OFFSET)))
	{
		network_channel_close(channel, _network_channel_closed_unknown2);

		return;
	}
	if (channel[NETWORK_CHANNEL_UNKNOWN84_OFFSET])
	{
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN88_OFFSET)= network_time_since(0);
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN8C_OFFSET)= network_time_since(0);
		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN90_OFFSET)= 0;
		network_channel_update_connecting(channel);
	}
	network_channel_reset_times(channel);
	index= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET);
	if (index!=NONE)
	{
		unknown_00095cf0(unknown_004d87d8+index*NETWORK_CHANNEL_UNKNOWN10_SIZE);
	}
	index= *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET);
	if (index!=NONE)
	{
		unknown_00094bf0(unknown_004d87dc+index*NETWORK_CHANNEL_UNKNOWN14_SIZE);
	}
}

/* one more of the things that a channel has, of a kind */
static inline __attribute__((always_inline)) void network_channel_add_part(
	byte *channel,
	long type,
	void *part)
{
	struct network_channel_part *parts= (struct network_channel_part *)(channel+NETWORK_CHANNEL_PARTS_OFFSET);
	long count= *(const long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET);

	parts[count].part= part;
	parts[count].type= type;
	*(long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET)= (long)((unsigned long)count+1);
}

/* 0x00088110: a channel that was not in use is, in its first state: with
its bits, its number, the four things that it is given, and no address nor
numbers of what it talks with. For each of three of its bits it has one
more thing: one of unknown_004d87d8, one of unknown_004d87dc (each taken
now), and what it has itself at 0x18. If one cannot be taken, the channel is
let go of; whether it could */
boolean __attribute__((stdcall)) network_channel_new(
	unsigned long flags,
	long index,
	byte *unknown0,
	byte *gateway,
	byte *unknown8,
	const struct network_channel_tries *tries,
	byte *channel)
{
	*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state_new;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN58_OFFSET)= 0;
	memset(channel+NETWORK_CHANNEL_LAST_ADDRESS_OFFSET, 0, sizeof(struct transport_address));
	*(unsigned long *)(channel+NETWORK_CHANNEL_UNKNOWN48_OFFSET)= flags;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET)= index;
	*(byte **)(channel+NETWORK_CHANNEL_UNKNOWN0_OFFSET)= unknown0;
	*(byte **)(channel+NETWORK_CHANNEL_UNKNOWN8_OFFSET)= unknown8;
	*(byte **)(channel+NETWORK_CHANNEL_GATEWAY_OFFSET)= gateway;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET)= NONE;
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET)= NONE;
	*(const struct network_channel_tries **)(channel+NETWORK_CHANNEL_TRIES_OFFSET)= tries;
	*(long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET)= 0;
	memset(channel+NETWORK_CHANNEL_PARTS_OFFSET, 0, MAXIMUM_NETWORK_CHANNEL_PARTS*sizeof(struct network_channel_part));
	*(long *)(channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET)= 0;
	*(long *)(channel+NETWORK_CHANNEL_OBSERVER_OFFSET)= 0;
	if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8)
	{
		long taken= network_channel_take_unknown10(0);

		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)= taken;
		if (taken==NONE)
		{
			network_channel_release(channel);

			return FALSE;
		}
		network_channel_add_part(channel, _network_channel_part_unknown10, unknown_004d87d8+taken*NETWORK_CHANNEL_UNKNOWN10_SIZE);
	}
	if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10)
	{
		long taken= network_channel_take_unknown14(0);

		*(long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)= taken;
		if (taken==NONE)
		{
			network_channel_release(channel);

			return FALSE;
		}
		network_channel_add_part(channel, _network_channel_part_unknown14, unknown_004d87dc+taken*NETWORK_CHANNEL_UNKNOWN14_SIZE);
	}
	if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit20)
	{
		network_channel_add_part(channel, _network_channel_part_unknown18, channel+NETWORK_CHANNEL_UNKNOWN18_OFFSET);
	}

	return TRUE;
}

/* 0x00082060: a channel of the game that is not in use is taken, with some
bits; which, or NONE if there are no channels, none is free, or the first
free one cannot have what its bits ask for. The name that it is given is not
looked at. The game has it far from the other functions of the channels */
long __attribute__((stdcall)) network_channel_allocate(
	const char *name,
	long flags)
{
	long index;

	if (!unknown_004d8ba0 || unknown_004d87d0<=0)
	{
		return NONE;
	}
	for (index= 0; index<unknown_004d87d0; index++)
	{
		byte *channel= unknown_004d87d4+index*NETWORK_CHANNEL_SIZE;

		if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state_none)
		{
			return network_channel_new(flags, index, unknown_00528000, unknown_00528b28, unknown_00529188, &unknown_004cf6d4,
				channel) ? index : NONE;
		}
	}

	return NONE;
}

/* 0x000820f0: the first of the things of unknown_004d87d8 that is no one's
is someone's: it starts over, and has who; which, or NONE if there are no
channels or all are someone's. There are as many as channels. The game has
this and the next one far from the other functions of the channels */
long __attribute__((stdcall)) network_channel_take_unknown10(
	long owner)
{
	long index;

	if (!unknown_004d8ba0 || unknown_004d87d0<=0)
	{
		return NONE;
	}
	for (index= 0; index<unknown_004d87d0; index++)
	{
		byte *unknown10= unknown_004d87d8+index*NETWORK_CHANNEL_UNKNOWN10_SIZE;

		if (!unknown10[NETWORK_CHANNEL_UNKNOWN10_USED_OFFSET])
		{
			unknown_00095cf0(unknown10);
			*(long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_OWNER_OFFSET)= owner;
			unknown10[NETWORK_CHANNEL_UNKNOWN10_USED_OFFSET]= TRUE;

			return index;
		}
	}

	return NONE;
}

/* 0x00082150: the same with the things of unknown_004d87dc, which also have
a place of what every channel is of */
long __attribute__((stdcall)) network_channel_take_unknown14(
	long owner)
{
	long index;

	if (!unknown_004d8ba0 || unknown_004d87d0<=0)
	{
		return NONE;
	}
	for (index= 0; index<unknown_004d87d0; index++)
	{
		byte *unknown14= unknown_004d87dc+index*NETWORK_CHANNEL_UNKNOWN14_SIZE;

		if (!unknown14[NETWORK_CHANNEL_UNKNOWN14_USED_OFFSET])
		{
			unknown_00094bf0(unknown14);
			*(long *)(unknown14+NETWORK_CHANNEL_UNKNOWN14_OWNER_OFFSET)= owner;
			unknown14[NETWORK_CHANNEL_UNKNOWN14_USED_OFFSET]= TRUE;
			*(byte **)(unknown14+NETWORK_CHANNEL_UNKNOWN14_UNKNOWNC_OFFSET)= unknown_00528000+UNKNOWN_00528000_UNKNOWN588_OFFSET;

			return index;
		}
	}

	return NONE;
}

/* 0x000891e0: the next of the things that a channel has, of a kind: first
its own, and then the ones of what it has at 0x3C. The one after NONE is the
first. A thing is of the kind if its bits have all the ones that are asked
for. Whether there is one more; with none, the index is NONE again. The high
bit of the index says that the thing is one of the second ones; the other
number counts them all */
boolean __attribute__((stdcall)) network_channel_parts_next(
	const byte *channel,
	struct network_channel_part_iterator *iterator)
{
	long index= iterator->index;
	long absolute= iterator->absolute;
	const struct network_channel_part *part;

	for (;;)
	{
		const struct network_channel_unknown3c *unknown3c;
		const struct network_channel_part *parts= NULL;
		boolean first= FALSE;
		long count= 0;
		long place;

		part= NULL;
		if (index==NONE)
		{
			index= 0;
			absolute= 0;
			first= TRUE;
		}
		if (index>=0)
		{
			place= index&NETWORK_CHANNEL_PART_INDEX_MASK;
			if (!first)
			{
				/* the game adds as the processor does */
				place= (long)((unsigned long)place+1);
				index= place&NETWORK_CHANNEL_PART_INDEX_MASK;
				absolute= place;
				first= TRUE;
			}
			if (place>=0 && place<*(const long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET))
			{
				part= (const struct network_channel_part *)(channel+NETWORK_CHANNEL_PARTS_OFFSET)+place;
			}
			else
			{
				absolute= *(const long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET);
				index= (long)NETWORK_CHANNEL_PART_SECOND_BIT;
			}
		}
		if (index<0)
		{
			place= index&NETWORK_CHANNEL_PART_INDEX_MASK;
			if (!first)
			{
				place= (long)((unsigned long)place+1);
				index= (long)((unsigned long)place|NETWORK_CHANNEL_PART_SECOND_BIT);
				absolute= (long)(*(const unsigned long *)(channel+NETWORK_CHANNEL_PART_COUNT_OFFSET)+(unsigned long)place);
			}
			unknown3c= *(const struct network_channel_unknown3c **)(channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
			if (unknown3c)
			{
				count= unknown3c->part_count;
				parts= unknown3c->parts;
			}
			if (place<0 || place>=count)
			{
				index= NONE;
				absolute= NONE;
				break;
			}
			part= parts+place;
		}
		if ((part->type&iterator->mask)==iterator->mask)
		{
			break;
		}
	}
	iterator->index= index;
	iterator->absolute= absolute;
	if (index==NONE)
	{
		iterator->part= NULL;
		iterator->type= 0;

		return FALSE;
	}
	iterator->part= part->part;
	iterator->type= part->type;

	return TRUE;
}

/* 0x000892f0: what a channel can have at 0x3C is set up: with a byte, a
number, the function that is told when the channel stops talking, and so
many things, each of a kind */
void __attribute__((stdcall)) network_channel_unknown3c_new(
	struct network_channel_unknown3c *unknown3c,
	boolean unknown30,
	void *const *parts,
	long unknown4,
	void (__attribute__((stdcall)) *closed)(long unknown4),
	long part_count,
	const unsigned long *types)
{
	long index;

	unknown3c->unknown30= unknown30;
	unknown3c->unknown4= unknown4;
	unknown3c->valid= TRUE;
	unknown3c->closed= closed;
	unknown3c->part_count= part_count;
	for (index= 0; index<part_count; index++)
	{
		unknown3c->parts[index].type= types[index];
		unknown3c->parts[index].part= parts[index];
	}
	unknown3c->unknown31= FALSE;
}

/* 0x00096ce0: something that one of the things of unknown_004d87d8 does,
with two numbers that it gives, until it says no. 0x000965e0: the next thing
that happened to one of them, with three numbers; nothing when there is no
more */
boolean __attribute__((stdcall)) unknown_00096ce0(byte *unknown10, long unknown, long *unknown8, long *unknownC);
long __attribute__((stdcall)) unknown_000965e0(byte *unknown10, long *unknown4, long *unknown8, long *unknownC);

/* 0x004cf728, 0x004cf72c: what a number of a thing of unknown_004d87d8 is
taken times, and what it has more, for the most that something took to be
in time */
real unknown_004cf728;
long unknown_004cf72c;

/* a real as the whole number that it is nearest to: the game rounds with
the old processor's instructions, in place */
long lrintf(float value);

/* what a channel is of an observer by: something with functions, of which
the second is told that a packet came by a channel, the third that something
of a size did, and the fourth that something of a channel was answered, or
was not */
struct network_channel_owner;

struct network_channel_owner_functions
{
	void *unknown0;
	void (__attribute__((thiscall)) *read)(struct network_channel_owner *owner, long channel_index, long unknown);
	void (__attribute__((thiscall)) *received)(struct network_channel_owner *owner, long channel_index, long unknown4,
		long unknown8);
	void (__attribute__((thiscall)) *answered)(struct network_channel_owner *owner, long channel_index, boolean answered,
		boolean in_time);
};

struct network_channel_owner
{
	const struct network_channel_owner_functions *functions;
};

/* one of the things that a channel has: something with functions, of which
the sixth reads its own of a packet, and the seventh and the eighth are told
what happened to something of it */
struct network_channel_part_object;

struct network_channel_part_functions
{
	void *unknown0[5];
	long (__attribute__((thiscall)) *read)(struct network_channel_part_object *part, long *unknown4,
		struct bitstream *stream);
	void (__attribute__((thiscall)) *unknown18)(struct network_channel_part_object *part, long unknown);
	void (__attribute__((thiscall)) *unknown1c)(struct network_channel_part_object *part, long unknown, boolean answered);
};

struct network_channel_part_object
{
	const struct network_channel_part_functions *functions;
};

/* 0x00088db0: what is done each time with a channel that has one of the
things of unknown_004d87d8: that does what it does until it says no
(unknown_00096ce0), and each thing that happened to it is seen to
(unknown_000965e0), by its kind. With 1 and 4: the fifth time of the channel
is now; a channel in its fourth state is in the fifth; and the observer that
has the channel is told, with two of the numbers. With 4, each of the
channel's things of the kind 4 is told, with the other number. With 1, 2 and
3, each of the kind 8 is told, with that number and whether it is not 3; and
the observer is told that too, and whether it was in time: it was with 3,
and with the others if the last number is no less than what a number of the
thing gives */
void network_channel_update_unknown10(
	byte *channel)
{
	byte *unknown10;
	long unknown8;
	long unknownC;
	long unknown4;
	long scratch8;
	long scratchC;
	long kind;

	if (!(channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8))
	{
		return;
	}
	unknown10= unknown_004d87d8+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*NETWORK_CHANNEL_UNKNOWN10_SIZE;
	while (unknown_00096ce0(unknown10, 0, &scratch8, &scratchC))
	{
	}
	for (kind= unknown_000965e0(unknown10, &unknown4, &unknown8, &unknownC); kind!=0;
		kind= unknown_000965e0(unknown10, &unknown4, &unknown8, &unknownC))
	{
		struct network_channel_owner *owner;

		if (kind==_network_channel_event4 || kind==_network_channel_event1)
		{
			network_channel_set_time(_network_channel_time_received, channel);
			if (*(const long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state4)
			{
				*(long *)(channel+NETWORK_CHANNEL_STATE_OFFSET)= _network_channel_state5;
			}
			owner= *(struct network_channel_owner **)(channel+NETWORK_CHANNEL_OBSERVER_OFFSET);
			if (owner)
			{
				owner->functions->received(owner, *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET), unknown8,
					unknownC);
			}
		}
		if (kind==_network_channel_event4)
		{
			struct network_channel_part_iterator iterator= {_network_channel_part_bit4, NONE, NONE, 0, NULL};

			while (network_channel_parts_next(channel, &iterator))
			{
				struct network_channel_part_object *part= iterator.part;

				part->functions->unknown18(part, unknown4);
			}
		}
		else if (kind==_network_channel_event1 || kind==_network_channel_event2 || kind==_network_channel_event3)
		{
			struct network_channel_part_iterator iterator= {_network_channel_part_bit8, NONE, NONE, 0, NULL};
			boolean answered= kind!=_network_channel_event3;

			while (network_channel_parts_next(channel, &iterator))
			{
				struct network_channel_part_object *part= iterator.part;

				part->functions->unknown1c(part, unknown4, answered);
			}
			owner= *(struct network_channel_owner **)(channel+NETWORK_CHANNEL_OBSERVER_OFFSET);
			if (owner)
			{
				boolean in_time= TRUE;

				if (answered)
				{
					long unknown968= *(const long *)(unknown10+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN968_OFFSET);
					long most= lrintf((real)unknown968*unknown_004cf728);
					/* the game adds as the processor does */
					long other= (long)((unsigned long)unknown968+(unsigned long)unknown_004cf72c);

					if (most<=other)
					{
						most= other;
					}
					in_time= unknownC>=most;
				}
				owner->functions->answered(owner, *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET), answered,
					in_time);
			}
		}
	}
}

/* 0x00089340: what a channel has itself at 0x18 writes itself in a packet:
one bit, its byte of 0x4. It says FALSE. It is one of its functions, which
the table at 0x00450d50 has; the game has bitstream_write_bool written
here */
boolean __attribute__((stdcall)) network_channel_unknown18_write(
	const byte *unknown18,
	long unknown4,
	struct bitstream *stream,
	long unknownC,
	long unknown10)
{
	bitstream_write_bool(stream, unknown18[NETWORK_CHANNEL_UNKNOWN18_UNKNOWN4_OFFSET]);

	return FALSE;
}

/* 0x00089390: and reads itself from one: one bit, which is not good (1) if
it is not its byte of 0x5 */
long __attribute__((stdcall)) network_channel_unknown18_read(
	const byte *unknown18,
	long unknown4,
	struct bitstream *stream)
{
	return bitstream_read_bool(stream)!=unknown18[NETWORK_CHANNEL_UNKNOWN18_UNKNOWN5_OFFSET];
}
/* 0x00096860: one of the things of unknown_004d87d8 is told a number that
came in a packet. 0x00095840: the next message that one of the things of
unknown_004d87dc has whole, with its kind and its size; whether there is
one. 0x00093aa0: a message that came by a channel is given to what it is
for */
void unknown_00096860(long unknown, byte *unknown10);
boolean __attribute__((stdcall)) unknown_00095840(byte *unknown14, long *type, long *size, void *data);
void __attribute__((stdcall)) unknown_00093aa0(const void *data, long unknown44, byte *unknown8, long type, long size);

/* 0x00088750: a packet that came by a channel is read; whether it was good.
Each of the channel's things reads its own of it, by its sixth function,
which says how it went: well, well and that is all of the packet, or badly;
only the ones with the first bit, unless what the channel has at 0x3C has
its second byte, which the channel notes. After them, a bit says whether
there is more, of so many bits (0x3000 at the most), which is read and left.
A stream that is past its end, or that is of a build for testing, is bad.

A good packet, unless it is only looked at: the fourth time of the channel
is now; what happened to its thing of unknown_004d87d8 is seen to
(network_channel_update_unknown10), and that is told the number that the
things read; and each message that its thing of unknown_004d87dc has whole
now is given to what it is for (unknown_00093aa0). And the observer that has
the channel is told, by its second function, with the number that came with
the packet */
boolean __attribute__((stdcall)) network_channel_read_packet(
	struct bitstream *stream,
	byte *channel,
	long unknown,
	boolean look_only)
{
	byte data[MAXIMUM_NETWORK_MESSAGE_SIZE];
	byte rest[NETWORK_CHANNEL_PACKET_REST_SIZE];
	struct network_channel_part_iterator iterator;
	const struct network_channel_unknown3c *unknown3c;
	struct network_channel_owner *owner;
	long unknown10_number= NONE;
	long result= _network_channel_read_good;
	boolean good= FALSE;

	if (!look_only)
	{
		bitstream_begin_reading(stream);
	}
	unknown3c= *(const struct network_channel_unknown3c **)(channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
	if (unknown3c && unknown3c->unknown31)
	{
		iterator.mask= 0;
		channel[NETWORK_CHANNEL_UNKNOWN1D_OFFSET]= TRUE;
	}
	else
	{
		iterator.mask= _network_channel_part_bit1;
		channel[NETWORK_CHANNEL_UNKNOWN1D_OFFSET]= FALSE;
	}
	iterator.index= NONE;
	iterator.absolute= NONE;
	iterator.type= 0;
	iterator.part= NULL;
	while (result==_network_channel_read_good && network_channel_parts_next(channel, &iterator))
	{
		struct network_channel_part_object *part= iterator.part;

		result= part->functions->read(part, &unknown10_number, stream);
	}
	if (result==_network_channel_read_good && bitstream_read_bool(stream))
	{
		long bit_count= (long)bitstream_read_integer(stream, NETWORK_CHANNEL_PACKET_REST_BITS);

		if (bit_count<=0 || bit_count>NETWORK_CHANNEL_PACKET_REST_SIZE*8)
		{
			result= _network_channel_read_bad;
		}
		else
		{
			bitstream_read_bits(stream, rest, bit_count);
			if (bitstream_overflowed(stream))
			{
				result= _network_channel_read_bad;
			}
		}
	}
	if (bitstream_overflowed(stream))
	{
		result= _network_channel_read_bad;
	}
	if (result==_network_channel_read_good || result==_network_channel_read_end)
	{
		if (result==_network_channel_read_end)
		{
			/* the game multiplies as the processor does */
			stream->position= (long)((unsigned long)stream->size<<3);
		}
		stream->unknownC= _bitstream_unknownC_read;
		good= TRUE;
		if (!look_only)
		{
			network_channel_set_time(_network_channel_time_read, channel);
			network_channel_update_unknown10(channel);
			if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8)
			{
				unknown_00096860(unknown10_number, unknown_004d87d8+
					*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*NETWORK_CHANNEL_UNKNOWN10_SIZE);
			}
			if (channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit10)
			{
				byte *unknown14= unknown_004d87dc+*(const long *)(channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)*
					NETWORK_CHANNEL_UNKNOWN14_SIZE;
				long type;
				long size;

				while (unknown_00095840(unknown14, &type, &size, data))
				{
					unknown_00093aa0(data, *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET),
						*(byte **)(channel+NETWORK_CHANNEL_UNKNOWN8_OFFSET), type, size);
				}
			}
		}
	}
	owner= *(struct network_channel_owner **)(channel+NETWORK_CHANNEL_OBSERVER_OFFSET);
	if (owner && !look_only)
	{
		owner->functions->read(owner, *(const long *)(channel+NETWORK_CHANNEL_UNKNOWN44_OFFSET), unknown);
	}

	return good;
}
