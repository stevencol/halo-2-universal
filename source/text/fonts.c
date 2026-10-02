/*
FONTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The fonts that the game has loaded. The names of the file and of the
functions are guesses.
*/

#include "fonts.h"

#include "cseries/async.h"
#include "text/draw_string.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x002d2084 */
int SwitchToThread(void);

/* ---------- globals */

/* 0x004e2920 */
struct font font_slots[NUMBER_OF_FONT_SLOTS];

/* ---------- code */

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
