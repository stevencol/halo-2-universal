/*
NETWORK_CHANNEL.H

The channels of the game: what this console talks with another by, once the
observer has an address for it. The name of the file is a guess, and so are
the names of its functions.
*/

#ifndef __NETWORK_CHANNEL_H
#define __NETWORK_CHANNEL_H

#include "cseries/cseries.h"
#include "networking/bitstream.h"
#include "networking/network_observer.h"

/* ---------- constants */

enum
{
	/* of a channel: what it is of, which has a list of them; what sends its
	messages that go by no channel; its number in that list; the address of
	the game that it last talked with; and six times, each with the count
	of pictures that there was then */
	NETWORK_CHANNEL_UNKNOWN0_OFFSET= 0x0,
	NETWORK_CHANNEL_GATEWAY_OFFSET= 0x4,
	NETWORK_CHANNEL_UNKNOWN44_OFFSET= 0x44,
	NETWORK_CHANNEL_LAST_ADDRESS_OFFSET= 0x5C,
	NETWORK_CHANNEL_TIMES_OFFSET= 0x98,
	NUMBER_OF_NETWORK_CHANNEL_TIMES= 6,
	/* one more bit of its byte of 0x48: it has one of the things of
	unknown_004d87dc */
	_network_channel_unknown48_bit10= 0x10,
	/* of the thing of unknown_004d87d8 that is a channel's own: a byte and a
	number that say that it is someone's, and four numbers */
	NETWORK_CHANNEL_UNKNOWN10_USED_OFFSET= 0x4,
	NETWORK_CHANNEL_UNKNOWN10_OWNER_OFFSET= 0x8,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN30_OFFSET= 0x30,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN44_OFFSET= 0x44,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN48_OFFSET= 0x48,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN94C_OFFSET= 0x94C,
	/* and of the one of unknown_004d87dc: the same, and two numbers */
	NETWORK_CHANNEL_UNKNOWN14_USED_OFFSET= 0x4,
	NETWORK_CHANNEL_UNKNOWN14_OWNER_OFFSET= 0x8,
	NETWORK_CHANNEL_UNKNOWN14_UNKNOWN18_OFFSET= 0x18,
	NETWORK_CHANNEL_UNKNOWN14_UNKNOWN1C_OFFSET= 0x1C,
	/* the states of a channel: not in use, and not talking with anyone. The
	kind of message that says that a channel is not talking any more, and
	two of the reasons: the other console said so, and the channel is let
	go of */
	_network_channel_state_none= 0,
	_network_channel_state_closed= 2,
	_network_message_connect_closed= 7,
	_network_channel_closed_by_other= 6,
	_network_channel_closed_released= 3,
	/* more of a channel: its tries at talking with another console; when it
	started, when the next one is, and how many it has made; and when it
	was given what it talks with. The state in which it tries, the reason
	that it stops with when it has tried for too long, and the kinds of
	the messages that ask to be talked with and say that it is so */
	NETWORK_CHANNEL_TRIES_OFFSET= 0xC,
	NETWORK_CHANNEL_UNKNOWN88_OFFSET= 0x88,
	NETWORK_CHANNEL_UNKNOWN8C_OFFSET= 0x8C,
	NETWORK_CHANNEL_UNKNOWN90_OFFSET= 0x90,
	NETWORK_CHANNEL_UNKNOWN94_OFFSET= 0x94,
	_network_channel_state_connecting= 3,
	_network_channel_closed_timeout= 4,
	_network_message_connect_request= 4,
	_network_message_connect_established= 6,
	/* more of a channel: one more thing that it is given; what it has
	itself that is one of its parts; how many parts it has, and which; and a
	byte that it starts to talk with. Of what it is of: the next number that
	one of its channels gives what it talks with. Two more of its bits, and
	the two that it does not have with another console. Its first state,
	one more reason that it stops with, and the kinds of its parts */
	NETWORK_CHANNEL_UNKNOWN8_OFFSET= 0x8,
	NETWORK_CHANNEL_UNKNOWN18_OFFSET= 0x18,
	NETWORK_CHANNEL_PART_COUNT_OFFSET= 0x20,
	NETWORK_CHANNEL_PARTS_OFFSET= 0x24,
	MAXIMUM_NETWORK_CHANNEL_PARTS= 3,
	NETWORK_CHANNEL_UNKNOWN84_OFFSET= 0x84,
	NETWORK_CHANNEL_UNKNOWN0_NEXT_IDENTIFIER_OFFSET= 0x4,
	_network_channel_unknown48_bit20= 0x20,
	_network_channel_unknown48_bitsC0= 0xC0,
	_network_channel_state_new= 1,
	_network_channel_closed_unknown2= 2,
	_network_channel_part_unknown18= 0x1,
	_network_channel_part_unknown14= 0x19,
	_network_channel_part_unknown10= 0x31,
	/* one more number of the things of unknown_004d87dc, and where in what
	every channel is of it is given to point */
	NETWORK_CHANNEL_UNKNOWN14_UNKNOWNC_OFFSET= 0xC,
	UNKNOWN_00528000_UNKNOWN588_OFFSET= 0x588,
	/* how many things what a channel has at 0x3C can have, and what the
	index of a thing of a channel is made of */
	MAXIMUM_NETWORK_CHANNEL_UNKNOWN3C_PARTS= 4,
	NETWORK_CHANNEL_PART_INDEX_MASK= 0x7FFFFFFF,
	/* the kinds of thing that can happen to a channel's thing of
	unknown_004d87d8; which of the channel's times is the one of when
	something last came; two bits of the kinds of a channel's things; and
	one more number of the thing of unknown_004d87d8 */
	_network_channel_event1= 1,
	_network_channel_event2= 2,
	_network_channel_event3= 3,
	_network_channel_event4= 4,
	_network_channel_time_received= 4,
	_network_channel_part_bit4= 0x4,
	_network_channel_part_bit8= 0x8,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN968_OFFSET= 0x968,
	/* of what a channel has itself at 0x18, after its functions: the byte
	that it writes, and the one that what it reads has to be */
	NETWORK_CHANNEL_UNKNOWN18_UNKNOWN4_OFFSET= 0x4,
	NETWORK_CHANNEL_UNKNOWN18_UNKNOWN5_OFFSET= 0x5
};

/* the bit of the index of a thing of a channel that says that it is one of
what the channel has at 0x3C */
#define NETWORK_CHANNEL_PART_SECOND_BIT 0x80000000u

/* ---------- structures */

/* when something of a channel last happened: by the clock, and by the count
of pictures */
struct network_channel_time
{
	long time;
	long pad;
	long long frame;
};

typedef char verify_network_channel_time_size[sizeof(struct network_channel_time) == 0x10 ? 1 : -1];

/* what a channel says when it stops talking: the two numbers of what it
talked with, and why */
struct network_channel_closed_message
{
	long unknown50;
	long unknown4c;
	long reason;
};

/* one of the things that a channel has: of what kind, and which */
struct network_channel_part
{
	unsigned long type;
	void *part;
};

/* what a channel has at 0x3C: something that is told when the channel stops
talking, by a function and with a number, and that has things of its own
that go with the channel's; and two bytes */
struct network_channel_unknown3c
{
	boolean valid;
	long unknown4;
	void (__attribute__((stdcall)) *closed)(long unknown4);
	long part_count;
	struct network_channel_part parts[MAXIMUM_NETWORK_CHANNEL_UNKNOWN3C_PARTS];
	boolean unknown30;
	boolean unknown31;
};

typedef char verify_network_channel_unknown3c_size[sizeof(struct network_channel_unknown3c) == 0x34 || sizeof(void *) != 4 ? 1 : -1];

/* where one is in going through the things that a channel has: the bits
that are asked for, the index, a count of them all, and the thing: its
kind and which */
struct network_channel_part_iterator
{
	unsigned long mask;
	long index;
	long absolute;
	unsigned long type;
	void *part;
};

/* the tries of a channel at talking with another console: how long there
is between two, how many there are, and how long it tries for in all */
struct network_channel_tries
{
	long period;
	long count;
	long timeout;
};

/* what a channel asks to be talked with by: its number, and its bits */
struct network_channel_request_message
{
	long unknown4c;
	unsigned long flags;
};

/* what a channel says when it talks: the two numbers of what it talks
with */
struct network_channel_established_message
{
	long unknown50;
	long unknown4c;
};

/* ---------- prototypes/NETWORK_CHANNEL.C */

boolean network_channel_flags_valid(unsigned long flags);
void network_channel_close(byte *channel, long unknown);
void __attribute__((stdcall)) network_channel_release(byte *game_channel);
void network_channel_reset_times(byte *channel);
void network_channel_set_time(long index, byte *channel);
void __attribute__((stdcall)) network_channel_send_established(byte *channel, boolean established);
void __attribute__((stdcall)) network_channel_accept(byte *channel, long identifier);
void network_channel_update_connecting(byte *channel);
void __attribute__((stdcall)) network_channel_connect(const struct transport_address *address, byte *channel,
	boolean unknown);
boolean __attribute__((stdcall)) network_channel_new(unsigned long flags, long index, byte *unknown0, byte *gateway,
	byte *unknown8, const struct network_channel_tries *tries, byte *channel);
long __attribute__((stdcall)) network_channel_allocate(const char *name, long flags);
boolean __attribute__((stdcall)) network_channel_parts_next(const byte *channel,
	struct network_channel_part_iterator *iterator);
void __attribute__((stdcall)) network_channel_unknown3c_new(struct network_channel_unknown3c *unknown3c,
	boolean unknown30, void *const *parts, long unknown4, void (__attribute__((stdcall)) *closed)(long unknown4),
	long part_count, const unsigned long *types);
void network_channel_update_unknown10(byte *channel);
boolean __attribute__((stdcall)) network_channel_unknown18_write(const byte *unknown18, long unknown4,
	struct bitstream *stream, long unknownC, long unknown10);
long __attribute__((stdcall)) network_channel_unknown18_read(const byte *unknown18, long unknown4,
	struct bitstream *stream);
long __attribute__((stdcall)) network_channel_take_unknown10(long owner);
long __attribute__((stdcall)) network_channel_take_unknown14(long owner);
long network_channel_unknown_00088fd0(const byte *game_channel);
long network_channel_unknown_00089000(const byte *game_channel);
long network_channel_unknown_00089030(const byte *game_channel);
long network_channel_unknown_00089070(const byte *game_channel);

#endif
