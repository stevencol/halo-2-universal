/*
VOICE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The voice of the players of the network game. The name of the file is a
guess, and so are the names of its functions.
*/

#include "voice.h"

#include "memory/crc.h"
#include "networking/game_voice.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* the library of the voice. What each does is a guess, by how the game
calls it; all of them but the last two say whether they could, as a number
that is less than 0 if not */

/* 0x00333055: makes the engine of the voice */
long unknown_00333055(const struct voice_engine_parameters *parameters, void **engine);

/* 0x00332d8f: the game uses a mode */
long unknown_00332d8f(void *engine, void *mode);

/* 0x00332d94: who the engine tells what happens */
long unknown_00332d94(void *engine, struct voice_globals *voice);

/* 0x00332e7c */
long unknown_00332e7c(void *engine, long value);

/* 0x00332da3, 0x00332f27: a controller has a voice from now on, and it has
none */
long unknown_00332da3(void *engine, long controller_index);
long unknown_00332f27(void *engine, long controller_index);

/* 0x00332da8 (in voice.h): the mode of the voice of a controller */

/* 0x00332db2: what the engine knows of the voice of a controller */
long unknown_00332db2(void *engine, long controller_index, struct voice_controller_status *status);

/* 0x003330f8: lets go of the engine */
long unknown_003330f8(void *engine);

/* 0x00332def: the players of other consoles that have a voice, and how
many: there is room for MAXIMUM_VOICE_REMOTE_TALKERS */
long unknown_00332def(void *engine, unsigned long *count, struct voice_talker *talkers);

/* 0x00332db7, 0x00332dd3: a player of another console has a voice from now
on, and has none */
long unknown_00332db7(void *engine, struct voice_talker talker);
long unknown_00332dd3(void *engine, struct voice_talker talker);

/* 0x00332e18: whether a player of another console is talking */
long unknown_00332e18(void *engine, struct voice_talker talker);

/* 0x00332e34: something for a player of another console */
long unknown_00332e34(void *engine, struct voice_talker talker, long unknown4, void *unknown8);

/* 0x00332dad: what the voice of a controller's player is changed with */
long unknown_00332dad(void *engine, long controller_index, const real *mask);

/* 0x00332e93 (in voice.h): a controller stops the message that it records
or plays */

/* 0x00332df4: what a player of another console said, as it came from the
network */
long unknown_00332df4(void *engine, struct voice_talker talker, const void *data, long size);

/* 0x00332e8a, 0x00332e81: a controller plays a message, and records one */
long unknown_00332e8a(void *engine, long controller_index, long size, const void *message, unsigned long unknown);
long unknown_00332e81(void *engine, long controller_index, long unknown, unsigned long buffer_size, void *buffer);


/* ---------- globals */

/* 0x0045095c */
const struct voice_callbacks voice_callbacks=
{
	voice_callback_data_ready,
	voice_callback_headset_changed,
	voice_callback_unknown8,
	voice_callback_unknownC,
	voice_callback_message_recorded,
	voice_callback_message_stopped
};

/* 0x00476fc8 */
struct voice_globals voice_globals=
{
	&voice_callbacks
};

/* 0x00510c90, of sound_manager.c: what the game knows of the effects of the
processor of the console's sound. The first thing of it is what the
console's library needs of them for the voice; NULL if there is none */
extern byte *unknown_00510c90;

/* 0x0052731c: what the library knows each mode by */
void *voice_modes[NUMBER_OF_VOICE_MODES];

/* ---------- code */

/* whether the voice has players of other consoles: only one that uses
_voice_mode_unknown2 does */
static inline __attribute__((always_inline)) boolean voice_has_remote_talkers(
	const struct voice_globals *voice)
{
	return voice->initialized && (voice->modes&(1u<<_voice_mode_unknown2));
}

/* the controller after one, and the first after NONE; NONE after the last.
The game has this in each loop over the controllers */
static inline __attribute__((always_inline)) long voice_next_controller(
	long controller_index)
{
	switch (controller_index)
	{
	case NONE:
		return 0;
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 3;
	}

	return NONE;
}

/* 0x000556a0: the voice of no controller is changed */
void voice_reset_controller_masks(
	struct voice_globals *voice)
{
	long controller_index;

	for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
	{
		voice->controller_masks[controller_index][3]= -1.0f;
		voice->controller_masks[controller_index][2]= -1.0f;
		voice->controller_masks[controller_index][1]= -1.0f;
		voice->controller_masks[controller_index][0]= -1.0f;
	}
}

/* 0x00055720: the mode that each controller starts with, by what the voice is */
void voice_reset_controllers(
	struct voice_globals *voice)
{
	long mode= voice->kind==_voice_kind_session ? _voice_mode_unknown3 : _voice_mode_unknown2;
	long controller_index;

	for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
	{
		voice->controller_modes[controller_index]= mode;
	}
}

/* 0x00055780: what the console's library is asked to start the voice with;
FALSE, and nothing written, if what it needs is not there */
boolean voice_engine_parameters_new(
	struct voice_engine_parameters *parameters,
	const struct voice_globals *voice)
{
	void *unknown= *(void **)unknown_00510c90;
	long unknown0= 0;
	long unknown8= 0;

	if (!unknown)
	{
		return FALSE;
	}

	memset(parameters, 0, sizeof(*parameters));
	if (voice->kind==_voice_kind_full)
	{
		unknown0= VOICE_ENGINE_FULL_UNKNOWN0;
		unknown8= VOICE_ENGINE_FULL_UNKNOWN8;
	}
	parameters->unknown0= unknown0;
	parameters->unknown4= VOICE_ENGINE_UNKNOWN4;
	parameters->unknown8= unknown8;
	parameters->unknownC= 0;
	parameters->unknown10= unknown;
	parameters->unknown14= VOICE_ENGINE_UNKNOWN14;
	parameters->unknown18= VOICE_ENGINE_UNKNOWN18;
	parameters->unknown1C= 0;
	parameters->unknown20= 0;

	return TRUE;
}

/* 0x00055810: makes the engine of the voice and tells it the modes that the
game uses, who it tells what happens, and that each controller has a voice,
in the mode it starts with. FALSE only if there was nothing to ask the
engine with: if the library says that it could not do something, the rest is
not done, and the voice is taken as started all the same */
boolean voice_start(
	struct voice_globals *voice)
{
	struct voice_engine_parameters parameters;
	struct voice_controller_status status;
	boolean started= voice_engine_parameters_new(&parameters, voice);

	if (started)
	{
		long result= unknown_00333055(&parameters, &voice->engine);

		if (result>=0)
		{
			unsigned long mode;

			voice->modes= 0;
			voice->modes= voice->kind==_voice_kind_session ? VOICE_MODES_SESSION : VOICE_MODES_FULL;
			for (mode= 0; mode<NUMBER_OF_VOICE_MODES; mode++)
			{
				if (voice->modes&(1u<<mode))
				{
					result= unknown_00332d8f(voice->engine, voice_modes[mode]);
				}
			}
			if (result>=0)
			{
				result= unknown_00332d94(voice->engine, voice);
				if (voice->modes&(1u<<_voice_mode_unknown2))
				{
					unknown_00332e7c(voice->engine, VOICE_ENGINE_UNKNOWN_00332E7C_VALUE);
				}
				if (result>=0)
				{
					long controller_index;

					for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
					{
						unknown_00332da3(voice->engine, controller_index);
						unknown_00332da8(voice->engine, controller_index,
							voice_modes[voice->controller_modes[controller_index]]);
						unknown_00332db2(voice->engine, controller_index, &status);
						voice->controller_has_headset[controller_index]= status.unknown0==0;
						voice->controller_talked[controller_index]= FALSE;
						voice->controller_message_busy[controller_index]= FALSE;
					}
					started= TRUE;
				}
			}
		}
	}

	return started;
}

/* 0x00055080: starts a voice of a kind, if it is not started */
void voice_initialize(
	struct voice_globals *voice,
	long kind)
{
	if (!voice->initialized)
	{
		voice->engine= NULL;
		voice->kind= kind;
		voice_reset_controller_masks(voice);
		voice_reset_controllers(voice);
		voice->initialized= voice_start(voice);
	}
}

/* 0x000550b0: stops the voice, if it was started: no controller has a
voice, and the engine is let go of */
void voice_dispose(
	struct voice_globals *voice)
{
	if (voice->initialized)
	{
		long controller_index;

		for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
		{
			unknown_00332f27(voice->engine, controller_index);
		}
		if (voice->modes&(1u<<_voice_mode_unknown2))
		{
			voice_remove_remote_talkers(voice);
		}
		if (voice->engine)
		{
			unknown_003330f8(voice->engine);
			voice->engine= NULL;
		}
	}
	voice->initialized= FALSE;
}

/* 0x00055130: whether a player of another console has a voice */
boolean voice_has_remote_talker(
	const struct voice_globals *voice,
	long identifier)
{
	boolean found= FALSE;

	if (voice_has_remote_talkers(voice))
	{
		unsigned long count;
		struct voice_talker talkers[MAXIMUM_VOICE_REMOTE_TALKERS];

		if (unknown_00332def(voice->engine, &count, talkers)>=0)
		{
			struct voice_talker talker= voice_talker_new(identifier);
			unsigned long index;

			for (index= 0; index<count && !found; index++)
			{
				found= talkers[index].identifier==talker.identifier &&
					!((talkers[index].flags^talker.flags)&VOICE_TALKER_FLAGS_MASK);
			}
		}
	}

	return found;
}

/* 0x000551c0: whether a player of another console is talking, as the
library says */
boolean voice_remote_talker_is_talking(
	const struct voice_globals *voice,
	long identifier)
{
	boolean result= FALSE;

	if (voice_has_remote_talkers(voice))
	{
		result= unknown_00332e18(voice->engine, voice_talker_new(identifier))!=0;
	}

	return result;
}

/* 0x00055210: tells the library what to change the voice of a controller's
player with, and keeps it if the library takes it */
void voice_set_controller_mask(
	struct voice_globals *voice,
	long controller_index,
	const real *mask)
{
	if (voice->initialized)
	{
		if (unknown_00332dad(voice->engine, controller_index, mask)>=0)
		{
			memcpy(voice->controller_masks[controller_index], mask, sizeof(voice->controller_masks[controller_index]));
		}
	}
}

/* 0x00055250: something that the library is told of a player of another
console */
void voice_remote_talker_unknown_00055250(
	const struct voice_globals *voice,
	long identifier,
	long unknown4,
	void *unknown8)
{
	if (voice->initialized)
	{
		unknown_00332e34(voice->engine, voice_talker_new(identifier), unknown4, unknown8);
	}
}

/* 0x000552a0: no player of another console has a voice. Each is known here
by the low half of its number: the game has voice_remote_talker_set in the
loop itself */
void voice_remove_remote_talkers(
	struct voice_globals *voice)
{
	if (voice_has_remote_talkers(voice))
	{
		unsigned long count;
		struct voice_talker talkers[MAXIMUM_VOICE_REMOTE_TALKERS];

		if (unknown_00332def(voice->engine, &count, talkers)>=0)
		{
			unsigned long index;

			for (index= 0; index<count; index++)
			{
				voice_remote_talker_set(voice, (long)talkers[index].identifier, FALSE);
			}
		}
	}
}

/* 0x00055330: a player of another console has a voice from now on, or has
none; whether the library could */
boolean voice_remote_talker_set(
	const struct voice_globals *voice,
	long identifier,
	boolean has_voice)
{
	if (voice_has_remote_talkers(voice))
	{
		if (has_voice)
		{
			if (unknown_00332db7(voice->engine, voice_talker_new(identifier))>=0)
			{
				return TRUE;
			}
		}
		else
		{
			if (unknown_00332dd3(voice->engine, voice_talker_new(identifier))>=0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x000554c0: a controller plays a message, which is a sound file. Only one
in _voice_mode_unknown3 does, and only with a headset, if the message is not
to be played without one */
void voice_play_message(
	const long *message,
	boolean unknown,
	long size,
	long controller_index,
	struct voice_globals *voice)
{
	if (voice->initialized && voice->controller_modes[controller_index]==_voice_mode_unknown3 &&
		size>VOICE_MESSAGE_MINIMUM_SIZE && message[0]==VOICE_MESSAGE_FILE_TAG && message[2]==VOICE_MESSAGE_FORM_TAG &&
		message[3]==VOICE_MESSAGE_FORMAT_TAG && (unknown || voice->controller_has_headset[controller_index]))
	{
		if (unknown_00332e8a(voice->engine, controller_index, size, message, unknown)>=0)
		{
			voice->controller_message_busy[controller_index]= TRUE;
		}
	}
}

/* 0x00055520: a controller records a message in a buffer, which is emptied
first. Its size and its length are 0 until the library says that the message
is done (voice_callback_message_recorded). Only a controller in
_voice_mode_unknown3 and with a headset does */
void voice_record_message(
	unsigned long buffer_size,
	long controller_index,
	struct voice_globals *voice,
	long unknown,
	void *buffer,
	long *size,
	long *length)
{
	*size= 0;
	*length= 0;
	memset(buffer, 0, buffer_size);
	if (voice->initialized && voice->controller_modes[controller_index]==_voice_mode_unknown3 &&
		voice->controller_has_headset[controller_index])
	{
		voice->controller_message_size[controller_index]= NULL;
		voice->controller_message_length[controller_index]= NULL;
		if (unknown_00332e81(voice->engine, controller_index, unknown, buffer_size, buffer)>=0)
		{
			voice->controller_message_size[controller_index]= size;
			voice->controller_message_length[controller_index]= length;
			voice->controller_message_busy[controller_index]= TRUE;
		}
	}
}

/* 0x000555b0: a headset was plugged into a controller or taken out of it */
long VOICE_CALLBACK voice_callback_headset_changed(
	struct voice_globals *voice,
	long controller_index,
	long change)
{
	if (change==_voice_headset_plugged_in)
	{
		voice->controller_has_headset[controller_index]= TRUE;
	}
	else if (change==_voice_headset_taken_out)
	{
		voice->controller_has_headset[controller_index]= FALSE;
	}

	return 0;
}

/* 0x000555f0: the library has something that the player of a controller
said */
long VOICE_CALLBACK voice_callback_data_ready(
	struct voice_globals *voice,
	long controller_index,
	unsigned long size,
	void *data)
{
	if (size>0)
	{
		voice->controller_talked[controller_index]= TRUE;
		game_voice_local_data_ready(controller_index, data, size);
	}

	return 0;
}

/* 0x00055620 */
long VOICE_CALLBACK voice_callback_unknownC(
	struct voice_globals *voice,
	long unknown4,
	long unknown8,
	long unknownC,
	long unknown10)
{
	(void)voice;
	(void)unknown4;
	(void)unknown8;
	(void)unknownC;
	(void)unknown10;

	return VOICE_NOT_IMPLEMENTED;
}

/* 0x00055630 */
long VOICE_CALLBACK voice_callback_unknown8(
	struct voice_globals *voice,
	long unknown4,
	long unknown8,
	long unknownC)
{
	(void)voice;
	(void)unknown4;
	(void)unknown8;
	(void)unknownC;

	return VOICE_NOT_IMPLEMENTED;
}

/* 0x00055640: the message that a controller recorded is done: its size and
its length go where voice_record_message was told */
long VOICE_CALLBACK voice_callback_message_recorded(
	struct voice_globals *voice,
	long controller_index,
	long length,
	long size)
{
	if (voice->controller_message_size[controller_index])
	{
		*voice->controller_message_size[controller_index]= size;
	}
	if (voice->controller_message_length[controller_index])
	{
		*voice->controller_message_length[controller_index]= length;
	}
	voice->controller_message_busy[controller_index]= FALSE;

	return 0;
}

/* 0x00055680: the message that a controller played is over */
long VOICE_CALLBACK voice_callback_message_stopped(
	struct voice_globals *voice,
	long controller_index)
{
	voice->controller_message_busy[controller_index]= FALSE;

	return 0;
}

/* 0x000553b0: gives the library what a player of another console said. If
the library does not take it, the game makes its checksum and a text with
the mode of each controller, and does nothing with them: what told of it is
not in this build */
void voice_remote_talker_data(
	const void *data,
	long size,
	const struct voice_globals *voice,
	long identifier)
{
	if (voice->initialized)
	{
		if (unknown_00332df4(voice->engine, voice_talker_new(identifier), data, size)<0)
		{
			unsigned long checksum;
			char text[VOICE_TEXT_SIZE];
			long controller_index;

			text[0]= 0;
			checksum= 0xFFFFFFFF;
			crc_checksum_buffer(data, size, &checksum);
			strncpy(text, "voice_modes:", VOICE_TEXT_SIZE);
			text[VOICE_TEXT_SIZE-1]= 0;
			for (controller_index= 0; controller_index!=NONE; controller_index= voice_next_controller(controller_index))
			{
				voice_text_append(text, "(c#%d %d)", controller_index,
					voice->initialized ? voice->controller_modes[controller_index] : 0);
			}
		}
	}
}

/* 0x00055920: writes formatted text after what a text of VOICE_TEXT_SIZE
has; the text */
char *voice_text_append(
	char *text,
	const char *format,
	...)
{
	unsigned long length= 0;
	unsigned long size;
	char *end;
	va_list arguments;

	while (text[length])
	{
		length++;
		if (length>=VOICE_TEXT_SIZE-1)
		{
			break;
		}
	}
	size= VOICE_TEXT_SIZE-length;
	end= text+length;
	va_start(arguments, format);
	vsnprintf(end, size-1, format, arguments);
	va_end(arguments);
	end[size-1]= 0;

	return text;
}
