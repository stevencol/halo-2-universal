/*
GAME_VOICE.H

What the game does with the voice of the players of the network game, and
the memory that the console's libraries of the sound ask for. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __GAME_VOICE_H
#define __GAME_VOICE_H

#include "cseries/cseries.h"
#include "memory/memory_pool.h"
#include "networking/voice.h"
#include "networking/voice_routing.h"

/* ---------- constants */

enum
{
	/* what is given to an allocation of no size */
	GAME_VOICE_DEFAULT_SIZE= 0x200,

	/* the pool of the voice, in a game that has it */
	GAME_VOICE_POOL_SIZE= 0x78000,
	/* the pool of the voice and of the menu, in a campaign in a session */
	GAME_VOICE_MENU_POOL_SIZE= 0x61800,

	/* what the allocation of the pool is told with: as that of the files of
	the players */
	GAME_VOICE_ALLOCATION_UNKNOWN= 2
};

/* how much of the voice a game has, by how its memory is shared out */
enum
{
	/* a campaign that is not in a session of the network game */
	_game_voice_none,
	/* a campaign in a session */
	_game_voice_session,
	/* anything but a campaign: the only one with the pool */
	_game_voice_full
};

enum
{
	/* the ways that the voice of a player can be changed: not at all, and
	so that it is not known */
	NUMBER_OF_GAME_VOICE_MASKS= 2,

	/* the players of a game: each has a bit in the numbers of 16 bits */
	MAXIMUM_GAME_VOICE_PLAYERS= 16,

	/* the twelve bytes that each of the sixteen players of the session is
	known by, as they were the last time */
	GAME_VOICE_SESSION_PLAYER_IDENTIFIERS_SIZE= 0xC0,

	/* the states of the network game (unknown_00527334) that the voice looks
	at, and what it makes of them (unknown_004c9888) */
	_network_game_state3= 3,
	_network_game_state7= 7,
	_network_game_state8= 8,
	_network_game_state_last= 9,
	_game_voice_network_none= 0,
	_game_voice_network1,
	_game_voice_network2,
	_game_voice_network3,

	/* in what unknown_000649c0 gives */
	UNKNOWN_000649C0_FLAGS_OFFSET= 0x48,

	/* what the game keeps of a profile (0x004cd904): whether it is there,
	the gamepad that has it, a part that is left as it is and the numbers of
	the voice of the gamepad's player */
	UNKNOWN_004CD904_SIZE= 0xD0,
	UNKNOWN_004CD904_COUNT= 4,
	UNKNOWN_004CD904_GAMEPAD_OFFSET= 0x10,
	UNKNOWN_004CD904_UNKNOWN14_OFFSET= 0x14,
	UNKNOWN_004CD904_UNKNOWN14_SIZE= 0x90,
	UNKNOWN_004CD904_VOICE_WORDS_OFFSET= 0xA4,
	/* in what the game knows of the player of a gamepad: which profile */
	UNKNOWN_0054E8E0_PROFILE_OFFSET= 4,

	/* in what unknown_004cf770 points to: something of each player */
	UNKNOWN_004CF770_PLAYER_OFFSET= 0x7C,
	UNKNOWN_004CF770_PLAYER_SIZE= 0x88,

	/* a player of player_data */
	GAME_VOICE_PLAYER_DATUM_SIZE= 0x21C,

	/* in the game's globals: flags of the kind of game. With the first one
	and what unknown_0055e4d0 has, game_voice_unknown_00053d40 says so */
	MAIN_GAME_GLOBALS_VARIANT_FLAGS_OFFSET= 0x184,
	/* in what unknown_004e9ae8 points to: which of unknown_0055e4d0 */
	UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET= 0xC14,
	UNKNOWN_0055E4D0_COUNT= 16,

	/* four things that the voice hands what the players say to, one after
	another in the game's memory: see voice_routing.h. The size of the last
	is a guess */
	UNKNOWN_005259B8_SIZE= 0x48,
	UNKNOWN_00525A00_SIZE= 0x1704,
	UNKNOWN_00527104_SIZE= 0x4,
	UNKNOWN_00527108_SIZE= 0xC4,
	UNKNOWN_00527108_PLAYERS_OFFSET= 0x44,

	/* in what the session says of who is in it: how many consoles, and the
	consoles */
	GAME_VOICE_MEMBERSHIP_PEER_COUNT_OFFSET= 0x8,
	GAME_VOICE_MEMBERSHIP_PEER_ARRAY_OFFSET= 0xC,

	/* of each console, in what the session says of who is in it: two
	numbers that have to be no less than unknown_004ce124 and
	unknown_004ce120, a number that is 3 for a console that cannot be chosen,
	and a bit for each console that it is joined to */
	GAME_VOICE_MEMBERSHIP_PEER_UNKNOWN9C_OFFSET= 0x9C,
	GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA0_OFFSET= 0xA0,
	GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA4_OFFSET= 0xA4,
	_game_voice_peer_unknownA4_3= 3,

	/* the players that the player of each gamepad wants not to hear: up to
	250, of 16 bytes each. A player is known by the first twelve */
	MAXIMUM_GAME_VOICE_MUTED_PLAYERS= 250,
	GAME_VOICE_MUTED_PLAYER_SIZE= 0x10,
	GAME_VOICE_PLAYER_IDENTIFIER_SIZE= 0xC,
	GAME_VOICE_MEMBERSHIP_PLAYER_IDENTIFIER_OFFSET= 0,

	/* of a text that game_voice_text_printf writes */
	GAME_VOICE_TEXT_SIZE= 0x80,

	/* in what the game knows of the player of a gamepad (unknown_0054e8e0):
	with this flag, what unknown_001906da says of the gamepad counts */
	UNKNOWN_0054E8E0_FLAGS_OFFSET= 0,
	_unknown_0054e8e0_unknown5_bit= 5
};

/* what the game knows of the voice of a player (0x004c9988): its player
can talk; it wants no voice (the third setting); and it is not allowed to
talk. The names are guesses, by what game_voice_update_local_player sets
them with */
enum
{
	_game_voice_player_can_talk_bit,
	_game_voice_player_voice_off_bit,
	_game_voice_player_not_allowed_bit
};

/* which of two things gives the session of the network game (0x004c988c) */
enum
{
	_game_voice_session_source1= 1,
	_game_voice_session_source2
};

enum
{
	/* in what network_life_cycle_get_session and network_life_cycle_get_other_session give: a number that is 0
	with no session, and what of it the voice is told of, which starts with
	a number that is NONE if there is none */
	GAME_VOICE_SESSION_UNKNOWN741C_OFFSET= 0x741C,
	GAME_VOICE_SESSION_MEMBERSHIP_OFFSET= 0x4C,
	/* in that: a bit for each player that there is */
	GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET= 0x10D0,
	/* and then the players: of each, the console that it is of and which of
	its controllers it has */
	GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET= 0x10D4,
	GAME_VOICE_MEMBERSHIP_PLAYER_SIZE= 0x13C,
	GAME_VOICE_MEMBERSHIP_PLAYER_PEER_OFFSET= 0xC,
	GAME_VOICE_MEMBERSHIP_PLAYER_CONTROLLER_OFFSET= 0x14,
	/* and its team, a byte with sign: NONE for none */
	GAME_VOICE_MEMBERSHIP_PLAYER_TEAM_OFFSET= 0x124,
	/* the consoles: a number of each */
	GAME_VOICE_MEMBERSHIP_PEER_SIZE= 0x10C,
	GAME_VOICE_MEMBERSHIP_PEER_UNKNOWNA8_OFFSET= 0xA8,
	/* in the session: this console, and another number */
	GAME_VOICE_SESSION_LOCAL_PEER_OFFSET= 0x72D8,
	GAME_VOICE_SESSION_UNKNOWN40_OFFSET= 0x40,
	/* the last of a player: its bits of the others, and what the game knows
	of its voice */
	GAME_VOICE_MEMBERSHIP_PLAYER_MUTES_OFFSET= 0x138,
	GAME_VOICE_MEMBERSHIP_PLAYER_FLAGS_OFFSET= 0x13A,
	/* in the session that network_life_cycle_get_current_session gives, and
	what it is when the session comes
	from the second place */
	UNKNOWN_000596D0_UNKNOWN14_OFFSET= 0x14,
	_unknown_000596d0_unknown14_2= 2,

	/* in what unknown_000649c0 gives, and what it is when
	game_voice_unknown_00053d90 says so */
	UNKNOWN_000649C0_STATE_OFFSET= 0x44,
	_unknown_000649c0_state7= 7,

	/* _control87: how the library of the voice wants the reals, and how the
	game has them */
	GAME_VOICE_FLOATING_POINT_CONTROL= 0x9001F,
	GAME_VOICE_FLOATING_POINT_LIBRARY_MASK= 0x8001F,
	GAME_VOICE_FLOATING_POINT_MASK= 0xFFFFF,
	/* of the register of the reals of the other kind: no error stops the
	program, and the errors that are noted */
	GAME_VOICE_MXCSR_EXCEPTION_MASKS= 0x1F80,
	GAME_VOICE_MXCSR_EXCEPTION_FLAGS= 0x3F
};

/* what a controller's player wants of the voice (0x004c99a8). What the
others are is not known */
enum
{
	_game_voice_controller_setting1= 1,
	_game_voice_controller_setting3= 3
};

/* as main_game.c calls those that take memory when a map is loaded, and as
voice.c calls what takes what a player said */
#define GAME_VOICE_CALLBACK __attribute__((stdcall))

/* ---------- structures */

/* what the memory of the pool is asked with: the functions, and a number
for them */
struct game_voice_allocation
{
	struct allocation allocation;
	long unknown;
};

#if defined(__i386__)
typedef char verify_game_voice_allocation_size[
	sizeof(struct game_voice_allocation) == 0x8 ? 1 : -1];
#endif

/* ---------- prototypes/GAME_VOICE.C */

void *game_voice_allocate(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) game_voice_free(void *address, unsigned long attributes);
void GAME_VOICE_CALLBACK game_voice_pool_new(long configuration);
void game_voice_pool_delete(void);
void game_voice_menu_pool_new(void);
void game_voice_menu_pool_delete(void);
void game_voice_start(void);
void game_voice_stop(void);
boolean game_voice_has_remote_talker(long identifier);
void game_voice_set_controller_mode(long controller_index, long mode);
long game_voice_get_controller_mode(long controller_index);
void game_voice_set_controller_mask(long mask, long controller_index);
boolean game_voice_player_has_muted(long player_index, long other_player_index);
void game_voice_remote_talker_data(long identifier, const void *data, long size);
void game_voice_play_message(long controller_index, const long *message, long size);
void game_voice_stop_message(long controller_index);
boolean game_voice_message_busy(long controller_index);
void game_voice_record_message(long controller_index, void *buffer, long *size, long unknown,
	unsigned long buffer_size, long *length);
void game_voice_stop_message_with_headset(long controller_index);
long game_voice_build_packet(long peer, byte *packet, long size);
boolean game_voice_player_can_be_heard(long player_index);
boolean game_voice_player_voice_off(long player_index);
boolean game_voice_player_not_allowed(long player_index);
unsigned long game_voice_player_flags(long player_index);
boolean game_voice_get_session(byte **session);
boolean game_voice_unknown_00053d90(void);
boolean game_voice_message_is_sound_file(const long *message, long size);
byte *game_voice_get_membership(void);
void game_voice_library_begin(void);
void game_voice_library_end(void);
void game_voice_update_session_source(void);
void game_voice_read_packet(const byte *packet, long size);
boolean game_voice_has_packet_for_peer(long peer);
long game_voice_packet_size_for_peer(long peer);
boolean game_voice_unknown_000549d0(void);
long game_voice_get_local_peer(void);
long game_voice_session_unknown40(void);
long game_voice_local_peer_unknownA8(void);
unsigned long game_voice_get_players(void);
byte *game_voice_get_player_array(void);
long game_voice_find_local_player(long controller_index);
long game_voice_player_controller(long player_index);
unsigned long game_voice_get_local_players(void);
boolean game_voice_controller_can_talk(long controller_index);
boolean game_voice_player_is_talking(long player_index);
boolean game_voice_unknown_00054ec0(void);
boolean game_voice_unknown_00054ef0(void);
long game_voice_get_session_source(void);
unsigned long game_voice_controller_player_words(long controller_index);
long game_voice_get_controller_setting(long controller_index);
void game_voice_set_controller_setting(long controller_index, long setting);
char *game_voice_text_printf(char *text, const char *format, ...);
void game_voice_library_update(void);
boolean game_voice_unknown_00053d40(void);
void game_voice_reset_players(void);
void game_voice_update_player(long player_index);
void game_voice_update_network_state(void);
boolean game_voice_unknown_00054df0(long player);
byte *game_voice_get_player_datum(long player);
void game_voice_update_profile(long controller_index);
void game_voice_update(void);
void GAME_VOICE_CALLBACK game_voice_local_data_ready(long controller_index, void *data, unsigned long size);
unsigned long game_voice_unknown_00053c70(void);
void game_voice_update_chosen_peer(void);
boolean game_voice_unknown_00054890(long peer);
void game_voice_update_players(void);
void GAME_VOICE_CALLBACK game_voice_update_local_player(long player_index);
void game_voice_update_session_players(void);
boolean game_voice_unknown_00054c70(void);
long game_voice_unknown_00054cc0(void);
unsigned long game_voice_get_talkers(void);

/* ---------- globals */

extern struct memory_pool *game_voice_pool;
extern struct memory_pool *game_voice_menu_pool;
extern boolean game_voice_use_menu_pool;
extern long game_voice_amount;
extern long game_voice_kind;
extern boolean game_voice_enabled;
extern long unknown_004c9888;
extern long game_voice_session_source;
extern long unknown_004c9878;
extern long unknown_004c987c;
extern long game_voice_session_players;
extern byte game_voice_muted_players[MAXIMUM_VOICE_CONTROLLERS][MAXIMUM_GAME_VOICE_MUTED_PLAYERS][GAME_VOICE_MUTED_PLAYER_SIZE];
extern long game_voice_muted_player_counts[MAXIMUM_VOICE_CONTROLLERS];
extern long unknown_004ce120;
extern long unknown_004ce124;
extern byte unknown_005259b8[UNKNOWN_005259B8_SIZE];
extern byte unknown_00527104[UNKNOWN_00527104_SIZE];
extern byte unknown_00527108[UNKNOWN_00527108_SIZE];
extern byte game_voice_session_player_identifiers[GAME_VOICE_SESSION_PLAYER_IDENTIFIERS_SIZE];
extern byte unknown_004cd904[UNKNOWN_004CD904_COUNT][UNKNOWN_004CD904_SIZE];
extern byte *unknown_004e9ae8;
extern long unknown_0055e4d0[UNKNOWN_0055E4D0_COUNT];
extern long unknown_004c9880;
extern long unknown_004c9884;
extern word game_voice_player_flag_words[MAXIMUM_GAME_VOICE_PLAYERS];
extern byte unknown_00525a00[UNKNOWN_00525A00_SIZE];
extern word game_voice_player_mutes[MAXIMUM_GAME_VOICE_PLAYERS];
extern long game_voice_controller_settings[MAXIMUM_VOICE_CONTROLLERS];
extern const real game_voice_masks[NUMBER_OF_GAME_VOICE_MASKS][NUMBER_OF_VOICE_MASK_VALUES];
extern word game_voice_talkers;
extern struct allocation *unknown_0046dd54;
extern struct game_voice_allocation game_voice_allocation;

#endif
