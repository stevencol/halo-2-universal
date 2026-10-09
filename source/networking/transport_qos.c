/*
TRANSPORT_QOS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The attempts that this console makes, with the console's libraries, at
finding out how its connection with others is: how long an answer takes and
how many bits a second go each way. The name of the file comes from the one
that the game gives the table of them, "transport qos attempts"; the names of
its functions are guesses.
*/

#include "transport_qos.h"

#include "networking/transport_security.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003cd156: an attempt starts, about so many consoles (what each is known
by, what a session is known by and its key) and none of the internet, with
so many tries and bits a second; what the libraries keep of it. 0x003cd172:
it is no more. 0x003cd365: an attempt about the service starts. Ghidra has
no name for this one: it is the one of the library that takes these three
things */
long __attribute__((stdcall)) XNetQosLookup(unsigned long count, const byte **addresses, const byte **identifiers,
	const byte **keys, unsigned long internet_count, const void *internet_addresses,
	const unsigned long *service_identifiers, unsigned long probe_count, unsigned long bits_per_second,
	unsigned long flags, void *event, struct xnet_qos **qos);
long __attribute__((stdcall)) XNetQosRelease(struct xnet_qos *qos);
long __attribute__((stdcall)) XNetQosServiceLookup(unsigned long flags, void *event, struct xnet_qos **qos);

/* ---------- globals */

/* 0x004cf8d8: the attempts. With unknown_004cf8d4 (transport_security.c)
they can be made */
struct data_array *transport_qos_attempts;

/* 0x00440188, 0x00440190: how many times a console is tried in an attempt of
each kind, and with how many bits a second when none are said */
const long transport_qos_probe_counts[NUMBER_OF_TRANSPORT_QOS_KINDS]= {8, 0};
const long transport_qos_bits_per_second[NUMBER_OF_TRANSPORT_QOS_KINDS]= {0x4000, 0x1000};

/* ---------- code */

/* a new attempt of the table, which is at the place that the low half of
its index says: what the console's libraries keep of it is its own */
static inline __attribute__((always_inline)) void transport_qos_attempt_set(
	long index,
	struct xnet_qos *qos)
{
	struct transport_qos_attempt *attempt= (struct transport_qos_attempt *)transport_qos_attempts->data+(index&0xFFFF);

	attempt->unknown2= 1;
	attempt->qos= qos;
}

/* the attempt that an index says, if attempts can be made and there is such
a one */
static inline __attribute__((always_inline)) struct transport_qos_attempt *transport_qos_attempt_get(
	long index)
{
	if (!unknown_004cf8d4)
	{
		return NULL;
	}

	return datum_try_and_get(transport_qos_attempts, index);
}

/* 0x0007b3e0: there are no attempts and none can be made yet; the table of
them has memory of its own (the game has data_new written here), and none if
there is none to give */
void transport_qos_initialize(
	void)
{
	unknown_004cf8d4= FALSE;
	transport_qos_attempts= NULL;
	transport_qos_attempts= data_new("transport qos attempts", MAXIMUM_TRANSPORT_QOS_ATTEMPTS,
		sizeof(struct transport_qos_attempt), 0, heap_allocation);
}

/* 0x0007b430: if attempts can be made, each one is no more
(transport_qos_delete), the table is not valid and none can be made */
void transport_qos_dispose(
	void)
{
	if (unknown_004cf8d4)
	{
		struct data_array *attempts= transport_qos_attempts;
		long index= NONE;

		while ((index= data_next_index(attempts, index))!=NONE)
		{
			transport_qos_delete(index);
		}
		transport_qos_attempts->valid= FALSE;
		unknown_004cf8d4= FALSE;
	}
}

/* 0x0007b4c0: an attempt about some consoles, 64 at the most, of one of the
two kinds; its index, NONE if attempts cannot be made, the console's
libraries do not start it, or there is no room for one more (and then it is
no more). With NONE for the bits a second, they are the ones of its kind */
long __attribute__((stdcall)) transport_qos_new(
	long bits_per_second,
	long count,
	long kind,
	const struct transport_qos_target *targets)
{
	const byte *keys[MAXIMUM_TRANSPORT_QOS_TARGETS];
	const byte *identifiers[MAXIMUM_TRANSPORT_QOS_TARGETS];
	const byte *addresses[MAXIMUM_TRANSPORT_QOS_TARGETS];
	struct xnet_qos *qos;
	long probe_count;
	long target_index;
	long index;

	if (!unknown_004cf8d4)
	{
		return NONE;
	}
	probe_count= transport_qos_probe_counts[kind];
	if (bits_per_second==NONE)
	{
		bits_per_second= transport_qos_bits_per_second[kind];
	}
	if (count>MAXIMUM_TRANSPORT_QOS_TARGETS)
	{
		count= MAXIMUM_TRANSPORT_QOS_TARGETS;
	}
	for (target_index= 0; target_index<count; target_index++)
	{
		identifiers[target_index]= targets[target_index].identifier;
		addresses[target_index]= targets[target_index].address;
		keys[target_index]= targets[target_index].key;
	}
	if (XNetQosLookup(count, addresses, identifiers, keys, 0, NULL, NULL, probe_count, bits_per_second, 0, NULL, &qos)!=0)
	{
		return NONE;
	}
	index= datum_new(transport_qos_attempts);
	if (index!=NONE)
	{
		transport_qos_attempt_set(index, qos);

		return index;
	}
	XNetQosRelease(qos);

	return NONE;
}

/* 0x0007b5e0: an attempt about the service; its index, NONE if attempts
cannot be made, the console's libraries do not start it, or there is no room
for one more (and then it is no more) */
long transport_qos_service_new(
	void)
{
	struct xnet_qos *qos;
	long index;

	if (!unknown_004cf8d4 || XNetQosServiceLookup(0, NULL, &qos)!=0)
	{
		return NONE;
	}
	index= datum_new(transport_qos_attempts);
	if (index!=NONE)
	{
		transport_qos_attempt_set(index, qos);

		return index;
	}
	XNetQosRelease(qos);

	return NONE;
}

/* 0x0007b650: an attempt is no more: the console's libraries let go of what
they keep of it, and it is out of the table */
void transport_qos_delete(
	long index)
{
	struct transport_qos_attempt *attempt= transport_qos_attempt_get(index);

	if (attempt)
	{
		if (XNetQosRelease(attempt->qos)==0)
		{
			attempt->qos= NULL;
		}
		datum_delete(transport_qos_attempts, index);
	}
}

/* 0x0007b6c0: whether an attempt is done with all its consoles; and there is
nothing to wait for of one that there is not */
boolean transport_qos_is_complete(
	long index)
{
	const struct transport_qos_attempt *attempt= transport_qos_attempt_get(index);

	if (attempt)
	{
		return attempt->qos->pending_count==0;
	}

	return TRUE;
}

/* 0x0007b720: how an attempt is going with one of its consoles */
long transport_qos_get_state(
	long index,
	long target_index)
{
	const struct transport_qos_attempt *attempt= transport_qos_attempt_get(index);
	const struct xnet_qos_info *info;

	if (!attempt)
	{
		return _transport_qos_target_failed;
	}
	info= &attempt->qos->info[target_index];
	if (info->flags&FLAG(_xnet_qos_complete_bit))
	{
		if (info->flags&FLAG(_xnet_qos_disabled_bit))
		{
			return _transport_qos_target_disabled;
		}

		return (info->flags&FLAG(_xnet_qos_contacted_bit)) ? _transport_qos_target_done : _transport_qos_target_failed;
	}
	if (info->flags&FLAG(_xnet_qos_partial_complete_bit))
	{
		return _transport_qos_target_partial;
	}

	return info->probes_sent!=0 ? _transport_qos_target_trying : _transport_qos_target_waiting;
}

/* 0x0007b7b0: what an attempt found of one of its consoles, if it answered
and the attempt is done with it, or has something of it; whether it did.
What the console sent back is only given if it sent something */
boolean __attribute__((stdcall)) transport_qos_get_result(
	long index,
	struct transport_qos_result *result,
	long target_index)
{
	const struct transport_qos_attempt *attempt= transport_qos_attempt_get(index);
	const struct xnet_qos_info *info;

	if (!attempt)
	{
		return FALSE;
	}
	info= &attempt->qos->info[target_index];
	if (!(info->flags&FLAG(_xnet_qos_contacted_bit)) ||
		!(info->flags&(FLAG(_xnet_qos_complete_bit)|FLAG(_xnet_qos_partial_complete_bit))))
	{
		return FALSE;
	}
	memset(result, 0, sizeof(*result));
	result->probes_sent= info->probes_sent;
	result->probes_received= info->probes_received;
	result->minimum_round_trip= info->minimum_round_trip;
	result->median_round_trip= info->median_round_trip;
	result->up_bits_per_second= info->up_bits_per_second;
	result->down_bits_per_second= info->down_bits_per_second;
	if (info->flags&FLAG(_xnet_qos_data_received_bit))
	{
		result->data_size= info->data_size;
		result->data= info->data;
	}

	return TRUE;
}
