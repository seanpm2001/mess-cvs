#include <windows.h>
#include <gx.h>
#include "mamece.h"
#include "osdepend.h"
#include "driver.h"

enum {
	GXSTATE_BLEND	= 1
};

struct gxstate_params
{
	GXDisplayProperties properties;
	int game_width, game_height;
	int scaled_width, scaled_height;
	int skipx_mask, skipy_mask;
	void **line;
	UINT32 *palette;
	long xadjustment;
	long yadjustment;
	long skip_adjustment;
	long base_adjustment;
	int flags;
};

inline int reduce_width_mask(int dimension, int skip_mask)
{
	int new_dimension = 0;

	while(dimension--) {
		if (dimension & skip_mask)
			new_dimension++;
	}
	return new_dimension;
}

static void calc_skip_mask(int dimension, int maximum, int *new_dimension, int *skip_mask)
{
	int i;

	if (dimension > maximum) {
		// Compute the most absurdly large mask
		i = dimension;
		*skip_mask = 0;
		while(i) {
			*skip_mask <<= 1;
			*skip_mask |= 1;
			i >>= 1;
		}

		do {
			*new_dimension = reduce_width_mask(dimension, *skip_mask);
			if (*new_dimension > maximum) {
				*skip_mask >>= 1;
				*skip_mask |= 1;
			}
		}
		while(*new_dimension > maximum);
	}
	else {
		// We already fit
		*skip_mask = 0;
		*new_dimension = dimension;
	}
}

template<class T>
void swap(T *t1, T *t2)
{
	T temp = *t1;
	*t1 = *t2;
	*t2 = temp;
}

static struct gxstate_params *create_gxstate(osd_bitmap *bitmap, int orientation, UINT32 *palette_16bit_lookup, UINT32 *palette_32bit_lookup)
{
	struct gxstate_params *params;

	// Allocate the params
	params = (struct gxstate_params *) malloc(sizeof(struct gxstate_params));
	if (!params)
		return NULL;

	// Get display properties from GX
	params->properties = GXGetDisplayProperties();

	// Modify GXProperties based on the orientation
	params->base_adjustment = 0;
	if (orientation & ORIENTATION_FLIP_X) {
		params->base_adjustment += (params->properties.cxWidth - 1) * params->properties.cbxPitch;
		params->properties.cbxPitch = -params->properties.cbxPitch;
	}
	if (orientation & ORIENTATION_FLIP_Y) {
		params->base_adjustment += (params->properties.cyHeight - 1) * params->properties.cbyPitch;
		params->properties.cbyPitch = -params->properties.cbyPitch;
	}
	if (orientation & ORIENTATION_SWAP_XY) {
		swap(&params->properties.cbxPitch, &params->properties.cbyPitch);
		swap(&params->properties.cxWidth, &params->properties.cyHeight);
	}

	// Set up instance variables
	params->game_width = bitmap->width;
	params->game_height = bitmap->height;
	params->line = bitmap->line;
	params->palette = ((bitmap->depth == 15) || (bitmap->depth == 16)) ? palette_16bit_lookup : NULL;
	params->flags = GXSTATE_BLEND;

	// Figure out how to scale the bitmap, if appropriate
	calc_skip_mask(params->game_width, params->properties.cxWidth, &params->scaled_width, &params->skipx_mask); 
	calc_skip_mask(params->game_height, params->properties.cyHeight, &params->scaled_height, &params->skipy_mask); 

	// Tweak the pitches and the bits
	params->xadjustment = params->properties.cbxPitch;
	params->yadjustment = params->properties.cbyPitch - (params->xadjustment * params->scaled_width);
	params->skip_adjustment = params->yadjustment + params->xadjustment * params->scaled_width;
	params->base_adjustment -= params->xadjustment;
	params->base_adjustment -= params->yadjustment;

	return params;
}

// --------------------------------------------------------------------------
// The main calls
// --------------------------------------------------------------------------

static struct gxstate_params *gxstate;

int gx_open_display(HWND hWnd)
{
	return GXOpenDisplay(hWnd, GX_FULLSCREEN);
}

int gx_close_display(void)
{
	if (gxstate) {
		free(gxstate);
		gxstate = NULL;
	}
	return GXCloseDisplay();
}

int gx_open_input(void)
{
	return GXOpenInput();
}

int gx_close_input(void)
{
	return GXCloseInput();
}

void gx_get_default_keys(struct gx_keylist *keylist)
{
	GXKeyList keys;
	keys = GXGetDefaultKeys(GX_NORMALKEYS);
	keylist->vkUp = keys.vkUp;
	keylist->vkDown = keys.vkDown;
	keylist->vkLeft = keys.vkLeft;
	keylist->vkRight = keys.vkRight;
	keylist->vkA = keys.vkA;
	keylist->vkB = keys.vkB;
	keylist->vkC = keys.vkC;
	keylist->vkStart = keys.vkStart;
}

void *gx_begin_draw(void)
{
	return GXBeginDraw();
}

int gx_end_draw(void)
{
	return GXEndDraw();
}

void gx_get_display_properties(struct gx_display_properties *properties)
{
	GXDisplayProperties gxproperties;
	
	gxproperties = GXGetDisplayProperties();

	properties->cxWidth = gxproperties.cxWidth;
	properties->cyHeight = gxproperties.cyHeight;
	properties->cbxPitch = gxproperties.cbxPitch;
	properties->cbyPitch = gxproperties.cbyPitch;
	properties->cBPP = gxproperties.cBPP;
	properties->ffFormat = gxproperties.ffFormat;
}

// --------------------------------------------------------------------------
// Blit code - speed very critical! templates out the wazoo!!
// --------------------------------------------------------------------------

#pragma optimize("agptb", on)

inline UINT16 rgb_part(UINT16 value, int size, int base)
{
	return (value >> (8 - size)) << base;
}

inline UINT16 rgb_value(const RGBQUAD *quad, int rbits, int gbits, int bbits)
{
	return rgb_part(quad->rgbRed, rbits, gbits + bbits) | rgb_part(quad->rgbGreen, gbits, bbits) | rgb_part(quad->rgbBlue, bbits, 0);
}

template<int rbits, int gbits, int bbits>
class blend16_functor {
public:
	void blend(UINT16 *pix, UINT16 newpix)
	{
		UINT16 p = *pix;
		if (p != newpix) {
			UINT16 mask = ((1 << (rbits + gbits + bbits)) - 1);
			mask &= ~(1 << 0);
			mask &= ~(1 << (bbits));
			mask &= ~(1 << (bbits + gbits));
			*pix = ((p & mask) >> 1) + ((newpix & mask) >> 1);
		}
	}
};

class blend888_functor {
public:
	void blend(RGBTRIPLE *pix, RGBTRIPLE newpix)
	{
		pix->rgbtRed = (pix->rgbtRed) / 2 + (newpix.rgbtRed / 2);
		pix->rgbtGreen = (pix->rgbtGreen) / 2 + (newpix.rgbtGreen / 2);
		pix->rgbtBlue = (pix->rgbtBlue) / 2 + (newpix.rgbtBlue / 2);
	}
};

class blendnull_functor {
public:
	void blend(RGBTRIPLE *pix, RGBTRIPLE newpix)
	{
		// Dummy
	}

	void blend(UINT16 *pix, UINT16 newpix)
	{
		// Dummy
	}
};

// The template that does the dirty work --- hyper optimized!!!
template<class pixtyp, class dummy, int do_skipx, int do_skipy, class blend_functor>
void __fastcall blit(pixtyp *pixels, struct gxstate_params *params, blend_functor blend, dummy *d)
{
	UINT8 *pvBits;
	UINT16 *line;
	int x, y;
	pixtyp ent;
	int height, width;
	int skipx_mask, skipy_mask;
	long xadjustment, yadjustment, skip_adjustment;
	void **linep;
	UINT32 *palette;

	// Pull stuff out of params
	height = params->game_height;
	width = params->game_width;
	xadjustment = params->xadjustment;
	yadjustment = params->yadjustment;
	skip_adjustment = params->skip_adjustment;
	skipx_mask = params->skipx_mask;
	skipy_mask = params->skipy_mask;
	linep = params->line;
	palette = params->palette;

	pvBits = (UINT8 *) pixels;

	// Do the dirty work
	y = height;
	while(y--) {
		if (do_skipy && !(y & skipy_mask)) {
			// Skipped
			pvBits += skip_adjustment;
		}
		else {
			pvBits += yadjustment;
			line = (UINT16 *) *(linep++);
			x = width;
			while(x--) {
				ent = *((pixtyp *) (&palette[*line]));
				if (do_skipx && !(x & skipx_mask)) {
					// Skipped
					blend.blend((pixtyp *) pvBits, ent);
				}
				else {
					pvBits += xadjustment;
					*((pixtyp *) pvBits) = ent;
				}
				line++;
			}
		}
	}

	// Schwag code to work around bugs in MSFT's compiler
	memset(d, sizeof(*d), sizeof(*d));
}

template<class pixtyp, class blend_functor>
void __fastcall blit2(pixtyp *pixels, struct gxstate_params *params, blend_functor blend)
{
	union {
		char c;
		short s;
		int i;
		long l;
		double d;
	} u;
	blendnull_functor nullfunctor;

	if (params->skipx_mask) {
		if (params->skipy_mask) {
			if (params->flags & GXSTATE_BLEND)
				blit<pixtyp, char, TRUE, TRUE, blend_functor>(pixels, params, blend, &u.c);
			else
				blit<pixtyp, char, TRUE, TRUE, blendnull_functor>(pixels, params, nullfunctor, &u.c);
		}
		else {
			if (params->flags & GXSTATE_BLEND)
				blit<pixtyp, short, TRUE, FALSE, blend_functor>(pixels, params, blend, &u.s);
			else
				blit<pixtyp, short, TRUE, FALSE, blendnull_functor>(pixels, params, nullfunctor, &u.s);
		}
	}
	else {
		if (params->skipy_mask) {
			if (params->flags & GXSTATE_BLEND)
				blit<pixtyp, long, FALSE, TRUE, blend_functor>(pixels, params, blend, &u.l);
			else
				blit<pixtyp, long, FALSE, TRUE, blendnull_functor>(pixels, params, nullfunctor, &u.l);
		}
		else {
			blit<pixtyp, double, FALSE, FALSE, blendnull_functor>(pixels, params, nullfunctor, &u.d);
		}
	}
}


// The uber-function that performs blits
void gx_blit(struct osd_bitmap *bitmap, int update, int orientation, UINT32 *palette_16bit_lookup, UINT32 *palette_32bit_lookup)
{
	BYTE *pvBits;
	DWORD ffFormat;

	if (!gxstate) {
		gxstate = create_gxstate(bitmap, orientation, palette_16bit_lookup, palette_32bit_lookup);
		if (!gxstate)
			return;
	}

	// Get the surface
	pvBits = (BYTE *) GXBeginDraw();
	pvBits += gxstate->base_adjustment;

	// What kind of surface are we?
	ffFormat = gxstate->properties.ffFormat;

	if (ffFormat & kfDirect)
	{
		if (gxstate->properties.ffFormat & kfDirect565) 
		{
			// The most common; so we are putting this first
			blit2((UINT16 *) pvBits, gxstate, blend16_functor<5,6,5>());
		}
		else if (ffFormat & kfDirect444)
		{
			blit2((UINT16 *) pvBits, gxstate, blend16_functor<4,4,4>());
		}
		else if (ffFormat & kfDirect555)
		{
			blit2((UINT16 *) pvBits, gxstate, blend16_functor<5,5,5>());
		}
		else if (ffFormat & kfDirect888)
		{
			blit2((RGBTRIPLE *) pvBits, gxstate, blend888_functor());
		}
	}
	else if (ffFormat & kfPalette)
	{
		// ???
	}
	else
	{
		// Bad?
	}

	GXEndDraw();
}
