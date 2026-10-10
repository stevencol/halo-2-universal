/*
NETWORK_CONNECTION.H

What a channel of the game has of its own to keep count of the packets that
it sends and of their answers: the things of unknown_004d87d8. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __NETWORK_CONNECTION_H
#define __NETWORK_CONNECTION_H

#include "cseries/cseries.h"
#include "networking/bitstream.h"

/* ---------- constants */

enum
{
	/* of one of them: the most that a number of what was answered has been;
	the three numbers of the last answer; the least that an answer has
	taken; how long an answer takes, smoothed, and how much that changes;
	how long is waited for one; and a number that goes up with each packet
	that is not answered and down with each that is */
	NETWORK_CONNECTION_UNKNOWN94C_OFFSET= 0x94C,
	NETWORK_CONNECTION_LAST_UNKNOWN_OFFSET= 0x95C,
	NETWORK_CONNECTION_LAST_TIME_OFFSET= 0x960,
	NETWORK_CONNECTION_LAST_UNKNOWN2_OFFSET= 0x964,
	NETWORK_CONNECTION_LEAST_TIME_OFFSET= 0x968,
	NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET= 0x96C,
	NETWORK_CONNECTION_TIME_DEVIATION_OFFSET= 0x970,
	NETWORK_CONNECTION_TIMEOUT_OFFSET= 0x974,
	NETWORK_CONNECTION_LOSS_OFFSET= 0x978,
	/* the kinds of thing that can happen to a packet, as a channel sees
	them: 1, 4 and 5 are answers, and 3 is that there was none */
	_network_connection_event1= 1,
	_network_connection_event_lost= 3,
	_network_connection_event4= 4,
	_network_connection_event5= 5,
	/* more of one of them: what it knows of the packets that came, and a
	word of each; what it knows of the ones that it sent, and what it keeps
	of each; and the bytes that it has sent */
	NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET= 0xC,
	NETWORK_CONNECTION_SENT_WINDOW_OFFSET= 0x28,
	NETWORK_CONNECTION_SENT_BYTES_OFFSET= 0x48,
	NETWORK_CONNECTION_RECEIVED_OFFSET= 0x4C,
	NETWORK_CONNECTION_SENT_OFFSET= 0x14C,
	/* the bit of the word of a packet that says that it came */
	_network_connection_received_bit= 0x1,
	/* more: a byte that says that it could not give a number for a packet;
	the number of a packet that it sent, from which the others are counted;
	two numbers and two bytes; how far the numbers of the packets that it
	sends can be from that and from the most of what was answered; two of
	the bits of a packet that was sent, with which it is not waited for;
	and what is said of one that is not kept any more */
	NETWORK_CONNECTION_FULL_OFFSET= 0x5,
	NETWORK_CONNECTION_UNKNOWN44_OFFSET= 0x44,
	NETWORK_CONNECTION_UNKNOWN950_OFFSET= 0x950,
	NETWORK_CONNECTION_UNKNOWN954_OFFSET= 0x954,
	NETWORK_CONNECTION_UNKNOWN958_OFFSET= 0x958,
	NETWORK_CONNECTION_UNKNOWN959_OFFSET= 0x959,
	NETWORK_CONNECTION_SEQUENCE_RANGE= 0x80,
	_network_connection_sent_bit1= 0x1,
	_network_connection_sent_bit4= 0x4,
	_network_connection_event_dropped= 6,
	/* the other things that can happen to a packet: it was answered, in its
	turn (1), or in its turn after that was said before it (2); and it was
	answered before its turn (4). And the other bits of a packet that was
	sent: that was said before its turn; it was answered, or not, when its
	turn came (which are the two with which it is not waited for); and its
	turn is over */
	_network_connection_event_answered_late= 2,
	_network_connection_event_answered_early= 4,
	_network_connection_sent_bit2= 0x2,
	_network_connection_sent_bit8= 0x8,
	_network_connection_sent_bit10= 0x10,
	/* what is said of an answer that is of packets too long ago */
	_network_connection_acknowledge_too_old= 2,
	/* what is said of a packet that came: it is good, or it is not one to
	read (2); and the state of a stream that is only looked at */
	_network_connection_read_good= 0,
	_network_connection_read_no= 2,
	_network_connection_stream_look_only= 4,
	/* the bits of what a packet has of the thing: the low byte of a number
	of a packet; a number of seven; the place of one bit, and the most bits
	that are written one by one */
	NETWORK_CONNECTION_SEQUENCE_BITS= 8,
	NETWORK_CONNECTION_UNKNOWN_BITS= 7,
	NETWORK_CONNECTION_ONE_BIT_BITS= 3,
	NETWORK_CONNECTION_SHORT_BIT_COUNT= 8,
	NETWORK_CONNECTION_BIT_COUNT_BITS= 7,
	MAXIMUM_NETWORK_CONNECTION_BITS= 128
};

/* ---------- structures */

/* the numbers of some packets, one after the other, of which something is
kept in a ring: whether it is in use; how many places the ring has; the number of the last
packet, and of the one before the first; one more count of places; the
place of the first, and how many there are */
struct network_connection_window
{
	boolean enabled;
	long size;
	long newest;
	long oldest;
	long capacity;
	long start;
	long count;
};

/* the functions of one of the things, of which the second says whether it
cannot send now */
struct network_connection_functions
{
	void *unknown0;
	boolean (__attribute__((thiscall)) *cannot_send)(byte *connection, long unknown);
};

/* what is kept of a packet that was sent: when, of what size, a number, how
far its number is from the one that they are counted from, and its bits */
struct network_connection_sent_packet
{
	long time;
	long size;
	long unknown8;
	short unknownC;
	word flags;
};

/* ---------- prototypes/NETWORK_CONNECTION.C */

void network_connection_reset(byte *connection);
long __attribute__((stdcall)) network_connection_read(byte *connection, long *sequence, struct bitstream *stream);
long __attribute__((stdcall)) network_connection_receive_sequence(byte *connection, long sequence_byte,
	long *sequence, boolean numbered, long unknown);
long __attribute__((stdcall)) network_connection_acknowledge(byte *connection, long sequence_byte, long bit_count,
	const unsigned long *bits, boolean unknown);
long __attribute__((stdcall)) network_connection_next_event(byte *connection, long *sequence, long *size,
	long *unknown8);
boolean __attribute__((stdcall)) network_connection_next_result(byte *connection, long *event, long *sequence,
	long *size, long *unknown8);
long __attribute__((stdcall)) network_connection_new_sequence(byte *connection, long time);
boolean __attribute__((stdcall)) network_connection_drop_oldest_sent(byte *connection, boolean force, long *event,
	long *sequence);
void network_connection_window_advance(struct network_connection_window *window, long sequence);
struct network_connection_sent_packet *network_connection_get_sent_packet(long sequence, byte *connection);
void network_connection_set_sent_size(long sequence, byte *connection, long size);
void network_connection_received(long sequence, byte *connection);
void network_connection_advance_received(byte *connection);
void __attribute__((stdcall)) network_connection_update_times(long event, byte *connection, long time, long unknown2,
	long unknown);

#endif
