/*
FONTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The fonts that the game has loaded. The names of the file and of the
functions are guesses.
*/

#include "fonts.h"

#include "cseries/async.h"
#include "main/global_preferences.h"
#include "text/draw_string.h"

#include <stddef.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d2084 */
int SwitchToThread(void);

/* 0x002d2750 */
uint32_t CreateFileA(const char *path, uint32_t access, uint32_t share_mode, void *security, uint32_t disposition,
	uint32_t flags, uint32_t template_file);

/* 0x002d2488 */
uint32_t SetFilePointer(uint32_t file, long distance, long *distance_high, uint32_t method);

/* 0x002d2241 */
int ReadFile(uint32_t file, void *buffer, uint32_t size, uint32_t *bytes_read, void *overlapped);

#define GENERIC_READ 0x80000000
#define OPEN_EXISTING 3
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_BEGIN 0
#define INVALID_HANDLE_VALUE (-1)

/* ---------- globals */

/* 0x004e2920 */
struct font font_slots[NUMBER_OF_FONT_SLOTS];

/* 0x004687f0: where the files of the fonts are */
const char *font_directory= "z:\\fonts\\";

/* ---------- code */

/* 0x001222d0: the work of the task that loads a font, which the thread of
the tasks does twice: the first time it opens the file of the font, and the
second it reads the start of it to the place of the font. If the file cannot
be opened, the font is not being loaded any more, and the task is done */
long ASYNC_CALLBACK font_load_work(
	void *parameters)
{
	const struct font_load_parameters *load= parameters;
	struct font *font= &font_slots[load->font_slot_index];
	boolean done= FALSE;

	if (font->file==(uint32_t)INVALID_HANDLE_VALUE)
	{
		char path[FONT_PATH_LENGTH];

		csnzprintf(path, sizeof(path), "%s%s", font_directory, load->name);
		font->file= CreateFileA(path, (uint32_t)GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
		if (font->file==(uint32_t)INVALID_HANDLE_VALUE)
		{
			font->loading= FALSE;
			done= TRUE;
		}
	}
	else
	{
		uint32_t bytes_read;

		SetFilePointer(font->file, FONT_FILE_HEADER_OFFSET, NULL, FILE_BEGIN);
		ReadFile(font->file, font, offsetof(struct font, file), &bytes_read, NULL);
		done= TRUE;
	}

	return done!=FALSE;
}

/* 0x001223a0: starts to load a font to a place: a task opens its file and
reads its start. If the game is to wait for it, it does, and then sees that
what was read is a font: if it is not, the preferences forget which fonts are
on the hard disk, and are written at once */
void font_load(
	long font_slot_index,
	const char *name,
	boolean wait)
{
	struct font_load_parameters parameters;
	struct font *font= &font_slots[font_slot_index];
	struct async_task *task;

	memset(&parameters, 0, sizeof(parameters));
	font->loading= TRUE;
	strncpy(parameters.name, name, sizeof(parameters.name));
	parameters.font_slot_index= font_slot_index;
	font->file= (uint32_t)INVALID_HANDLE_VALUE;
	parameters.name[sizeof(parameters.name)-1]= 0;

	/* async_task_add, which the game has in itself here */
	async_globals.tasks_added++;
	font->loaded= FALSE;
	task= async_task_allocate();
	memcpy(task->parameters, &parameters, sizeof(task->parameters));
	task->done= &font->loaded;
	task->function= font_load_work;
	task->category= FONT_TASK_CATEGORY;
	task->priority= wait ? FONT_LOAD_PRIORITY : FONT_LOAD_PRIORITY_NO_WAIT;
	font->load_task= async_task_queue(task);

	if (wait)
	{
		while (!*(volatile boolean *)&font->loaded)
		{
			SwitchToThread();
		}
		if (font->signature!=FONT_SIGNATURE)
		{
			global_preferences_forget_fonts();
		}
	}
}

/* 0x001224c0: the font of a place, once it is loaded: if it is being
loaded, the game hurries the task that loads it and waits for it. NULL if
there is none */
struct font *font_get(
	long font_slot_index)
{
	struct font *font= &font_slots[font_slot_index];

	if (font_slot_index<0 || font_slot_index>=NUMBER_OF_FONT_SLOTS)
	{
		return NULL;
	}
	if (!font->loaded && !font->loading)
	{
		return NULL;
	}
	if (!font->loaded)
	{
		async_task_set_priority(font->load_task, FONT_LOAD_PRIORITY);
		while (!*(volatile boolean *)&font->loaded)
		{
			SwitchToThread();
		}
	}

	return font;
}

/* 0x00122540: how high a line of a font is */
long font_get_line_height(
	long font_index)
{
	const struct font *font= font_get(font_slot_indices[font_index]);

	if (!font)
	{
		return FONT_DEFAULT_LINE_HEIGHT;
	}

	return font->leading_height+font->descending_height+font->ascending_height;
}

/* 0x00122570: how much nearer or farther a character is drawn when it is
after another, with a font: 0 if the font says nothing of the two. Only
characters of 8 bits have it */
short font_get_kerning(
	const struct font *font,
	unsigned long first,
	unsigned long second)
{
	long index= 0;

	if (!font || !first || !second || first>=NUMBER_OF_FONT_KERNING_CHARACTERS ||
		second>=NUMBER_OF_FONT_KERNING_CHARACTERS)
	{
		return 0;
	}
	if (!(font->kerning_characters[first>>5]&(1UL<<(first&31))))
	{
		return 0;
	}
	/* the first pair of the first character: the game looks at a pair
	before it sees whether there is one */
	while (font->kerning_pairs[index].first<first)
	{
		index++;
		if (index>=font->kerning_pair_count)
		{
			return 0;
		}
	}
	if (font->kerning_pairs[index].first!=first)
	{
		return 0;
	}
	while (font->kerning_pairs[index].second<second)
	{
		index++;
		if (index>=font->kerning_pair_count || font->kerning_pairs[index].first!=first)
		{
			return 0;
		}
	}
	if (font->kerning_pairs[index].second!=second)
	{
		return 0;
	}

	return font->kerning_pairs[index].offset;
}

/* a pixel of a character: 4 bits of how opaque it is, from 3, and a colour
of 12 bits */
static inline __attribute__((always_inline)) word font_pixel(
	unsigned long alpha,
	word color)
{
	return (word)((((alpha&1)|(alpha<<1))<<12)|color);
}

/* 0x00122610: the pixels of a character, from how they are in the file of a
font. How many there are: they are written only if there is where to.

Each byte is one of four things, by its two high bits: 0, so many pixels
that are clear, and with none, a new colour, which is in the two bytes that
follow and is a pixel as it is; 1, so many pixels that are opaque; 2, a pixel
of an opacity and a few after it, clear or opaque; 3, two pixels, each of its
opacity. The colour is white until another is set */
long font_character_decode_pixels(
	const byte *source,
	long size,
	word *pixels)
{
	word color= FONT_PIXEL_COLOR_MASK;
	long count= 0;

	while (size>0)
	{
		unsigned long code= *source;
		unsigned long kind= code>>6;
		long advance= 1;
		long run= 0;
		word pixel= 0;

		if (kind<=_font_pixels_opaque_run)
		{
			run= (long)(code&0x3F);
			if (!run && kind==_font_pixels_clear_run)
			{
				word value= (word)((source[1]<<8)|source[2]);

				color= value&FONT_PIXEL_COLOR_MASK;
				if (pixels)
				{
					*pixels++= value;
				}
				count++;
				advance= 3;
			}
			else
			{
				pixel= font_pixel(kind==_font_pixels_clear_run ? 0 : FONT_PIXEL_MAXIMUM_ALPHA, color);
			}
		}
		else if (kind==_font_pixels_one_and_run)
		{
			static const byte runs[8]= { 0, 4, 3, 2, 5, 4, 3, 2 };

			if (pixels)
			{
				*pixels++= font_pixel((code>>3)&FONT_PIXEL_MAXIMUM_ALPHA, color);
			}
			count++;
			run= runs[code&7];
			/* the first three are opaque, and the others clear */
			pixel= (code&7)>=1 && (code&7)<=3 ? (word)(color|0xF000) : color;
		}
		else
		{
			if (pixels)
			{
				*pixels++= font_pixel((code>>3)&FONT_PIXEL_MAXIMUM_ALPHA, color);
				*pixels++= font_pixel(*source&FONT_PIXEL_MAXIMUM_ALPHA, color);
			}
			count+= 2;
		}
		if (run>0)
		{
			if (pixels)
			{
				long index;

				for (index= 0; index<run; index++)
				{
					pixels[index]= pixel;
				}
				pixels+= run;
			}
			count+= run;
		}
		size-= advance;
		source+= advance;
	}

	return count;
}
