/*
VOICE.H

The voice of the players of the network game, over the console's library for
it. The name of the file is a guess, and so are the names of its functions.
*/

#ifndef __VOICE_H
#define __VOICE_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* one voice for each controller */
	MAXIMUM_VOICE_CONTROLLERS= 4,
	NUMBER_OF_VOICE_CONTROLLER_LEVELS= 4,
	/* the players of other consoles that the engine can have a voice for */
	MAXIMUM_VOICE_REMOTE_TALKERS= 30,
	/* what of the flags of two players has to be the same for them to be
	the same player */
	VOICE_TALKER_FLAGS_MASK= 3
};

/* what the voice is told that it is, by how much of it the game has */
enum
{
	_voice_kind_none,
	/* of a campaign in a session of the network game */
	_voice_kind_session,
	/* of anything but a campaign */
	_voice_kind_full
};

/* what the library does with a voice: each controller has one of them, and
the library is told which of them the game uses */
enum
{
	_voice_mode_unknown0,
	_voice_mode_unknown1,
	/* what each controller starts with; not used in a campaign */
	_voice_mode_unknown2,
	/* what each controller starts with in a campaign */
	_voice_mode_unknown3,
	NUMBER_OF_VOICE_MODES,

	/* those that the game uses: all of them, and in a campaign */
	VOICE_MODES_FULL= 0xF,
	VOICE_MODES_SESSION= 0xB
};

enum
{
	/* what the library is asked for, with a full voice */
	VOICE_ENGINE_FULL_UNKNOWN0= 15,
	VOICE_ENGINE_FULL_UNKNOWN8= 5,
	VOICE_ENGINE_UNKNOWN4= 4,
	VOICE_ENGINE_UNKNOWN14= 11,
	VOICE_ENGINE_UNKNOWN18= 32,

	/* what the library is told when the game uses _voice_mode_unknown2 */
	VOICE_ENGINE_UNKNOWN_00332E7C_VALUE= 15
};

/* what the library tells of what a player talks and listens with */
enum
{
	_voice_headset_plugged_in,
	_voice_headset_taken_out
};

enum
{
	/* what a function that the library calls says when it does nothing */
	VOICE_NOT_IMPLEMENTED= (long)0x80004001,

	/* a message that is played is a sound file: how it starts, and it is
	longer than this */
	VOICE_MESSAGE_FILE_TAG= 0x46464952, /* RIFF */
	VOICE_MESSAGE_FORM_TAG= 0x45564157, /* WAVE */
	VOICE_MESSAGE_FORMAT_TAG= 0x20746d66, /* fmt */
	VOICE_MESSAGE_MINIMUM_SIZE= 0x10,

	/* of the text that is made when the library does not take what a player
	of another console said */
	VOICE_TEXT_SIZE= 0x80
};

/* as the library calls the game */
#define VOICE_CALLBACK __attribute__((stdcall))

struct voice_globals;

/* ---------- structures */

/* what the console's library starts the voice with */
struct voice_engine_parameters
{
	/* both 0 without a full voice */
	long unknown0;
	long unknown4;
	long unknown8;
	long unknownC;
	/* the first thing of unknown_00510c90 */
	void *unknown10;
	long unknown14;
	long unknown18;
	long unknown1C;
	long unknown20;
};

/* 0x00476fc8 */
struct voice_globals
{
	const struct voice_callbacks *callbacks;
	boolean initialized;
	byte pad5[3];
	long kind;
	/* a bit for each of the modes that the game uses */
	unsigned long modes;
	/* of the console's library; NULL if there is none */
	void *engine;
	/* -1 when the voice starts */
	real controller_levels[MAXIMUM_VOICE_CONTROLLERS][NUMBER_OF_VOICE_CONTROLLER_LEVELS];
	long controller_modes[MAXIMUM_VOICE_CONTROLLERS];
	/* the controller has what a player talks and listens with: the first
	thing that the library says of it is 0, when the voice starts, and the
	library tells when it is plugged in and taken out */
	boolean controller_has_headset[MAXIMUM_VOICE_CONTROLLERS];
	/* where the size and the length of the message that the controller
	records are left when it is done; NULL if nobody wants them */
	long *controller_message_size[MAXIMUM_VOICE_CONTROLLERS];
	long *controller_message_length[MAXIMUM_VOICE_CONTROLLERS];
	/* the controller records a message or plays one */
	boolean controller_message_busy[MAXIMUM_VOICE_CONTROLLERS];
	/* the library gave something that the controller's player said */
	boolean controller_talked[MAXIMUM_VOICE_CONTROLLERS];
};

/* the functions that the library tells the game what happens with: the
first thing of voice_globals points to them. Each gets the voice first */
struct voice_callbacks
{
	long (VOICE_CALLBACK *data_ready)(struct voice_globals *voice, long controller_index, unsigned long size,
		void *data);
	long (VOICE_CALLBACK *headset_changed)(struct voice_globals *voice, long controller_index, long change);
	long (VOICE_CALLBACK *unknown8)(struct voice_globals *voice, long unknown4, long unknown8, long unknownC);
	long (VOICE_CALLBACK *unknownC)(struct voice_globals *voice, long unknown4, long unknown8, long unknownC,
		long unknown10);
	long (VOICE_CALLBACK *message_recorded)(struct voice_globals *voice, long controller_index, long length,
		long size);
	long (VOICE_CALLBACK *message_stopped)(struct voice_globals *voice, long controller_index);
};

/* a player of another console, as the console's libraries know one: a
number of 64 bits and flags. The game makes it from a number of 32 bits,
with the sign, and no flags */
struct voice_talker
{
	long long identifier;
	unsigned long flags;
};

/* what the library says of a controller. Only the first thing of it is
looked at */
struct voice_controller_status
{
	long unknown0;
	long unknown4;
	long unknown8;
};

typedef char verify_voice_engine_parameters_size[
	sizeof(struct voice_engine_parameters) == (sizeof(void *) == 4 ? 0x24 : 0x28) ? 1 : -1];
#if defined(__i386__)
typedef char verify_voice_talker_size[
	sizeof(struct voice_talker) == 0xC ? 1 : -1];
typedef char verify_voice_globals_size[
	sizeof(struct voice_globals) == 0x90 ? 1 : -1];
typedef char verify_voice_globals_controller_modes_offset[
	offsetof(struct voice_globals, controller_modes) == 0x54 ? 1 : -1];
typedef char verify_voice_globals_controller_message_busy_offset[
	offsetof(struct voice_globals, controller_message_busy) == 0x88 ? 1 : -1];
typedef char verify_voice_callbacks_size[
	sizeof(struct voice_callbacks) == 0x18 ? 1 : -1];
#endif

/* ---------- prototypes/VOICE.C */

void voice_reset_controller_levels(struct voice_globals *voice);
void voice_reset_controllers(struct voice_globals *voice);
boolean voice_engine_parameters_new(struct voice_engine_parameters *parameters, const struct voice_globals *voice);
boolean voice_start(struct voice_globals *voice);
void voice_initialize(struct voice_globals *voice, long kind);
void voice_dispose(struct voice_globals *voice);
boolean voice_has_remote_talker(const struct voice_globals *voice, long identifier);
boolean voice_remote_talker_unknown_000551c0(const struct voice_globals *voice, long identifier);
void voice_update_controller_levels(struct voice_globals *voice, long controller_index, real *levels);
void voice_remote_talker_unknown_00055250(const struct voice_globals *voice, long identifier, long unknown4,
	void *unknown8);
void voice_remove_remote_talkers(struct voice_globals *voice);
boolean voice_remote_talker_set(const struct voice_globals *voice, long identifier, boolean has_voice);
void voice_play_message(const long *message, boolean unknown, long size, long controller_index,
	struct voice_globals *voice);
void voice_record_message(unsigned long buffer_size, long controller_index, struct voice_globals *voice,
	long unknown, void *buffer, long *size, long *length);
void voice_remote_talker_data(const void *data, long size, const struct voice_globals *voice, long identifier);
char *voice_text_append(char *text, const char *format, ...);
long VOICE_CALLBACK voice_callback_headset_changed(struct voice_globals *voice, long controller_index, long change);
long VOICE_CALLBACK voice_callback_data_ready(struct voice_globals *voice, long controller_index,
	unsigned long size, void *data);
long VOICE_CALLBACK voice_callback_unknownC(struct voice_globals *voice, long unknown4, long unknown8,
	long unknownC, long unknown10);
long VOICE_CALLBACK voice_callback_unknown8(struct voice_globals *voice, long unknown4, long unknown8,
	long unknownC);
long VOICE_CALLBACK voice_callback_message_recorded(struct voice_globals *voice, long controller_index,
	long length, long size);
long VOICE_CALLBACK voice_callback_message_stopped(struct voice_globals *voice, long controller_index);

/* ---------- globals */

extern const struct voice_callbacks voice_callbacks;
extern struct voice_globals voice_globals;
extern void *voice_modes[NUMBER_OF_VOICE_MODES];

#endif
