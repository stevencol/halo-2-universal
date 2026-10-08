/*
NETWORK_SESSION.H

A session of the network game: the consoles that play together, and their
players. Only a little of it is known. The name of the file is a guess, and
so are the names of its functions.
*/

#ifndef __NETWORK_SESSION_H
#define __NETWORK_SESSION_H

#include "cseries/cseries.h"
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
	characters of 16 bits, of 16 and of 32; four numbers that start at NONE;
	and two more that it comes in with or not */
	NETWORK_SESSION_PEER_ADDRESS_OFFSET= 0,
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
	NETWORK_SESSION_PEER_UNKNOWNFC_OFFSET= 0xFC,
	NETWORK_SESSION_PEER_UNKNOWNFC_COUNT= 4,
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
	MAXIMUM_NETWORK_SESSION_PLAYERS= 16,
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
	/* what unknown_003cd147 is told */
	_unknown_003cd147_start= 1,
	_unknown_003cd147_end= 0x10
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
long NETWORK_SESSION_CALLBACK network_session_find_player(const byte *session, const byte *identifier);
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
