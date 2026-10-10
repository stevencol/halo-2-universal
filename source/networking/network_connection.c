/*
NETWORK_CONNECTION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a channel of the game has of its own to keep count of the packets that
it sends and of their answers: the things of unknown_004d87d8. The name of
the file is a guess, and so are the names of its functions.
*/

#include "network_connection.h"

#include "networking/network_observer.h"

#include <string.h>

/* ---------- globals */

/* 0x004cf6f8, 0x004cf6fc: by how many bits a new time is taken into the
smoothed one, and a new difference into how much it changes. 0x004cf700,
0x004cf704: what that is taken times for the wait, and the least that it
gives. 0x004cf708: the least wait. 0x004cf71c, 0x004cf720: what a packet
with no answer adds to the loss, and the most that that is. 0x004cf724: what
an answer takes from it */
long unknown_004cf6f8;

/* 0x004cf6e8: what the time is taken times, in thousandths, for the number
of the first packet. 0x004cf6f4: what is waited for an answer more than the
wait. 0x004cf70c to 0x004cf718: what one of the things starts with: the least
time, the smoothed one, how much it changes, and the wait */
long unknown_004cf6e8;

/* 0x004cf6ec: how many packets before the ones that are kept an answer can
be of, and nothing be said of it */
long unknown_004cf6ec;

/* 0x004cf6f0: how many packets after one have to be answered for it to be
one that was not */
long unknown_004cf6f0;
long unknown_004cf6f4;
long unknown_004cf70c;
long unknown_004cf710;
long unknown_004cf714;
long unknown_004cf718;
long unknown_004cf6fc;
long unknown_004cf700;
long unknown_004cf704;
long unknown_004cf708;
long unknown_004cf71c;
long unknown_004cf720;
long unknown_004cf724;

/* ---------- code */

/* 0x00096d80: one of the things of unknown_004d87d8 is told what became of
a packet. With no answer, its loss is more, up to the most. With an answer
that took a time: the three numbers of it are kept, the most of one and the
least time; the smoothed time goes toward the new one by a part of the
difference, and how much it changes toward the size of the difference, each
by a shift; the wait for an answer is the smoothed time and so many times
how much it changes, no less than two numbers; and the loss is less, down to
nothing. The game adds, takes and multiplies as the processor does, and
shifts by the low five bits */
void __attribute__((stdcall)) network_connection_update_times(
	long event,
	byte *connection,
	long time,
	long unknown2,
	long unknown)
{
	if (event==_network_connection_event1 || event==_network_connection_event4 || event==_network_connection_event5)
	{
		long smoothed= *(const long *)(connection+NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET);
		long deviation= *(const long *)(connection+NETWORK_CONNECTION_TIME_DEVIATION_OFFSET);
		long difference= (long)((unsigned long)time-(unsigned long)smoothed);
		long size= (long)(difference<0 ? 0u-(unsigned long)difference : (unsigned long)difference);
		long timeout;
		long loss;

		*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN_OFFSET)= unknown;
		if (unknown2>*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET))
		{
			*(long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET)= unknown2;
		}
		*(long *)(connection+NETWORK_CONNECTION_LAST_TIME_OFFSET)= time;
		if (*(const long *)(connection+NETWORK_CONNECTION_LEAST_TIME_OFFSET)>time)
		{
			*(long *)(connection+NETWORK_CONNECTION_LEAST_TIME_OFFSET)= time;
		}
		*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN2_OFFSET)= unknown2;
		smoothed= (long)((unsigned long)(difference>>(unknown_004cf6f8&31))+(unsigned long)smoothed);
		*(long *)(connection+NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET)= smoothed;
		deviation= (long)((unsigned long)((long)((unsigned long)size-(unsigned long)deviation)>>(unknown_004cf6fc&31))+
			(unsigned long)deviation);
		*(long *)(connection+NETWORK_CONNECTION_TIME_DEVIATION_OFFSET)= deviation;
		timeout= (long)((unsigned long)deviation*(unsigned long)unknown_004cf700);
		if (timeout<=unknown_004cf704)
		{
			timeout= unknown_004cf704;
		}
		timeout= (long)((unsigned long)timeout+(unsigned long)smoothed);
		if (timeout<=unknown_004cf708)
		{
			timeout= unknown_004cf708;
		}
		*(long *)(connection+NETWORK_CONNECTION_TIMEOUT_OFFSET)= timeout;
		loss= (long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)-(unsigned long)unknown_004cf724);
		*(long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)= loss>0 ? loss : 0;
	}
	else if (event==_network_connection_event_lost)
	{
		long loss= (long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)+(unsigned long)unknown_004cf71c);

		*(long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)= loss>unknown_004cf720 ? unknown_004cf720 : loss;
	}
}

/* the place in its ring of what is kept of the packet of a number, or NONE
if the number is not one of the ones that there are. The game has this
written wherever it looks for one; with no packets it starts from before the
first place */
static inline __attribute__((always_inline)) long network_connection_window_place(
	const struct network_connection_window *window,
	long sequence)
{
	if (sequence>window->oldest && sequence<=window->newest)
	{
		return (long)((unsigned long)sequence-(unsigned long)window->oldest-1+
			(unsigned long)(window->count!=0 ? window->start : NONE))%window->size;
	}

	return NONE;
}

/* 0x000966d0: what one of the things keeps of the packet that it sent with
a number, or NULL */
struct network_connection_sent_packet *network_connection_get_sent_packet(
	long sequence,
	byte *connection)
{
	long place= network_connection_window_place(
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET), sequence);

	return place==NONE ? NULL : (struct network_connection_sent_packet *)(connection+NETWORK_CONNECTION_SENT_OFFSET)+place;
}

/* 0x00096810: the packet that one of the things sent with a number was of a
size, which is added to the bytes that it has sent. The game has
network_connection_get_sent_packet written here, and does not look whether
there is such a packet */
void network_connection_set_sent_size(
	long sequence,
	byte *connection,
	long size)
{
	network_connection_get_sent_packet(sequence, connection)->size= size;
	*(long *)(connection+NETWORK_CONNECTION_SENT_BYTES_OFFSET)+= size;
}

/* 0x00096860: the packet of a number came, as one of the things is told;
with NONE nothing is done. The packets that it knows of go on up to that
one, one at a time, each new one with nothing in its word; and the word of
that one has the bit that says that it came */
void network_connection_received(
	long sequence,
	byte *connection)
{
	struct network_connection_window *window=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	word *received= (word *)(connection+NETWORK_CONNECTION_RECEIVED_OFFSET);
	word *packet;

	if (sequence==NONE)
	{
		return;
	}
	while (sequence>window->newest)
	{
		long next= (long)((unsigned long)window->newest+1);

		if (next>window->newest)
		{
			window->count= (long)((unsigned long)window->count+(unsigned long)next-(unsigned long)window->newest);
			window->newest= next;
		}
		received[network_connection_window_place(window, window->newest)]= 0;
	}
	packet= &received[network_connection_window_place(window, sequence)];
	if (!(*packet&_network_connection_received_bit))
	{
		*packet|= _network_connection_received_bit;
	}
}

/* 0x00096c50: one of the things that is in use for packets that come does
not keep the first ones that came any more: while the one after the last
that it does not keep came, that is the last that it does not keep, and the
ring starts one place on */
void network_connection_advance_received(
	byte *connection)
{
	struct network_connection_window *window=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	const word *received= (const word *)(connection+NETWORK_CONNECTION_RECEIVED_OFFSET);

	if (!window->enabled)
	{
		return;
	}
	while (window->newest!=window->oldest)
	{
		long next= (long)((unsigned long)window->oldest+1);
		long count= (long)((unsigned long)next-(unsigned long)window->oldest);

		if (!(received[network_connection_window_place(window, next)]&_network_connection_received_bit))
		{
			break;
		}
		if (count>0)
		{
			if (window->count>=count)
			{
				window->start= (window->start+count)%window->capacity;
				window->count-= count;
			}
			window->oldest= next;
		}
	}
}

/* 0x00095cf0: one of the things starts over. The number of the packets that
it sends starts from the time, taken times a number, in thousandths and of
no more than a byte: no packets sent, with that number; none that came, and
that not in use; the most of what was answered is the number before; and its
times are the ones that the game has for the start, with no loss */
void network_connection_reset(
	byte *connection)
{
	struct network_connection_window *received=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	struct network_connection_window *sent=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	long sequence;

	connection[NETWORK_CONNECTION_FULL_OFFSET]= FALSE;
	sequence= (long)(((unsigned long)unknown_004cf6e8*(unsigned long)network_time_since(0))/1000&0xFF);
	sent->newest= sequence;
	sent->oldest= sequence;
	sent->start= 0;
	sent->count= 0;
	sent->enabled= TRUE;
	received->newest= 0;
	received->oldest= 0;
	received->start= 0;
	received->count= 0;
	received->enabled= FALSE;
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)= sequence;
	*(long *)(connection+NETWORK_CONNECTION_SENT_BYTES_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN950_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN954_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_LAST_TIME_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_LAST_UNKNOWN2_OFFSET)= 0;
	connection[NETWORK_CONNECTION_UNKNOWN958_OFFSET]= FALSE;
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET)= sequence-1;
	connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET]= TRUE;
	*(long *)(connection+NETWORK_CONNECTION_LEAST_TIME_OFFSET)= unknown_004cf70c;
	*(long *)(connection+NETWORK_CONNECTION_SMOOTHED_TIME_OFFSET)= unknown_004cf710;
	*(long *)(connection+NETWORK_CONNECTION_TIME_DEVIATION_OFFSET)= unknown_004cf714;
	*(long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)= 0;
	*(long *)(connection+NETWORK_CONNECTION_TIMEOUT_OFFSET)= unknown_004cf718;
}

/* 0x00096510: the number for one more packet that one of the things sends,
at a time; NONE if it cannot send now, as its second function says, or if it
has no room, and then it notes that. If the ring is full, the first of the
packets that it keeps is not kept any more
(network_connection_drop_oldest_sent, whatever its bits). There is no room
if the number would be 0x80 or more after the most of what was answered, or
after the number that they are counted from, or if the ring is still full.
The packet is kept from now, with how far its number is from that one */
long __attribute__((stdcall)) network_connection_new_sequence(
	byte *connection,
	long time)
{
	struct network_connection_window *sent=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	struct network_connection_sent_packet *packet;
	long sequence;
	long newest;

	if ((*(const struct network_connection_functions **)connection)->cannot_send(connection, 0))
	{
		return NONE;
	}
	if (sent->newest-sent->oldest>=sent->size)
	{
		long dropped_sequence;
		long event;

		network_connection_drop_oldest_sent(connection, TRUE, &event, &dropped_sequence);
	}
	newest= sent->newest;
	if (*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET)-newest+NETWORK_CONNECTION_SEQUENCE_RANGE<=0 ||
		newest-sent->oldest>=sent->size ||
		newest-*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)+1>=NETWORK_CONNECTION_SEQUENCE_RANGE)
	{
		connection[NETWORK_CONNECTION_FULL_OFFSET]= TRUE;

		return NONE;
	}
	sequence= newest+1;
	if (sequence>newest)
	{
		sent->count+= sequence-newest;
		sent->newest= sequence;
	}
	connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET]= FALSE;
	packet= network_connection_get_sent_packet(sequence, connection);
	memset(packet, 0, sizeof(*packet));
	packet->unknownC= (short)(sequence-*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET));
	packet->time= time;
	packet->unknown8= NONE;

	return sequence;
}

/* 0x00096ce0: the first of the packets that one of the things sent and
keeps is not kept any more, if it is time: it has one of two bits, or it is
to be so whatever, or it has been waited for as long as the wait and what the
game has more. Whether it was so; then 6 and its number are given. Nothing is
done with no packets, nor with one whose number is after the one that they
are counted from */
boolean __attribute__((stdcall)) network_connection_drop_oldest_sent(
	byte *connection,
	boolean force,
	long *event,
	long *sequence_out)
{
	struct network_connection_window *sent=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	const struct network_connection_sent_packet *packet;
	long sequence;

	*event= 0;
	if (sent->newest==sent->oldest)
	{
		return FALSE;
	}
	sequence= sent->oldest+1;
	if (sequence>*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET))
	{
		return FALSE;
	}
	packet= network_connection_get_sent_packet(sequence, connection);
	if (!(packet->flags&_network_connection_sent_bit4) && !force && !(packet->flags&_network_connection_sent_bit1) &&
		network_time_since(packet->time)<(long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_TIMEOUT_OFFSET)+
		(unsigned long)unknown_004cf6f4))
	{
		return FALSE;
	}
	*event= _network_connection_event_dropped;
	*sequence_out= sequence;
	network_connection_window_advance(sent, sequence);

	return TRUE;
}

/* 0x001a4840: the packets of some, of which something is kept in a ring,
start after one of a number: if that is after the one that they started
after, the ring starts so many places on and has so many fewer, unless it
has not so many. It is away from the rest in the executable, with other
things of the kind */
void network_connection_window_advance(
	struct network_connection_window *window,
	long sequence)
{
	long count= (long)((unsigned long)sequence-(unsigned long)window->oldest);

	if (count>0)
	{
		if (window->count>=count)
		{
			window->start= (long)((unsigned long)window->start+(unsigned long)count)%window->capacity;
			window->count-= count;
		}
		window->oldest= sequence;
	}
}

/* 0x000965e0: the next thing that happened to a packet that one of the
things sent, with the number of the packet, its size and a number of it;
nothing if there is none. First what became of the packet whose turn it is
(network_connection_next_result). If that is not known yet, the first of the
packets after it that was answered, and of which that was not said, is said
now: 4; and that is noted of it */
long __attribute__((stdcall)) network_connection_next_event(
	byte *connection,
	long *sequence_out,
	long *size,
	long *unknown8)
{
	const struct network_connection_window *sent=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	long event= 0;
	long sequence;
	long newest;

	if (network_connection_next_result(connection, &event, sequence_out, size, unknown8) || sent->newest==sent->oldest)
	{
		return event;
	}
	newest= sent->newest;
	for (sequence= *(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)+1; sequence<newest; sequence++)
	{
		struct network_connection_sent_packet *packet= network_connection_get_sent_packet(sequence, connection);

		if ((packet->flags&_network_connection_sent_bit1) && !(packet->flags&_network_connection_sent_bit2))
		{
			*sequence_out= sequence;
			*size= packet->size;
			*unknown8= packet->unknown8;
			packet->flags|= _network_connection_sent_bit2;

			return _network_connection_event_answered_early;
		}
	}

	return event;
}

/* 0x00096b00: what became of the packet whose turn it is, which is the one
after the number that they are counted from: whether it is known. With no
such packet, it is not. One that was answered: 1, or 2 if that was said
before. One that was not: if it has been waited for as long as the wait and
the loss, or if so many packets after it were answered, 3, with how long it
was; and the loss is more, as network_connection_update_times does. With
one or the other, the number of the packet, its size and a number of it are
given, its turn is over, the next one's is, and its size is taken from the
bytes that were sent */
boolean __attribute__((stdcall)) network_connection_next_result(
	byte *connection,
	long *event,
	long *sequence_out,
	long *size,
	long *unknown8)
{
	long sequence= *(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)+1;
	struct network_connection_sent_packet *packet= network_connection_get_sent_packet(sequence, connection);

	if (!packet)
	{
		return FALSE;
	}
	if (packet->flags&_network_connection_sent_bit1)
	{
		*event= (packet->flags&_network_connection_sent_bit2) ? _network_connection_event_answered_late :
			_network_connection_event1;
		*sequence_out= sequence;
		*size= packet->size;
		*unknown8= packet->unknown8;
		packet->flags|= _network_connection_sent_bit4;
	}
	else
	{
		long elapsed= network_time_since(packet->time);

		if (elapsed>=(long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_TIMEOUT_OFFSET)+
			*(const unsigned long *)(connection+NETWORK_CONNECTION_LOSS_OFFSET)) ||
			(long)(*(const unsigned long *)(connection+NETWORK_CONNECTION_UNKNOWN94C_OFFSET)-(unsigned long)sequence)>=
			unknown_004cf6f0)
		{
			packet->unknown8= elapsed;
			*event= _network_connection_event_lost;
			*sequence_out= sequence;
			*size= packet->size;
			*unknown8= packet->unknown8;
			packet->flags|= _network_connection_sent_bit8;
			network_connection_update_times(_network_connection_event_lost, connection, network_time_since(0), 0, 0);
		}
	}
	if (!(packet->flags&(_network_connection_sent_bit4|_network_connection_sent_bit8)))
	{
		return FALSE;
	}
	packet->flags|= _network_connection_sent_bit10;
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)= sequence;
	*(long *)(connection+NETWORK_CONNECTION_SENT_BYTES_OFFSET)-= packet->size;

	return TRUE;
}

/* 0x00096910: one of the things is told what the other console says of the
packets that it sent: the low byte of the number of the last that came
there, and some bits of the ones before it, of which the first is of the
one just before; a bit that is set says that the packet did not come.

The number is the last one that the thing sent, or before, that has that
byte. An answer of a packet that is not kept any more does nothing, and if
it is of more than so many before, 2 is what is said. If not, each packet
from that one back to the first that is kept, that was not answered yet, is
answered now: that one; the ones before it whose bit is not set; and, of the
ones that there are no bits for, the ones after the number that they are
counted from, now or when the packet of the answer was sent. An answered
packet has the bit of it, and how long the answer took, and the times of the
thing follow (network_connection_update_times, with 4, or with 5 if the
packet had been taken for one with no answer).

And if the answer is of the last packet that was sent, and that is the one
that they are counted from, a byte of the thing is as the last thing given */
long __attribute__((stdcall)) network_connection_acknowledge(
	byte *connection,
	long sequence_byte,
	long bit_count,
	const unsigned long *bits,
	boolean unknown)
{
	const struct network_connection_window *sent=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	const struct network_connection_sent_packet *answered;
	long result= 0;
	long difference= (sent->newest&0xFF)-sequence_byte;
	long answered_sequence;
	long base;

	if (difference<0)
	{
		difference+= 0x100;
	}
	answered_sequence= sent->newest-difference;
	answered= network_connection_get_sent_packet(answered_sequence, connection);
	base= answered ? answered_sequence-(word)answered->unknownC :
		*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET);
	connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET]= FALSE;
	if (answered_sequence<=sent->oldest)
	{
		if (answered_sequence<sent->oldest-unknown_004cf6ec)
		{
			result= _network_connection_acknowledge_too_old;
		}
	}
	else
	{
		long sequence;
		long bit= NONE;

		for (sequence= answered_sequence; sequence>sent->oldest; sequence--, bit++)
		{
			struct network_connection_sent_packet *packet= network_connection_get_sent_packet(sequence, connection);
			boolean came;

			if (packet->flags&_network_connection_sent_bit1)
			{
				continue;
			}
			if (sequence==answered_sequence)
			{
				came= TRUE;
			}
			else if (bit<bit_count)
			{
				came= !(bits[bit>>5]&(1u<<(bit&31)));
			}
			else
			{
				came= sequence>*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET) || sequence>base;
			}
			if (came)
			{
				long time= packet->time;
				long event= (packet->flags&_network_connection_sent_bit8) ? _network_connection_event5 :
					_network_connection_event4;

				packet->flags|= _network_connection_sent_bit1;
				packet->unknown8= network_time_since(time);
				network_connection_update_times(event, connection, packet->unknown8, sequence, network_time_since(0));
			}
		}
	}
	if (answered_sequence==sent->newest && *(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET)==sent->newest &&
		unknown)
	{
		connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET]= TRUE;
	}

	return result;
}

/* 0x00096360: one of the things reads its own of a packet that came. It is
its sixth function, the one that a channel calls for each of its things
(network_channel_read_packet), and what it says is what that takes: well
(0), or that the packet is not one to read (2), which is also said when the
thing cannot send, as its second function says.

A bit says whether the packet has a number; then come the low byte of its
number and a number of seven bits, with which the thing sees to the packets
that came (network_connection_receive_sequence); what that says, if it is
not nothing, is said. The number of the packet is given.

Then what the other console says of the packets that it was sent: the low
byte of the number of the last that came there, and the bits of the ones
before it, written in one of four ways, as two bits say: none, and one bit
more that is kept; one bit that is set, by its place in three bits; eight
bits; or so many bits as seven say (and before them a bit with which nothing
more is done). With that the thing is told
(network_connection_acknowledge), and what that says is said.

In a stream that is only looked at, the number that is given is the one of
the last packet that the thing sent, or NONE, and nothing is done */
long __attribute__((stdcall)) network_connection_read(
	byte *connection,
	long *sequence_out,
	struct bitstream *stream)
{
	unsigned long bits[MAXIMUM_NETWORK_CONNECTION_BITS/32]= {0};
	boolean unknown= FALSE;
	long bit_count= 0;
	long sequence_byte;
	boolean numbered;
	long unknown7;
	boolean first;
	boolean second;
	long bit;

	if ((*(const struct network_connection_functions **)connection)->cannot_send(connection, 0))
	{
		return _network_connection_read_no;
	}
	numbered= bitstream_read_bool(stream);
	sequence_byte= (long)bitstream_read_integer(stream, NETWORK_CONNECTION_SEQUENCE_BITS);
	unknown7= (long)bitstream_read_integer(stream, NETWORK_CONNECTION_UNKNOWN_BITS);
	if (stream->unknownC==_network_connection_stream_look_only)
	{
		*sequence_out= numbered ?
			((const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET))->newest : NONE;
	}
	else
	{
		long result= network_connection_receive_sequence(connection, sequence_byte, sequence_out, numbered, unknown7);

		if (result!=_network_connection_read_good)
		{
			return result;
		}
	}
	sequence_byte= (long)bitstream_read_integer(stream, NETWORK_CONNECTION_SEQUENCE_BITS);
	first= bitstream_read_bool(stream);
	second= bitstream_read_bool(stream);
	if (!first && !second)
	{
		unknown= bitstream_read_bool(stream);
	}
	else if (!first)
	{
		bit= (long)bitstream_read_integer(stream, NETWORK_CONNECTION_ONE_BIT_BITS);
		bits[0]|= 1u<<(bit&31);
		bit_count= bit+1;
	}
	else if (!second)
	{
		for (bit= 0; bit<NETWORK_CONNECTION_SHORT_BIT_COUNT; bit++)
		{
			if (bitstream_read_bool(stream))
			{
				bits[0]|= 1u<<bit;
				bit_count= bit+1;
			}
		}
	}
	else
	{
		if (bitstream_read_bool(stream))
		{
			return _network_connection_read_good;
		}
		bit_count= (long)bitstream_read_integer(stream, NETWORK_CONNECTION_BIT_COUNT_BITS);
		for (bit= 0; bit<bit_count; bit++)
		{
			if (bitstream_read_bool(stream))
			{
				bits[bit>>5]|= 1u<<(bit&31);
			}
			else
			{
				bits[bit>>5]&= ~(1u<<(bit&31));
			}
		}
	}
	if (stream->unknownC==_network_connection_stream_look_only)
	{
		return _network_connection_read_good;
	}

	return network_connection_acknowledge(connection, sequence_byte, bit_count, bits, unknown);
}

/* 0x00096710: one of the things sees to the packets that came, with what a
packet says: the low byte of its number, whether it has one, and how many
packets before it the other console does not wait for any more. The first
time, the packets that came start just before this one.

The number is the one nearest the last that came that has that byte, before
or after; it is given, or NONE for a packet with no number. A byte of the
thing says whether the other number is nothing, and the most that the number
less that has been is kept. The packets that came do not start before that:
they start over from there if they were all before, or the first ones are
not kept any more.

What is said: nothing, for a packet after the first that is kept; 2, for one
before it, or for that very one if it has a number */
long __attribute__((stdcall)) network_connection_receive_sequence(
	byte *connection,
	long sequence_byte,
	long *sequence_out,
	boolean numbered,
	long unknown)
{
	struct network_connection_window *received=
		(struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	long difference;
	long sequence;
	long least;

	if (!received->enabled)
	{
		received->newest= sequence_byte-1;
		received->oldest= sequence_byte-1;
		received->start= 0;
		received->count= 0;
		received->enabled= TRUE;
	}
	difference= sequence_byte-(received->newest&0xFF);
	if (difference>0x80)
	{
		difference-= 0x100;
	}
	else if (difference<=-0x80)
	{
		difference+= 0x100;
	}
	sequence= (long)((unsigned long)received->newest+(unsigned long)difference);
	*sequence_out= numbered ? sequence : NONE;
	connection[NETWORK_CONNECTION_UNKNOWN958_OFFSET]= unknown==0;
	least= (long)((unsigned long)sequence-(unsigned long)unknown);
	if (least>*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN954_OFFSET))
	{
		*(long *)(connection+NETWORK_CONNECTION_UNKNOWN954_OFFSET)= least;
	}
	if (received->newest<=least)
	{
		received->newest= least;
		received->oldest= least;
		received->start= 0;
		received->count= 0;
		received->enabled= TRUE;
	}
	else
	{
		while (received->oldest<least)
		{
			network_connection_window_advance(received, (long)((unsigned long)received->oldest+1));
		}
	}
	if (sequence>received->oldest)
	{
		return _network_connection_read_good;
	}
	if (sequence==received->oldest && !numbered)
	{
		return _network_connection_read_good;
	}

	return _network_connection_read_no;
}

/* 0x00081410: whether the channel of one of the things is to stop, and why:
it is, with 0xB for a reason, if the thing is someone's and had no room for
the number of a packet. It is the second function of the thing, which the
table at 0x00450dd8 has, as the three below are, and Ghidra's list has none
of them; this one is away from the rest in the executable */
boolean __attribute__((stdcall)) network_connection_closes(
	const byte *connection,
	long *reason)
{
	if (connection[NETWORK_CONNECTION_USED_OFFSET] && connection[NETWORK_CONNECTION_FULL_OFFSET])
	{
		if (reason)
		{
			*reason= _network_connection_closed_full;
		}

		return TRUE;
	}

	return FALSE;
}

/* 0x00095dc0: whether one of the things has something to send: it has if
packets came that it has not said of, or of which the other console waits
for more than it said; or if a byte of it is not set. The third function */
boolean __attribute__((stdcall)) network_connection_has_something(
	const byte *connection,
	boolean *urgent)
{
	const struct network_connection_window *received=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	boolean result= FALSE;

	if (received->enabled && (*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN950_OFFSET)<received->newest ||
		*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN954_OFFSET)<received->newest))
	{
		result= TRUE;
	}
	if (!connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET])
	{
		result= TRUE;
	}

	return result;
}

/* 0x00095df0: how many bits one of the things needs in a packet, at the
least: after not keeping the first packets that came any more
(network_connection_advance_received), 0x13 if it is not in use for packets
that come, 0x12 with none kept, 0x1A with nine or fewer, and 0x19 and one for
each with more. The fourth function */
long __attribute__((stdcall)) network_connection_minimum_bits(
	byte *connection,
	long available,
	long left)
{
	const struct network_connection_window *received=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);

	network_connection_advance_received(connection);
	if (!received->enabled)
	{
		return NETWORK_CONNECTION_BITS_NOT_RECEIVING;
	}
	if (received->newest==received->oldest)
	{
		return NETWORK_CONNECTION_BITS_NONE;
	}
	if (received->newest-received->oldest<=NETWORK_CONNECTION_FEW_PACKETS)
	{
		return NETWORK_CONNECTION_BITS_FEW;
	}

	return received->newest-received->oldest+NETWORK_CONNECTION_BITS_MANY;
}

/* whether the packet of a number came, by its word */
static inline __attribute__((always_inline)) boolean network_connection_packet_received(
	const byte *connection,
	long sequence)
{
	const struct network_connection_window *received=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);

	return (((const word *)(connection+NETWORK_CONNECTION_RECEIVED_OFFSET))[network_connection_window_place(received,
		sequence)]&_network_connection_received_bit)!=0;
}

/* 0x00095e40: one of the things writes its own in a packet: what
network_connection_read reads. Whether the packet has a number; the low byte
of the number of the last packet that the thing sent, and how far that is
from the one that they are counted from, in seven bits (with nothing between
them, a byte of the thing is set).

Then, after not keeping the first packets that came any more, what it says
of the packets that came. If it is not in use for them: nothing in eight
bits, and three bits set. If it is: the low byte of the number of the last
that came, which is noted as said, and the bits of the ones before it down
to the first that is kept, a bit set for one that did not come, in the
shortest of the ways: with none kept, two bits clear and a byte of the thing
as a bit; with nine or fewer, the place of the only one that did not come,
in three bits, after a bit clear and one set, or else eight bits, after a bit
set and one clear; and with more, three bits (set, set, clear), how many in
seven bits, and all of them.

It says FALSE. The fifth function; the game has bitstream_write_bool and
bitstream_write_integer_checked written here */
boolean __attribute__((stdcall)) network_connection_write(
	byte *connection,
	long sequence,
	struct bitstream *stream,
	long bits,
	long rest)
{
	const struct network_connection_window *received=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_RECEIVED_WINDOW_OFFSET);
	const struct network_connection_window *sent=
		(const struct network_connection_window *)(connection+NETWORK_CONNECTION_SENT_WINDOW_OFFSET);
	long newest= sent->newest;
	long distance= newest-*(const long *)(connection+NETWORK_CONNECTION_UNKNOWN44_OFFSET);
	long other;

	bitstream_write_bool(stream, sequence!=NONE);
	bitstream_write_integer_checked((unsigned long)(newest&0xFF), NETWORK_CONNECTION_SEQUENCE_BITS, stream);
	bitstream_write_integer_checked((unsigned long)distance, NETWORK_CONNECTION_UNKNOWN_BITS, stream);
	if (distance==0)
	{
		connection[NETWORK_CONNECTION_UNKNOWN959_OFFSET]= TRUE;
	}
	network_connection_advance_received(connection);
	if (!received->enabled)
	{
		bitstream_write_integer(stream, 0, NETWORK_CONNECTION_SEQUENCE_BITS);
		bitstream_write_bool(stream, TRUE);
		bitstream_write_bool(stream, TRUE);
		bitstream_write_bool(stream, TRUE);

		return FALSE;
	}
	other= received->newest-1;
	bitstream_write_integer_checked((unsigned long)(received->newest&0xFF), NETWORK_CONNECTION_SEQUENCE_BITS, stream);
	*(long *)(connection+NETWORK_CONNECTION_UNKNOWN950_OFFSET)= received->newest;
	if (received->newest==received->oldest)
	{
		bitstream_write_bool(stream, FALSE);
		bitstream_write_bool(stream, FALSE);
		bitstream_write_bool(stream, connection[NETWORK_CONNECTION_UNKNOWN958_OFFSET]);
	}
	else if (received->newest-received->oldest<=NETWORK_CONNECTION_FEW_PACKETS)
	{
		unsigned long missing= 0;
		boolean several= FALSE;
		long first= NONE;
		long bit;

		for (bit= 0; other>received->oldest; other--, bit++)
		{
			if (!network_connection_packet_received(connection, other))
			{
				missing|= 1u<<(bit&31);
				if (first==NONE)
				{
					first= bit;
				}
				else
				{
					several= TRUE;
				}
			}
		}
		if (several)
		{
			bitstream_write_bool(stream, TRUE);
			bitstream_write_bool(stream, FALSE);
			for (bit= 0; bit<NETWORK_CONNECTION_SHORT_BIT_COUNT; bit++)
			{
				bitstream_write_bool(stream, (missing&(1u<<bit))!=0);
			}
		}
		else
		{
			bitstream_write_bool(stream, FALSE);
			bitstream_write_bool(stream, TRUE);
			bitstream_write_integer_checked((unsigned long)first, NETWORK_CONNECTION_ONE_BIT_BITS, stream);
		}
	}
	else
	{
		bitstream_write_bool(stream, TRUE);
		bitstream_write_bool(stream, TRUE);
		bitstream_write_bool(stream, FALSE);
		bitstream_write_integer_checked((unsigned long)(received->newest-received->oldest-1), NETWORK_CONNECTION_BIT_COUNT_BITS,
			stream);
		for (; other>received->oldest; other--)
		{
			/* the game looks at the word of the packet only if the stream has
			room for the bit */
			if ((long)(((unsigned long)stream->size<<3)-(unsigned long)stream->position)>=1)
			{
				bitstream_write_bool(stream, !network_connection_packet_received(connection, other));
			}
			else
			{
				bitstream_write_bool(stream, FALSE);
			}
		}
	}

	return FALSE;
}
