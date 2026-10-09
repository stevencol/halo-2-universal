/*
TRANSPORT_QOS.H

The attempts that this console makes, with the console's libraries, at
finding out how its connection with others is. The name of the file comes
from the one that the game gives the table of them, "transport qos attempts";
the names of its functions are guesses.
*/

#ifndef __TRANSPORT_QOS_H
#define __TRANSPORT_QOS_H

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	/* how many attempts there can be at once, and how many consoles one can
	be about */
	MAXIMUM_TRANSPORT_QOS_ATTEMPTS= 32,
	MAXIMUM_TRANSPORT_QOS_TARGETS= 64,
	/* the two kinds of attempt, which differ in how many times a console is
	tried and with how many bits a second */
	NUMBER_OF_TRANSPORT_QOS_KINDS= 2,
	/* what a session is known by in the network and its key, and what a
	console is known by: the sizes that the console's libraries have */
	TRANSPORT_QOS_IDENTIFIER_SIZE= 0x8,
	TRANSPORT_QOS_KEY_SIZE= 0x10,
	TRANSPORT_QOS_ADDRESS_SIZE= 0x24,

	/* what the console's libraries say of a console of an attempt */
	_xnet_qos_complete_bit= 0,
	_xnet_qos_contacted_bit,
	_xnet_qos_disabled_bit,
	_xnet_qos_data_received_bit,
	_xnet_qos_partial_complete_bit,

	/* how an attempt is going with a console: not tried yet, tried, with
	something of what is to be found, done without an answer (and what is
	said of an attempt that there is not), done with a console that takes no
	attempts, and done with an answer */
	_transport_qos_target_waiting= 0,
	_transport_qos_target_trying,
	_transport_qos_target_partial,
	_transport_qos_target_failed,
	_transport_qos_target_disabled,
	_transport_qos_target_done
};

/* ---------- structures */

/* what the console's libraries keep of an attempt: how many consoles it is
about, how many of them are not done, and what is known of each: how many
times it was tried and answered, what it sent back, how long the answers took
in milliseconds (the least and the middle one), and how many bits a second go
each way */
struct xnet_qos_info
{
	byte flags;
	byte reserved;
	word probes_sent;
	word probes_received;
	word data_size;
	byte *data;
	word minimum_round_trip;
	word median_round_trip;
	unsigned long up_bits_per_second;
	unsigned long down_bits_per_second;
};

typedef char verify_xnet_qos_info_size[sizeof(struct xnet_qos_info) == 0x18 || sizeof(void *) != 4 ? 1 : -1];

struct xnet_qos
{
	unsigned long count;
	unsigned long pending_count;
	struct xnet_qos_info info[1];
};

/* an attempt: what the console's libraries keep of it */
struct transport_qos_attempt
{
	short identifier;
	short unknown2;
	struct xnet_qos *qos;
};

typedef char verify_transport_qos_attempt_size[sizeof(struct transport_qos_attempt) == 0x8 || sizeof(void *) != 4 ? 1 : -1];

/* a console that an attempt is about: what a session is known by, its key,
and what the console is known by */
struct transport_qos_target
{
	byte identifier[TRANSPORT_QOS_IDENTIFIER_SIZE];
	byte key[TRANSPORT_QOS_KEY_SIZE];
	byte address[TRANSPORT_QOS_ADDRESS_SIZE];
};

typedef char verify_transport_qos_target_size[sizeof(struct transport_qos_target) == 0x3C ? 1 : -1];

/* what an attempt found of a console */
struct transport_qos_result
{
	long probes_sent;
	long probes_received;
	long minimum_round_trip;
	long median_round_trip;
	unsigned long up_bits_per_second;
	unsigned long down_bits_per_second;
	long data_size;
	byte *data;
};

typedef char verify_transport_qos_result_size[sizeof(struct transport_qos_result) == 0x20 || sizeof(void *) != 4 ? 1 : -1];

/* ---------- prototypes/TRANSPORT_QOS.C */

void transport_qos_initialize(void);
void transport_qos_dispose(void);
long __attribute__((stdcall)) transport_qos_new(long bits_per_second, long count, long kind,
	const struct transport_qos_target *targets);
long transport_qos_service_new(void);
void transport_qos_delete(long index);
boolean transport_qos_is_complete(long index);
long transport_qos_get_state(long index, long target_index);
boolean __attribute__((stdcall)) transport_qos_get_result(long index, struct transport_qos_result *result,
	long target_index);

/* ---------- globals */

extern struct data_array *transport_qos_attempts;
extern const long transport_qos_probe_counts[NUMBER_OF_TRANSPORT_QOS_KINDS];
extern const long transport_qos_bits_per_second[NUMBER_OF_TRANSPORT_QOS_KINDS];

#endif
