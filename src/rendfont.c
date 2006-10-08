/***************************************************************************

    rendfont.c

    Rendering system font management.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "rendfont.h"
#include "rendutil.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define FONT_FORMAT_TEXT		1
#define FONT_FORMAT_BINARY		2



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a render_font contains information about a single character in a font */
typedef struct _render_font_char render_font_char;
struct _render_font_char
{
	INT32				width;				/* width from this character to the next */
	INT32				xoffs, yoffs;		/* X and Y offset from baseline to top,left of bitmap */
	INT32				bmwidth, bmheight;	/* width and height of bitmap */
	const char *		rawdata;			/* pointer to the raw data for this one */
	mame_bitmap *		bitmap;				/* pointer to the bitmap containing the raw data */
	render_texture *	texture;			/* pointer to a texture for rendering and sizing */
};


/* a render_font contains information about a font */
/*typedef struct _render_font render_font; -- defined in rendfont.h */
struct _render_font
{
	int					format;				/* format of font data */
	int					height;				/* height of the font, from ascent to descent */
	int					yoffs;				/* y offset from baseline to descent */
	float				scale;				/* 1 / height precomputed */
	render_font_char *	chars[256];			/* array of character subtables */
	const char *		rawdata;			/* pointer to the raw data for the font */
	UINT64				rawsize;			/* size of the raw font data */
};



/***************************************************************************
    BUILT-IN FONT DATA
***************************************************************************/

static const UINT8 uifontdata[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* [ 0- 1] */
	0x7c,0x80,0x98,0x90,0x80,0xbc,0x80,0x7c,0xf8,0x04,0x64,0x44,0x04,0xf4,0x04,0xf8,	/* [ 2- 3] tape pos 1 */
	0x7c,0x80,0x98,0x88,0x80,0xbc,0x80,0x7c,0xf8,0x04,0x64,0x24,0x04,0xf4,0x04,0xf8,	/* [ 4- 5] tape pos 2 */
	0x7c,0x80,0x88,0x98,0x80,0xbc,0x80,0x7c,0xf8,0x04,0x24,0x64,0x04,0xf4,0x04,0xf8,	/* [ 6- 7] tape pos 3 */
	0x7c,0x80,0x90,0x98,0x80,0xbc,0x80,0x7c,0xf8,0x04,0x44,0x64,0x04,0xf4,0x04,0xf8,	/* [ 8- 9] tape pos 3 */
	0x30,0x48,0x84,0xb4,0xb4,0x84,0x48,0x30,0x30,0x48,0x84,0x84,0x84,0x84,0x48,0x30,	/* [10-11] */
	0x00,0xfc,0x84,0x8c,0xd4,0xa4,0xfc,0x00,0x00,0xfc,0x84,0x84,0x84,0x84,0xfc,0x00,	/* [12-13] */
	0x00,0x38,0x7c,0x7c,0x7c,0x38,0x00,0x00,0x00,0x30,0x68,0x78,0x78,0x30,0x00,0x00,	/* [14-15] circle & bullet */
	0x80,0xc0,0xe0,0xf0,0xe0,0xc0,0x80,0x00,0x04,0x0c,0x1c,0x3c,0x1c,0x0c,0x04,0x00,	/* [16-17] R/L triangle */
	0x20,0x70,0xf8,0x20,0x20,0xf8,0x70,0x20,0x48,0x48,0x48,0x48,0x48,0x00,0x48,0x00,
	0x00,0x00,0x30,0x68,0x78,0x30,0x00,0x00,0x00,0x30,0x68,0x78,0x78,0x30,0x00,0x00,
	0x70,0xd8,0xe8,0xe8,0xf8,0xf8,0x70,0x00,0x1c,0x7c,0x74,0x44,0x44,0x4c,0xcc,0xc0,
	0x20,0x70,0xf8,0x70,0x70,0x70,0x70,0x00,0x70,0x70,0x70,0x70,0xf8,0x70,0x20,0x00,
	0x00,0x10,0xf8,0xfc,0xf8,0x10,0x00,0x00,0x00,0x20,0x7c,0xfc,0x7c,0x20,0x00,0x00,
	0xb0,0x54,0xb8,0xb8,0x54,0xb0,0x00,0x00,0x00,0x28,0x6c,0xfc,0x6c,0x28,0x00,0x00,
	0x00,0x30,0x30,0x78,0x78,0xfc,0x00,0x00,0xfc,0x78,0x78,0x30,0x30,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,
	0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xf8,0x50,0xf8,0x50,0x00,0x00,
	0x20,0x70,0xc0,0x70,0x18,0xf0,0x20,0x00,0x40,0xa4,0x48,0x10,0x20,0x48,0x94,0x08,
	0x60,0x90,0xa0,0x40,0xa8,0x90,0x68,0x00,0x10,0x20,0x40,0x00,0x00,0x00,0x00,0x00,
	0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x00,0x10,0x08,0x08,0x08,0x08,0x08,0x10,0x00,
	0x20,0xa8,0x70,0xf8,0x70,0xa8,0x20,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,
	0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,
	0x70,0x88,0x08,0x10,0x20,0x40,0xf8,0x00,0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
	0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,0xf8,0x80,0xf0,0x08,0x08,0x88,0x70,0x00,
	0x70,0x80,0xf0,0x88,0x88,0x88,0x70,0x00,0xf8,0x08,0x08,0x10,0x20,0x20,0x20,0x00,
	0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x70,0x88,0x88,0x88,0x78,0x08,0x70,0x00,
	0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x60,
	0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,
	0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,
	0x30,0x48,0x94,0xa4,0xa4,0x94,0x48,0x30,0x70,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,
	0xf0,0x88,0x88,0xf0,0x88,0x88,0xf0,0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,
	0xf0,0x88,0x88,0x88,0x88,0x88,0xf0,0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,
	0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x80,0x98,0x88,0x88,0x70,0x00,
	0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
	0x08,0x08,0x08,0x08,0x88,0x88,0x70,0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,
	0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x88,0xd8,0xa8,0x88,0x88,0x88,0x88,0x00,
	0x88,0xc8,0xa8,0x98,0x88,0x88,0x88,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
	0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x08,
	0xf0,0x88,0x88,0xf0,0x88,0x88,0x88,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,
	0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
	0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00,0x88,0x88,0x88,0x88,0xa8,0xd8,0x88,0x00,
	0x88,0x50,0x20,0x20,0x20,0x50,0x88,0x00,0x88,0x88,0x88,0x50,0x20,0x20,0x20,0x00,
	0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x30,0x20,0x20,0x20,0x20,0x20,0x30,0x00,
	0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x30,0x10,0x10,0x10,0x10,0x10,0x30,0x00,
	0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,
	0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,
	0x80,0x80,0xf0,0x88,0x88,0x88,0xf0,0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x78,0x00,
	0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x78,0x00,
	0x18,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x70,
	0x80,0x80,0xf0,0x88,0x88,0x88,0x88,0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,
	0x20,0x00,0x20,0x20,0x20,0x20,0x20,0xc0,0x80,0x80,0x90,0xa0,0xe0,0x90,0x88,0x00,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xf0,0xa8,0xa8,0xa8,0xa8,0x00,
	0x00,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,
	0x00,0x00,0xf0,0x88,0x88,0xf0,0x80,0x80,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08,
	0x00,0x00,0xb0,0xc8,0x80,0x80,0x80,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xf0,0x00,
	0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,
	0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00,0x00,0x00,0xa8,0xa8,0xa8,0xa8,0x50,0x00,
	0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x88,0x88,0x88,0x78,0x08,0x70,
	0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x00,0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00,
	0x00,0x68,0xb0,0x00,0x00,0x00,0x00,0x00,0x20,0x50,0x20,0x50,0xa8,0x50,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x40,0x0c,0x10,0x38,0x10,0x20,0x20,0xc0,0x00,
	0x00,0x00,0x00,0x00,0x00,0x28,0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0xa8,0x00,
	0x70,0xa8,0xf8,0x20,0x20,0x20,0x20,0x00,0x70,0xa8,0xf8,0x20,0x20,0xf8,0xa8,0x70,
	0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,0x44,0xa8,0x50,0x20,0x68,0xd4,0x28,0x00,
	0x88,0x70,0x88,0x60,0x30,0x88,0x70,0x00,0x00,0x10,0x20,0x40,0x20,0x10,0x00,0x00,
	0x78,0xa0,0xa0,0xb0,0xa0,0xa0,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x20,0x20,0x00,0x00,0x00,0x00,0x00,
	0x10,0x10,0x20,0x00,0x00,0x00,0x00,0x00,0x28,0x50,0x50,0x00,0x00,0x00,0x00,0x00,
	0x28,0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x78,0x78,0x30,0x00,0x00,
	0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,
	0x68,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,0xf4,0x5c,0x54,0x54,0x00,0x00,0x00,0x00,
	0x88,0x70,0x78,0x80,0x70,0x08,0xf0,0x00,0x00,0x40,0x20,0x10,0x20,0x40,0x00,0x00,
	0x00,0x00,0x70,0xa8,0xb8,0xa0,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x88,0x88,0x50,0x20,0x20,0x20,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,
	0x00,0x20,0x70,0xa8,0xa0,0xa8,0x70,0x20,0x30,0x48,0x40,0xe0,0x40,0x48,0xf0,0x00,
	0x00,0x48,0x30,0x48,0x48,0x30,0x48,0x00,0x88,0x88,0x50,0xf8,0x20,0xf8,0x20,0x00,
	0x20,0x20,0x20,0x00,0x20,0x20,0x20,0x00,0x78,0x80,0x70,0x88,0x70,0x08,0xf0,0x00,
	0xd8,0xd8,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x48,0x94,0xa4,0xa4,0x94,0x48,0x30,
	0x60,0x10,0x70,0x90,0x70,0x00,0x00,0x00,0x00,0x28,0x50,0xa0,0x50,0x28,0x00,0x00,
	0x00,0x00,0x00,0xf8,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,
	0x30,0x48,0xb4,0xb4,0xa4,0xb4,0x48,0x30,0x7c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x60,0x90,0x90,0x60,0x00,0x00,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0xf8,0x00,
	0x60,0x90,0x20,0x40,0xf0,0x00,0x00,0x00,0x60,0x90,0x20,0x90,0x60,0x00,0x00,0x00,
	0x10,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x88,0xc8,0xb0,0x80,
	0x78,0xd0,0xd0,0xd0,0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x10,0x20,0x00,0x20,0x60,0x20,0x20,0x70,0x00,0x00,0x00,
	0x20,0x50,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0xa0,0x50,0x28,0x50,0xa0,0x00,0x00,
	0x40,0x48,0x50,0x28,0x58,0xa8,0x38,0x08,0x40,0x48,0x50,0x28,0x44,0x98,0x20,0x3c,
	0xc0,0x28,0xd0,0x28,0xd8,0xa8,0x38,0x08,0x20,0x00,0x20,0x40,0x80,0x88,0x70,0x00,
	0x40,0x20,0x70,0x88,0xf8,0x88,0x88,0x00,0x10,0x20,0x70,0x88,0xf8,0x88,0x88,0x00,
	0x70,0x00,0x70,0x88,0xf8,0x88,0x88,0x00,0x68,0xb0,0x70,0x88,0xf8,0x88,0x88,0x00,
	0x50,0x00,0x70,0x88,0xf8,0x88,0x88,0x00,0x20,0x50,0x70,0x88,0xf8,0x88,0x88,0x00,
	0x78,0xa0,0xa0,0xf0,0xa0,0xa0,0xb8,0x00,0x70,0x88,0x80,0x80,0x88,0x70,0x08,0x70,
	0x40,0x20,0xf8,0x80,0xf0,0x80,0xf8,0x00,0x10,0x20,0xf8,0x80,0xf0,0x80,0xf8,0x00,
	0x70,0x00,0xf8,0x80,0xf0,0x80,0xf8,0x00,0x50,0x00,0xf8,0x80,0xf0,0x80,0xf8,0x00,
	0x40,0x20,0x70,0x20,0x20,0x20,0x70,0x00,0x10,0x20,0x70,0x20,0x20,0x20,0x70,0x00,
	0x70,0x00,0x70,0x20,0x20,0x20,0x70,0x00,0x50,0x00,0x70,0x20,0x20,0x20,0x70,0x00,
	0x70,0x48,0x48,0xe8,0x48,0x48,0x70,0x00,0x68,0xb0,0x88,0xc8,0xa8,0x98,0x88,0x00,
	0x40,0x20,0x70,0x88,0x88,0x88,0x70,0x00,0x10,0x20,0x70,0x88,0x88,0x88,0x70,0x00,
	0x70,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x68,0xb0,0x70,0x88,0x88,0x88,0x70,0x00,
	0x50,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,
	0x00,0x74,0x88,0x90,0xa8,0x48,0xb0,0x00,0x40,0x20,0x88,0x88,0x88,0x88,0x70,0x00,
	0x10,0x20,0x88,0x88,0x88,0x88,0x70,0x00,0x70,0x00,0x88,0x88,0x88,0x88,0x70,0x00,
	0x50,0x00,0x88,0x88,0x88,0x88,0x70,0x00,0x10,0xa8,0x88,0x50,0x20,0x20,0x20,0x00,
	0x00,0x80,0xf0,0x88,0x88,0xf0,0x80,0x80,0x60,0x90,0x90,0xb0,0x88,0x88,0xb0,0x00,
	0x40,0x20,0x70,0x08,0x78,0x88,0x78,0x00,0x10,0x20,0x70,0x08,0x78,0x88,0x78,0x00,
	0x70,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x68,0xb0,0x70,0x08,0x78,0x88,0x78,0x00,
	0x50,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x20,0x50,0x70,0x08,0x78,0x88,0x78,0x00,
	0x00,0x00,0xf0,0x28,0x78,0xa0,0x78,0x00,0x00,0x00,0x70,0x88,0x80,0x78,0x08,0x70,
	0x40,0x20,0x70,0x88,0xf8,0x80,0x70,0x00,0x10,0x20,0x70,0x88,0xf8,0x80,0x70,0x00,
	0x70,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,0x50,0x00,0x70,0x88,0xf8,0x80,0x70,0x00,
	0x40,0x20,0x00,0x60,0x20,0x20,0x70,0x00,0x10,0x20,0x00,0x60,0x20,0x20,0x70,0x00,
	0x20,0x50,0x00,0x60,0x20,0x20,0x70,0x00,0x50,0x00,0x00,0x60,0x20,0x20,0x70,0x00,
	0x50,0x60,0x10,0x78,0x88,0x88,0x70,0x00,0x68,0xb0,0x00,0xf0,0x88,0x88,0x88,0x00,
	0x40,0x20,0x00,0x70,0x88,0x88,0x70,0x00,0x10,0x20,0x00,0x70,0x88,0x88,0x70,0x00,
	0x20,0x50,0x00,0x70,0x88,0x88,0x70,0x00,0x68,0xb0,0x00,0x70,0x88,0x88,0x70,0x00,
	0x00,0x50,0x00,0x70,0x88,0x88,0x70,0x00,0x00,0x20,0x00,0xf8,0x00,0x20,0x00,0x00,
	0x00,0x00,0x68,0x90,0xa8,0x48,0xb0,0x00,0x40,0x20,0x88,0x88,0x88,0x98,0x68,0x00,
	0x10,0x20,0x88,0x88,0x88,0x98,0x68,0x00,0x70,0x00,0x88,0x88,0x88,0x98,0x68,0x00,
	0x50,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x10,0x20,0x88,0x88,0x88,0x78,0x08,0x70,
	0x80,0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x50,0x00,0x88,0x88,0x88,0x78,0x08,0x70
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void render_font_char_expand(render_font *font, render_font_char *ch);
static void font_scale(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param);
static int render_font_load_bdf(render_font *font, const char *filename);
static int render_font_load_raw(render_font *font, const UINT8 *data, int width, int height, int numchars);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    next_line - return a pointer to the start of
    the next line
-------------------------------------------------*/

INLINE const char *next_line(const char *ptr)
{
	/* scan forward until we hit the end or a carriage return */
	while (*ptr != 13 && *ptr != 10 && *ptr != 0) ptr++;

	/* if we hit the end, return NULL */
	if (*ptr == 0)
		return NULL;

	/* eat the trailing linefeed if present */
	if (*++ptr == 10)
		ptr++;
	return ptr;
}


/*-------------------------------------------------
    get_char - return a pointer to a character
    in a font, expanding if necessary
-------------------------------------------------*/

INLINE render_font_char *get_char(render_font *font, UINT16 chnum)
{
	static render_font_char dummy_char;
	render_font_char *chtable;
	render_font_char *ch;

	/* grab the table; if none, return the dummy character */
	chtable = font->chars[chnum / 256];
	if (chtable == NULL)
		return &dummy_char;

	/* if the character isn't generated yet, do it now */
	ch = &chtable[chnum % 256];
	if (ch->bitmap == NULL)
		render_font_char_expand(font, ch);

	/* return the resulting character */
	return ch;
}



/***************************************************************************

    Render fonts

***************************************************************************/

/*-------------------------------------------------
    render_font_alloc - allocate a new font
    and load the BDF file
-------------------------------------------------*/

render_font *render_font_alloc(const char *filename)
{
	render_font *font;

	/* allocate and clear memory */
	font = malloc_or_die(sizeof(*font));
	memset(font, 0, sizeof(*font));

	/* attempt to load the font */
	if (filename != NULL && render_font_load_bdf(font, filename) == 0)
		return font;

	/* if we failed, clean up and realloc */
	render_font_free(font);
	font = malloc_or_die(sizeof(*font));
	memset(font, 0, sizeof(*font));

	/* load the raw data instead */
	render_font_load_raw(font, uifontdata, 6, 8, 256);
	return font;
}


/*-------------------------------------------------
    render_font_free - free an allocated font and
    all of its owned subobjects
-------------------------------------------------*/

void render_font_free(render_font *font)
{
	int tablenum;

	/* free all the subtables */
	for (tablenum = 0; tablenum < 256; tablenum++)
		if (font->chars[tablenum] != NULL)
		{
			int charnum;

			/* loop over characters */
			for (charnum = 0; charnum < 256; charnum++)
			{
				render_font_char *ch = &font->chars[tablenum][charnum];
				if (ch->texture != NULL)
					render_texture_free(ch->texture);
				if (ch->bitmap != NULL)
					bitmap_free(ch->bitmap);
			}

			/* free the subtable itself */
			free(font->chars[tablenum]);
		}

	/* free the raw data and the size itself */
	if (font->rawdata != NULL)
		free((void *)font->rawdata);
	free(font);
}


/*-------------------------------------------------
    render_font_char_expand - expand the raw data
    for a character into a bitmap
-------------------------------------------------*/

static void render_font_char_expand(render_font *font, render_font_char *ch)
{
	const char *ptr = ch->rawdata;
	int x, y;

	/* punt if nothing there */
	if (ch->bmwidth == 0 || ch->bmheight == 0 || ch->rawdata == NULL)
		return;

	/* allocate a new bitmap of the size we need */
	ch->bitmap = bitmap_alloc_depth(ch->bmwidth, font->height, 32);

	/* extract the data */
	for (y = 0; y < ch->bmheight; y++)
	{
		int desty = y + font->height + font->yoffs - ch->yoffs - ch->bmheight;
		UINT32 *dest = (desty >= 0 && desty < font->height) ? (UINT32 *)ch->bitmap->line[desty] : NULL;

		/* text format */
		if (font->format == FONT_FORMAT_TEXT)
		{
			/* loop over bytes */
			for (x = 0; x < ch->bmwidth; x += 4)
			{
				int bits = -1;

				/* scan for the next hex digit */
				while (*ptr != 13 && bits == -1)
				{
					if (*ptr >= '0' && *ptr <= '9')
						bits = *ptr++ - '0';
					else if (*ptr >= 'A' && *ptr <= 'F')
						bits = *ptr++ - 'A' + 10;
					else if (*ptr >= 'a' && *ptr <= 'f')
						bits = *ptr++ - 'a' + 10;
					else
						ptr++;
				}

				/* expand the four bits */
				if (dest != NULL)
				{
					*dest++ = (bits & 8) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 4) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 2) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 1) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
				}
			}

			/* advance to the next line */
			ptr = next_line(ptr);
		}

		/* binary format */
		else
		{
			/* loop over bytes */
			for (x = 0; x < ch->bmwidth; x += 8)
			{
				int bits = *ptr++;

				/* expand the eight bits */
				if (dest != NULL)
				{
					*dest++ = (bits & 0x80) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x40) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x20) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x10) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x08) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x04) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x02) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					*dest++ = (bits & 0x01) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
				}
			}
		}
	}

	/* wrap a texture around the bitmap */
	ch->texture = render_texture_alloc(ch->bitmap, NULL, NULL, TEXFORMAT_ARGB32, font_scale, NULL);
}


/*-------------------------------------------------
    render_font_get_char_texture_and_bounds -
    return the texture for a character and compute
    the bounds of the final bitmap
-------------------------------------------------*/

render_texture *render_font_get_char_texture_and_bounds(render_font *font, float height, float aspect, UINT16 chnum, render_bounds *bounds)
{
	render_font_char *ch = get_char(font, chnum);
	float scale = font->scale * height;

	/* on entry, assume x0,y0 are the top,left coordinate of the cell and add */
	/* the character bounding box to that position */
	bounds->x0 += (float)ch->xoffs * scale * aspect;

	/* compute x1,y1 from there based on the bitmap size */
	bounds->x1 = bounds->x0 + (float)ch->bmwidth * scale * aspect;
	bounds->y1 = bounds->y0 + (float)font->height * scale;

	/* return the texture */
	return ch->texture;
}


/*-------------------------------------------------
    render_font_draw_string_to_bitmap - draw a
    string to a bitmap
-------------------------------------------------*/

void render_font_get_scaled_bitmap_and_bounds(render_font *font, mame_bitmap *dest, float height, float aspect, UINT16 chnum, rectangle *bounds)
{
	render_font_char *ch = get_char(font, chnum);
	float scale = font->scale * height;
	INT32 origwidth, origheight;

	/* on entry, assume x0,y0 are the top,left coordinate of the cell and add */
	/* the character bounding box to that position */
	bounds->min_x = (float)ch->xoffs * scale * aspect;
	bounds->min_y = 0;

	/* compute x1,y1 from there based on the bitmap size */
	bounds->max_x = bounds->min_x + (float)ch->bmwidth * scale * aspect;
	bounds->max_y = bounds->min_y + (float)font->height * scale;

	/* if the bitmap isn't big enough, bail */
	if (dest->width < bounds->max_x - bounds->min_x || dest->height < bounds->max_y - bounds->min_y)
		return;

	/* scale the font */
	origwidth = dest->width;
	origheight = dest->height;
	dest->width = bounds->max_x - bounds->min_x;
	dest->height = bounds->max_y - bounds->min_y;
	font_scale(dest, ch->bitmap, NULL, NULL);
	dest->width = origwidth;
	dest->height = origheight;
}


/*-------------------------------------------------
    render_font_char_width - return the width of
    a character at the given height
-------------------------------------------------*/

float render_font_get_char_width(render_font *font, float height, float aspect, UINT16 ch)
{
	return (float)get_char(font, ch)->width * font->scale * height * aspect;
}


/*-------------------------------------------------
    render_font_string_width - return the width of
    a string at the given height
-------------------------------------------------*/

float render_font_get_string_width(render_font *font, float height, float aspect, const char *string)
{
	const unsigned char *ptr;
	int totwidth = 0;

	/* loop over the string and accumulate widths */
	for (ptr = (const unsigned char *)string; *ptr != 0; ptr++)
		totwidth += get_char(font, *ptr)->width;

	/* scale the final result based on height */
	return (float)totwidth * font->scale * height * aspect;
}


/*-------------------------------------------------
    render_font_wstring_width - return the width of
    a wide-char string at the given height
-------------------------------------------------*/

float render_font_get_wstring_width(render_font *font, float height, float aspect, const UINT16 *wstring)
{
	const UINT16 *ptr;
	int totwidth = 0;

	/* loop over the string and accumulate widths */
	for (ptr = wstring; *ptr != 0; ptr++)
		totwidth += get_char(font, *ptr)->width;

	/* scale the final result based on height */
	return (float)totwidth * font->scale * height * aspect;
}


/*-------------------------------------------------
    font_scale - scale fonts
-------------------------------------------------*/

static void font_scale(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param)
{
	render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	int dheight = dest->height;
	int dwidth = dest->width;
	int dxoffs = 0, dyoffs = 0;

	/* special casing for small fonts, which tend to look bad scaled to small sizes */
	if (source->height <= 12)
	{
		/* clear the bitmap so that the areas that aren't drawn are transparent */
		fillbitmap(dest, MAKE_ARGB(0x00,0xff,0xff,0xff), NULL);

		/* if the target height is less than 1.0 of the source height, do some */
		/* creative downscaling */
		if (dheight < source->height)
		{
			UINT8 row[20], col[20];
			int x, y;

			/* if the width is larger, pad it */
			if (dwidth >= source->width)
			{
				dxoffs = (dwidth - source->width + 1) / 2;
				dwidth = source->width;
			}

			/* determine which rows/columns we will skip */
			for (x = 0; x <= dwidth; x++)
				col[x] = (x * source->width) / dwidth;
			for (y = 0; y <= dheight; y++)
				row[y] = (y * source->height) / dheight;

			/* now map the pixels */
			for (y = 0; y < dheight; y++)
				for (x = 0; x < dwidth; x++)
				{
					int sx = col[x], nx = col[x+1];
					int sy = row[y], ny = row[y+1];
					UINT32 pix;

					/* start with the selected pixel */
					pix = *((UINT32 *)source->base + sy * source->rowpixels + sx);

					/* if the next column isn't the next source pixel, OR in the source pixel to our right */
					if (nx > sx + 1)
					{
						pix |= *((UINT32 *)source->base + sy * source->rowpixels + sx + 1);

						/* if the next row isn't the next source pixel, OR in the source pixel to our bottom-right */
						if (ny > sy + 1)
							pix |= *((UINT32 *)source->base + (sy + 1) * source->rowpixels + sx + 1);
					}

					/* if the next row isn't the next source pixel, OR in the source pixel to our bottom-right */
					if (ny > sy + 1)
						pix |= *((UINT32 *)source->base + (sy + 1) * source->rowpixels + sx);

					/* store the final result */
					*((UINT32 *)dest->base + y * dest->rowpixels + x + dxoffs) = pix;
				}

			/* bail early as we've done all the work */
			return;
		}

		/* if the target height is between 1.0 and 1.6667 of the source height, */
		/* pad without scaling instead of scaling */
		else if (dheight < source->height * 5 / 3)
		{
			dyoffs = (dheight - source->height + 1) / 2;
			dheight = source->height;

			/* same for the width unless it is at least 2x the original width */
			if (dwidth >= source->width && dwidth < source->width * 2)
			{
				dxoffs = (dwidth - source->width + 1) / 2;
				dwidth = source->width;
			}
		}
	}

	/* use the standard rescaling to do most of the work */
	render_resample_argb_bitmap_hq((UINT32 *)dest->base + dyoffs * dest->rowpixels + dxoffs, dest->rowpixels, dwidth, dheight,
			source, sbounds, &color);
}


/*-------------------------------------------------
    render_font_load_bdf - parse and load a BDF
    font
-------------------------------------------------*/

static int render_font_load_bdf(render_font *font, const char *filename)
{
	mame_file_error filerr;
	mame_file *file;
	const char *ptr;
	UINT64 bytes;
	char *data;

	/* attempt to open the file */
	filerr = mame_fopen(SEARCHPATH_FONT, filename, OPEN_FLAG_READ, &file);
	if (filerr != FILERR_NONE)
		return 1;

	/* determine the file size and allocate memory */
	font->rawsize = mame_fsize(file);
	data = malloc_or_die(font->rawsize + 1);

	/* read the raw data and NULL-terminate */
	bytes = mame_fread(file, data, font->rawsize);
	if (bytes != font->rawsize)
		goto error;
	data[font->rawsize] = 0;
	font->rawdata = data;
	font->format = FONT_FORMAT_TEXT;

	/* first find the FONTBOUNDINGBOX tag */
	for (ptr = font->rawdata; ptr != NULL; ptr = next_line(ptr))
	{
		int dummy1, dummy2;

		/* we only care about a tiny few fields */
		if (strncmp(ptr, "FONTBOUNDINGBOX ", 16) == 0)
		{
			if (sscanf(ptr + 16, "%d %d %d %d", &dummy1, &font->height, &dummy2, &font->yoffs) != 4)
				goto error;
			break;
		}
	}

	/* compute the scale factor */
	font->scale = 1.0f / (float)font->height;

	/* now scan for characters */
	for ( ; ptr != NULL; ptr = next_line(ptr))
	{
		/* stop at ENDFONT */
		if (strncmp(ptr, "ENDFONT", 7) == 0)
			break;

		/* once we hit a STARTCHAR, parse until the end */
		if (strncmp(ptr, "STARTCHAR ", 10) == 0)
		{
			int bmwidth = -1, bmheight = -1, xoffs = -1, yoffs = -1;
			const char *rawdata = NULL;
			int charnum = -1;
			int width = -1;
			int dummy1;

			/* scan for interesting per-character tags */
			for ( ; ptr != NULL; ptr = next_line(ptr))
			{
				/* ENCODING tells us which character */
				if (strncmp(ptr, "ENCODING ", 9) == 0)
				{
					if (sscanf(ptr + 9, "%d", &charnum) != 1)
						goto error;
				}

				/* DWIDTH tells us the width to the next character */
				else if (strncmp(ptr, "DWIDTH ", 7) == 0)
				{
					if (sscanf(ptr + 7, "%d %d", &width, &dummy1) != 2)
						goto error;
				}

				/* BBX tells us the height/width of the bitmap and the offsets */
				else if (strncmp(ptr, "BBX ", 4) == 0)
				{
					if (sscanf(ptr + 4, "%d %d %d %d", &bmwidth, &bmheight, &xoffs, &yoffs) != 4)
						goto error;
				}

				/* BITMAP is the start of the data */
				else if (strncmp(ptr, "BITMAP", 6) == 0)
				{
					/* stash the raw pointer and scan for the end of the character */
					for (rawdata = ptr = next_line(ptr); ptr != NULL && strncmp(ptr, "ENDCHAR", 7) != 0; ptr = next_line(ptr)) ;
					break;
				}
			}

			/* if we have everything, allocate a new character */
			if (charnum >= 0 && charnum < 65536 && rawdata != NULL && bmwidth >= 0 && bmheight >= 0)
			{
				render_font_char *ch;

				/* if we don't have a subtable yet, make one */
				if (font->chars[charnum / 256] == NULL)
				{
					font->chars[charnum / 256] = malloc_or_die(256 * sizeof(font->chars[0][0]));
					memset(font->chars[charnum / 256], 0, 256 * sizeof(font->chars[0][0]));
				}

				/* fill in the entry */
				ch = &font->chars[charnum / 256][charnum % 256];
				ch->width = width;
				ch->bmwidth = bmwidth;
				ch->bmheight = bmheight;
				ch->xoffs = xoffs;
				ch->yoffs = yoffs;
				ch->rawdata = rawdata;
			}
		}
	}

	mame_fclose(file);
	return 0;

error:
	mame_fclose(file);
	return 1;
}


/*-------------------------------------------------
    render_font_load_raw - load raw bitmap data
    into a font
-------------------------------------------------*/

static int render_font_load_raw(render_font *font, const UINT8 *data, int width, int height, int numchars)
{
	int charnum;

	/* fill in the core data */
	font->format = FONT_FORMAT_BINARY;
	font->height = height + 1;
	font->yoffs = 0;
	font->scale = 1.0f / (float)font->height;
	font->rawdata = NULL;
	font->rawsize = ((width + 7) / 8) * height * numchars;

	/* iterate over characters */
	for (charnum = 0; charnum < numchars; charnum++)
	{
		render_font_char *ch;

		/* if we don't have a subtable yet, make one */
		if (font->chars[charnum / 256] == NULL)
		{
			font->chars[charnum / 256] = malloc_or_die(256 * sizeof(font->chars[0][0]));
			memset(font->chars[charnum / 256], 0, 256 * sizeof(font->chars[0][0]));
		}

		/* fill in the entry */
		ch = &font->chars[charnum / 256][charnum % 256];
		ch->width = width;
		ch->xoffs = 0;
		ch->yoffs = 0;
		ch->bmwidth = width;
		ch->bmheight = height;
		ch->rawdata = data;

		/* advance to the next character */
		data += ((width + 7) / 8) * height;
	}

	return 0;
}
