/*
NETWORK_SESSION.H

A session of the network game: the consoles that play together, and their
players. Only a little of it is known. The name of the file is a guess, and
so are the names of its functions.
*/

#ifndef __NETWORK_SESSION_H
#define __NETWORK_SESSION_H

#include "cseries/cseries.h"
#include "text/unicode.h"
#include "networking/transport_security.h"

/* ---------- constants */

enum
{
	/* no less than this: network_session_start writes up to here */
	NETWORK_SESSION_SIZE= 0x78B8,

	/* in a session: its state, 0 for none; which console this one is and
	which one runs the session; and more that is only read */
	NETWORK_SESSION_UNKNOWN4_OFFSET= 0x4,
	NETWORK_SESSION_UNKNOWN8_OFFSET= 0x8,
	NETWORK_SESSION_TABLE_OFFSET= 0xC,
	NETWORK_SESSION_INDEX_OFFSET= 0x10,
	NETWORK_SESSION_UNKNOWN14_OFFSET= 0x14,
	NETWORK_SESSION_UNKNOWN18_OFFSET= 0x18,
	/* eight bytes that the session is known by, and whether it has them */
	NETWORK_SESSION_IDENTIFIER_OFFSET= 0x1C,
	NETWORK_SESSION_HAS_IDENTIFIER_OFFSET= 0x24,
	NETWORK_SESSION_UNKNOWN38_OFFSET= 0x38,
	NETWORK_SESSION_UNKNOWN40_OFFSET= 0x40,
	NETWORK_SESSION_UNKNOWN48_OFFSET= 0x48,
	/* who is in the session, and who will be: each starts with a number that
	is NONE when it says nothing */
	NETWORK_SESSION_MEMBERSHIP_OFFSET= 0x4C,
	NETWORK_SESSION_MEMBERSHIP_SIZE= 0x2494,
	NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET= 0x24E0,
	NETWORK_SESSION_HOST_PEER_OFFSET= 0x50,
	/* two more things of the same size, which start with NONE too */
	NETWORK_SESSION_UNKNOWN4978_OFFSET= 0x4978,
	NETWORK_SESSION_UNKNOWN4978_SIZE= 0x14B0,
	NETWORK_SESSION_UNKNOWN5E28_OFFSET= 0x5E28,
	NETWORK_SESSION_UNKNOWN72DC_OFFSET= 0x72DC,
	NETWORK_SESSION_UNKNOWN72DC_SIZE= 0x140,
	NETWORK_SESSION_UNKNOWN7618_OFFSET= 0x7618,
	NETWORK_SESSION_UNKNOWN761C_OFFSET= 0x761C,
	NETWORK_SESSION_UNKNOWN761C_SIZE= 0x34,
	NETWORK_SESSION_UNKNOWN7650_OFFSET= 0x7650,
	NETWORK_SESSION_UNKNOWN7654_OFFSET= 0x7654,
	NETWORK_SESSION_UNKNOWN7658_OFFSET= 0x7658,
	NETWORK_SESSION_UNKNOWN7668_OFFSET= 0x7668,
	NETWORK_SESSION_UNKNOWN7668_SIZE= 0x240,
	NETWORK_SESSION_UNKNOWN78A8_OFFSET= 0x78A8,
	NETWORK_SESSION_UNKNOWN78AC_OFFSET= 0x78AC,

	/* what a session is given when it is set up has something of 0x24 bytes
	for each session: two numbers that start at NONE, and the session */
	NETWORK_SESSION_UNKNOWN8_ENTRY_SIZE= 0x24,
	NETWORK_SESSION_UNKNOWN8_ENTRY_SESSION_OFFSET= 0x14,
	NETWORK_SESSION_UNKNOWN8_ENTRY_UNKNOWN18_OFFSET= 0x18,
	NETWORK_SESSION_UNKNOWN8_ENTRY_UNKNOWN1C_OFFSET= 0x1C,
	NETWORK_SESSION_UNKNOWN49A8_OFFSET= 0x49A8,
	NETWORK_SESSION_UNKNOWN49AC_OFFSET= 0x49AC,
	NETWORK_SESSION_UNKNOWN49F8_OFFSET= 0x49F8,
	NETWORK_SESSION_LOCAL_PEER_OFFSET= 0x72D8,
	NETWORK_SESSION_STATE_OFFSET= 0x741C,
	NETWORK_SESSION_UNKNOWN7420_OFFSET= 0x7420,

	/* what network_session_start sets in the first of the two things of
	0x14B0 bytes: when the session started (GetTickCount), the language of
	the game, two numbers that start at 16, eight bytes made at random, and
	more that is not known */
	NETWORK_SESSION_UNKNOWN497C_OFFSET= 0x497C,
	NETWORK_SESSION_UNKNOWN4980_OFFSET= 0x4980,
	NETWORK_SESSION_START_TIME_OFFSET= 0x4984,
	NETWORK_SESSION_LANGUAGE_OFFSET= 0x4988,
	NETWORK_SESSION_UNKNOWN498C_OFFSET= 0x498C,
	NETWORK_SESSION_UNKNOWN4990_OFFSET= 0x4990,
	NETWORK_SESSION_UNKNOWN4994_OFFSET= 0x4994,
	NETWORK_SESSION_UNKNOWN4990_DEFAULT= 16,
	NETWORK_SESSION_UNKNOWN49A4_OFFSET= 0x49A4,
	NETWORK_SESSION_UNKNOWN49B0_OFFSET= 0x49B0,
	NETWORK_SESSION_UNKNOWN49B4_OFFSET= 0x49B4,
	NETWORK_SESSION_UNKNOWN49C8_OFFSET= 0x49C8,
	NETWORK_SESSION_UNKNOWN4D08_OFFSET= 0x4D08,
	NETWORK_SESSION_UNKNOWN4D0C_OFFSET= 0x4D0C,
	NETWORK_SESSION_RANDOM_OFFSET= 0x4DA0,
	NETWORK_SESSION_RANDOM_SIZE= 0x8,
	NETWORK_SESSION_UNKNOWN4DA8_OFFSET= 0x4DA8,
	NETWORK_SESSION_UNKNOWN4DAC_OFFSET= 0x4DAC,
	NETWORK_SESSION_UNKNOWN5DD0_OFFSET= 0x5DD0,
	NETWORK_SESSION_UNKNOWN5E20_OFFSET= 0x5E20,
	/* in who is in the session: how many consoles, and the consoles */
	NETWORK_SESSION_PEER_COUNT_OFFSET= 0x54,
	NETWORK_SESSION_PEER_ARRAY_OFFSET= 0x58,
	NETWORK_SESSION_PEER_SIZE= 0x10C,
	/* of a console: what it is known by in the network; two texts of
	characters of 16 bits, of 16 and of 32; how many players it has and
	which they are, NONE where there is none; and two numbers that it comes
	in with or not */
	NETWORK_SESSION_PEER_PLAYER_COUNT_OFFSET= 0xF8,
	NETWORK_SESSION_PEER_ADDRESS_OFFSET= 0,
	/* six bytes of what it is known by, which a console can be looked for
	by: where the console's libraries have those of its network card */
	NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET= 0xA,
	NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_SIZE= 6,
	NETWORK_SESSION_PEER_UNKNOWN28_OFFSET= 0x28,
	NETWORK_SESSION_PEER_UNKNOWN28_LENGTH= 16,
	NETWORK_SESSION_PEER_UNKNOWN48_OFFSET= 0x48,
	NETWORK_SESSION_PEER_UNKNOWN48_LENGTH= 32,
	NETWORK_SESSION_PEER_UNKNOWN88_OFFSET= 0x88,
	NETWORK_SESSION_PEER_UNKNOWN8C_OFFSET= 0x8C,
	NETWORK_SESSION_PEER_UNKNOWN90_OFFSET= 0x90,
	NETWORK_SESSION_PEER_UNKNOWN90_SIZE= 0xC,
	NETWORK_SESSION_PEER_UNKNOWNAC_OFFSET= 0xAC,
	NETWORK_SESSION_PEER_UNKNOWNAC_SIZE= 0x40,
	NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET= 0xEC,
	NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET= 0xF0,
	NETWORK_SESSION_PEER_PLAYERS_OFFSET= 0xFC,
	MAXIMUM_NETWORK_SESSION_PEER_PLAYERS= 4,
	/* a byte of the session that goes to what it keeps of a console that
	comes in, and two numbers that go with it: one of 0x4980 and a time */
	NETWORK_SESSION_UNKNOWN765C_OFFSET= 0x765C,
	NETWORK_SESSION_UNKNOWN7660_OFFSET= 0x7660,
	NETWORK_SESSION_UNKNOWN7664_OFFSET= 0x7664,
	NETWORK_SESSION_UNKNOWN44_OFFSET= 0x44,
	/* what the session has from its state on, which starts over in the
	fifth state */
	NETWORK_SESSION_STATE_DATA_OFFSET= 0x7420,
	NETWORK_SESSION_STATE_DATA_SIZE= 0x1F8,
	/* in who is in the session: a bit for each player that there is, and
	the players, which start with the twelve bytes that each is known by */
	NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET= 0x10D0,
	NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET= 0x10D4,
	NETWORK_SESSION_PLAYER_SIZE= 0x13C,
	NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE= 0xC,
	/* of a player: a number that starts at NONE, and two things of 0x90
	bytes that start empty */
	NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET= 0x14,
	NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET= 0x18,
	NETWORK_SESSION_PLAYER_UNKNOWNA8_OFFSET= 0xA8,
	NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE= 0x90,
	/* in what a console sends to answer: the two numbers that the session
	has at 0x4980 and 0x497C */
	NETWORK_SESSION_ANSWER_UNKNOWN8_OFFSET= 0x8,
	NETWORK_SESSION_ANSWER_UNKNOWNC_OFFSET= 0xC,
	/* how many players there are; and of a player, its console and which of
	the players of its console it is */
	NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET= 0x10CC,
	NETWORK_SESSION_PLAYER_PEER_OFFSET= 0xC,
	NETWORK_SESSION_PLAYER_PEER_PLAYER_OFFSET= 0x10,
	MAXIMUM_NETWORK_SESSION_PLAYERS= 16,
	/* in what the session was given when it was set up: things of 0x528
	bytes, one for each console that the session has a number of; each has a
	byte with a bit for each session that uses it */
	NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE= 0x528,
	NETWORK_SESSION_UNKNOWN8_CHANNEL_SESSIONS_OFFSET= 0xB1,
	/* the first thing of one of those: 7 when it can be sent things in the
	other way too */
	NETWORK_SESSION_UNKNOWN8_CHANNEL_STATE_OFFSET= 0xA8,
	_network_session_unknown8_channel_state7= 7,
	/* two kinds of what a console is sent before it is taken out of the
	session: what the session is known by goes with both */
	_network_session_message_unknownD= 0xD,
	_network_session_message_unknownE= 0xE,
	/* in its first state, the session keeps an address of the game */
	NETWORK_SESSION_STATE1_ADDRESS_OFFSET= 0x7448,
	/* the players that ask to come into the session, at 0x7668 */
	MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS= 16,
	/* and after the rest of the session: GetTickCount with the other sign,
	and the time of the C library */
	NETWORK_SESSION_UNKNOWN78B0_OFFSET= 0x78B0,
	NETWORK_SESSION_UNKNOWN78B4_OFFSET= 0x78B4,

	/* the key of a session, after what it is known by and whether it has
	them, and a number that goes with them: NONE with none. The sizes are in
	transport_security.h */
	NETWORK_SESSION_KEY_OFFSET= 0x25,
	NETWORK_SESSION_UNKNOWN3C_OFFSET= 0x3C,
	/* what XNetQosListen is told */
	_xnet_qos_listen_enable= 1,
	_xnet_qos_listen_release= 0x10
};

/* what a session keeps of each console that is in it, at 0x72DC, apart from
who is in it: whether there is one, three bytes, a number that it is given
(NONE for this console), two that start at NONE and when it came in */
struct network_session_peer_state
{
	boolean valid;
	boolean unknown1;
	boolean unknown2;
	boolean unknown3;
	long unknown4;
	long unknown8;
	long unknownC;
	long time;
};

typedef char verify_network_session_peer_state_size[
	sizeof(struct network_session_peer_state) == 0x14 ? 1 : -1];

/* a player that asks to come into a session: whether there is one; whether
the session has the player already; eight bytes of the console that asks;
what the player is known by; a number of the player; when it asked; and a
number of all those that asked with it */
struct network_session_player_request
{
	boolean valid;
	boolean has_player;
	byte unknown2[8];
	byte identifier[NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE];
	byte pad16[2];
	long unknown18;
	long time;
	long unknown20;
};

typedef char verify_network_session_player_request_size[
	sizeof(struct network_session_player_request) == 0x24 ? 1 : -1];
typedef char verify_network_session_player_requests_size[
	sizeof(struct network_session_player_request)*MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS ==
	NETWORK_SESSION_UNKNOWN7668_SIZE ? 1 : -1];

/* what a session has at 0x78A8, if it has it: something that is asked, by
its fourth function, whether a player can come in */
struct network_session_unknown78A8;

struct network_session_unknown78A8_functions
{
	void *unknown0;
	void *unknown4;
	void *unknown8;
	boolean (__attribute__((thiscall)) *player_can_join)(struct network_session_unknown78A8 *object,
		const byte *identifier);
};

struct network_session_unknown78A8
{
	const struct network_session_unknown78A8_functions *functions;
};

/* what a session knows of a console apart from what it is known by in the
network and its players: from 0x28 of the console. Two texts, and numbers */
struct network_session_peer_properties
{
	wchar unknown0[NETWORK_SESSION_PEER_UNKNOWN28_LENGTH];
	wchar unknown20[NETWORK_SESSION_PEER_UNKNOWN48_LENGTH];
	long unknown60;
	long unknown64;
	long unknown68;
	long unknown6C;
	long unknown70;
	long unknown74[4];
	/* these are aligned to four bytes */
	long unknown84[NETWORK_SESSION_PEER_UNKNOWNAC_SIZE/sizeof(long)];
	long unknownC4;
};

/* what has changed of that: each part of it goes after a byte that says
whether it has */
struct network_session_peer_properties_update
{
	boolean has_texts;
	wchar unknown0[NETWORK_SESSION_PEER_UNKNOWN28_LENGTH];
	wchar unknown20[NETWORK_SESSION_PEER_UNKNOWN48_LENGTH];
	boolean has_unknown60;
	long unknown60;
	long unknown64;
	boolean has_unknown68;
	long unknown68;
	long unknown6C;
	long unknown70;
	long unknown74[4];
	boolean has_unknown84;
	long unknown84[NETWORK_SESSION_PEER_UNKNOWNAC_SIZE/sizeof(long)];
	boolean has_unknownC4;
	long unknownC4;
};

typedef char verify_network_session_peer_properties_size[
	sizeof(struct network_session_peer_properties) == 0xC8 ? 1 : -1];
typedef char verify_network_session_peer_properties_end[
	NETWORK_SESSION_PEER_UNKNOWN28_OFFSET+sizeof(struct network_session_peer_properties) ==
	NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET ? 1 : -1];
typedef char verify_network_session_peer_properties_update_size[
	sizeof(struct network_session_peer_properties_update) == 0xD8 ? 1 : -1];
typedef char verify_network_session_peer_properties_update_unknown84_offset[
	offsetof(struct network_session_peer_properties_update, unknown84) == 0x90 ? 1 : -1];

/* a number that network_session_start leaves in a session */
#define NETWORK_SESSION_UNKNOWN4DA8_VALUE 0xDEADBEEFu

/* the states of a session. From the third one to the last, the session
says who is in it */
enum
{
	_network_session_state_none,
	_network_session_state1,
	_network_session_state2,
	_network_session_state3,
	_network_session_state4,
	_network_session_state5,
	_network_session_state6,
	_network_session_state7,
	_network_session_state8
};

/* a function that takes its arguments off the stack */
#define NETWORK_SESSION_CALLBACK __attribute__((stdcall))

/* ---------- prototypes/NETWORK_SESSION.C */

boolean network_session_unknown_00058d20(const byte *session);
boolean network_session_local_peer_is_host(const byte *session);
boolean network_session_established(const byte *session);
boolean network_session_unknown_00058d90(const byte *session);
boolean NETWORK_SESSION_CALLBACK network_session_initialize(byte *session, byte *unknown8, byte **table,
	long index, long unknown14, long unknown38, long unknown4);
boolean NETWORK_SESSION_CALLBACK network_session_start(byte *session, long unknown18, long unknown8, long unknownC,
	long unknown10, long player_count, const byte *identifiers, const long *unknown1C, long *unknown20,
	long unknown24);

void NETWORK_SESSION_CALLBACK network_session_peer_state_new(boolean unknown1, byte *session, long peer_index,
	long unknown4);
void NETWORK_SESSION_CALLBACK network_session_add_peer(byte *session, long peer_index, const byte *address,
	boolean unknown1, long unknown4, const long *unknownF0);
const byte *network_session_get_identifier(const byte *session);
boolean NETWORK_SESSION_CALLBACK network_session_get_security(const byte *session, byte *key, long *unknown38,
	long *unknown3C, byte *identifier);
byte *network_session_get_membership(long *local_peer, byte *session, long *host_peer);
long network_session_has_channel_peer(const byte *session, long channel_index);
boolean network_session_matches_state1_address(const byte *session, const struct transport_address *address);
void network_session_stop_waiting_if_done(byte *session);
boolean NETWORK_SESSION_CALLBACK network_session_peer_answered(byte *session, long unknown, const byte *answer);
void NETWORK_SESSION_CALLBACK network_session_add_player(const byte *identifier, byte *session, long peer_index,
	long player_index, long peer_player_index);
void network_session_remove_player_and_count(byte *session, long player_index);
long NETWORK_SESSION_CALLBACK network_session_add_peer_player(byte *session, long peer_index,
	long peer_player_index, const byte *identifier);
long network_session_free_player_places(const byte *session);
void NETWORK_SESSION_CALLBACK network_session_peer_properties_get_update(
	struct network_session_peer_properties_update *update, const struct network_session_peer_properties *properties,
	const struct network_session_peer_properties *baseline);
void network_session_remove_peer(byte *session, long peer_index);
void NETWORK_SESSION_CALLBACK network_session_remove_player(byte *session, long player_index);
boolean NETWORK_SESSION_CALLBACK network_session_find_request(byte *session, const byte *identifier,
	struct network_session_player_request **found);
void network_session_remove_peer_with_message_e(long peer_index, byte *session);
void network_session_remove_peer_with_message_d(long peer_index, byte *session);
boolean NETWORK_SESSION_CALLBACK network_session_channel_is_host(const byte *session, long unknown);
long network_session_find_peer_by_channel(const byte *session, long channel_index);
long network_session_find_peer_by_transport_address(const byte *session, const struct transport_address *address);
long NETWORK_SESSION_CALLBACK network_session_find_peer_by_unknownA(const byte *session, const byte *unknownA);
long NETWORK_SESSION_CALLBACK network_session_find_peer(const byte *session, const byte *address);
boolean network_session_address_is_host(const byte *session, const struct transport_address *address);
boolean NETWORK_SESSION_CALLBACK network_session_has_players_of(const byte *session, const byte *other_session);
long NETWORK_SESSION_CALLBACK network_session_find_player(const byte *session, const byte *identifier);
void network_session_peer_state_delete(byte *session, long peer_index);
void NETWORK_SESSION_CALLBACK network_session_reset_requests(byte *session, boolean unknown);
void network_session_enter_state5(byte *session);
boolean NETWORK_SESSION_CALLBACK network_session_request_players(const byte *identifiers, const long *unknown2,
	byte *session, long player_count, long unknown20, const long *unknown18s);
boolean NETWORK_SESSION_CALLBACK network_session_request_player(const byte *identifier, const long *unknown2,
	byte *session, long unknown20, long unknown18);
void network_session_forget_identifier(byte *session);
void network_session_set_identifier(byte *session, const byte *identifier, const byte *key, long unknown3C);
boolean NETWORK_SESSION_CALLBACK network_session_use_identifier(byte *session, const byte *key, long unknown8,
	long unknown18, const byte *identifier);
boolean NETWORK_SESSION_CALLBACK network_session_create_identifier(byte *session, long unknown8, long unknown18);

/* ---------- globals */

extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

#endif
