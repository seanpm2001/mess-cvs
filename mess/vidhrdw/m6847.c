/* m6847.c -- Implementation of Motorola 6847 video hardware chip
 *
 * Nate Woods
 *
 * Originally based on src/mess/vidhrdw/dragon.c by Mathis Rosenhauer
 *
 * Sources:
 *  M6847 data sheet (http://www.spies.com/arcade/schematics/DataSheets/6847.pdf)
 *  M6847T1 info from Rainbow magazine (10/86-12/86)
 */

#include <assert.h>
#include "m6847.h"
#include "state.h"
#include "vidhrdw/generic.h"
#include "includes/rstrbits.h"
#include "includes/rstrtrck.h"

/* The "Back doors" are declared here */
#include "includes/dragon.h"

#ifdef MAME_DEBUG
#define LOG_FS	1
#define LOG_HS	0
#else /* !MAME_DEBUG */
#define LOG_FS	0
#define LOG_HS	0
#endif /* MAME_DEBUG */

static void m6847_rastertrack_newscreen(struct rastertrack_vvars *vvars, struct rastertrack_hvars *hvars);
//static void m6847_rastertrack_endcontent(void);
static void m6847_rastertrack_getvideomode(struct rastertrack_hvars *hvars);

struct m6847_state {
	struct m6847_init_params initparams;
	int modebits;
	int videooffset;
	int latched_videooffset;
	int rowsize;
	int fs, hs;
};

static struct m6847_state the_state;

enum {
	M6847_MODEBIT_AG		= 0x80,
	M6847_MODEBIT_AS		= 0x40,
	M6847_MODEBIT_INTEXT	= 0x20,
	M6847_MODEBIT_INV		= 0x10,
	M6847_MODEBIT_CSS		= 0x08,
	M6847_MODEBIT_GM2		= 0x04,
	M6847_MODEBIT_GM1		= 0x02,
	M6847_MODEBIT_GM0		= 0x01
};

#define MAX_VRAM 6144

#define LOG_M6847 0

static UINT8 *game_palette;

static unsigned char palette[] = {
	0x00,0x00,0x00, /* BLACK */
	0x00,0xff,0x00, /* GREEN */
	0xff,0xff,0x00, /* YELLOW */
	0x00,0x00,0xff, /* BLUE */
	0xff,0x00,0x00, /* RED */
	0xff,0xff,0xff, /* BUFF */
	0x00,0xff,0xff, /* CYAN */
	0xff,0x00,0xff, /* MAGENTA */
	0xff,0x80,0x00, /* ORANGE */
	0x00,0x40,0x00,	/* ALPHANUMERIC DARK GREEN */
	0x00,0xff,0x00,	/* ALPHANUMERIC BRIGHT GREEN */
	0x40,0x10,0x00,	/* ALPHANUMERIC DARK ORANGE */
	0xff,0xc4,0x18	/* ALPHANUMERIC BRIGHT ORANGE */
};

static double artifactfactors[] = {
#if M6847_ARTIFACT_COLOR_COUNT == 2
	1.000, 0.500, 0.000, /* [ 1] */
	0.000, 0.500, 1.000  /* [ 2] */
#elif M6847_ARTIFACT_COLOR_COUNT == 14
	0.157, 0.000, 0.157, /* [ 1] - dk purple   (reverse  2) */
	0.000, 0.157, 0.000, /* [ 2] - dk green    (reverse  1) */
	1.000, 0.824, 1.000, /* [ 3] - lt purple   (reverse  4) */
	0.824, 1.000, 0.824, /* [ 4] - lt green    (reverse  3) */
	0.706, 0.236, 0.118, /* [ 5] - dk blue     (reverse  6) */
	0.000, 0.197, 0.471, /* [ 6] - dk red      (reverse  5) */
	1.000, 0.550, 0.393, /* [ 7] - lt blue     (reverse  8) */
	0.275, 0.785, 1.000, /* [ 8] - lt red      (reverse  7) */
	0.000, 0.500, 1.000, /* [ 9] - red         (reverse 10) */
	1.000, 0.500, 0.000, /* [10] - blue        (reverse  9) */
	1.000, 0.942, 0.785, /* [11] - cyan        (reverse 12) */
	0.393, 0.942, 1.000, /* [12] - yellow      (reverse 11) */
	0.236, 0.000, 0.000, /* [13] - black-blue  (reverse 14) */
	0.000, 0.000, 0.236  /* [14] - black-red   (reverse 13) */
#else
#error Bad Artifact Color Count!!
#endif
};

static unsigned char fontdata8x12[] =
{
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x1a, 0x2a, 0x2a, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x12, 0x12, 0x1c, 0x12, 0x12, 0x3c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x12, 0x12, 0x12, 0x12, 0x12, 0x3c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1e, 0x20, 0x20, 0x26, 0x22, 0x22, 0x1e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x36, 0x2a, 0x2a, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x32, 0x2a, 0x26, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x2a, 0x24, 0x1a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x28, 0x24, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x10, 0x08, 0x04, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x2a, 0x2a, 0x36, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x1c, 0x2a, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x3e, 0x10, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x14, 0x14, 0x36, 0x00, 0x36, 0x14, 0x14, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x1e, 0x20, 0x1c, 0x02, 0x3c, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x32, 0x32, 0x04, 0x08, 0x10, 0x26, 0x26, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x10, 0x2a, 0x24, 0x1a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x3e, 0x1c, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x10, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x1c, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x0c, 0x02, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x0c, 0x14, 0x3e, 0x04, 0x04, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x3c, 0x02, 0x02, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1e, 0x02, 0x02, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x24, 0x04, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,

	/* Lower case */
	0x00, 0x00, 0x00, 0x0C, 0x12, 0x10, 0x38, 0x10, 0x12, 0x3C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x02, 0x1E, 0x22, 0x1E, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x3C, 0x22, 0x22, 0x22, 0x3C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x20, 0x20, 0x20, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x02, 0x1E, 0x22, 0x22, 0x22, 0x1E, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x3E, 0x20, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0C, 0x12, 0x10, 0x38, 0x10, 0x10, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x22, 0x22, 0x22, 0x1E, 0x02, 0x1C,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x3C, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x24, 0x18,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x24, 0x28, 0x38, 0x24, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x2A, 0x2A, 0x2A, 0x2A, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x32, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x22, 0x3C, 0x20, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x22, 0x22, 0x22, 0x1E, 0x02, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x3C, 0x10, 0x10, 0x10, 0x12, 0x0C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x26, 0x1A, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x2A, 0x2A, 0x1C, 0x14, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x1E, 0x02, 0x1C,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x04, 0x08, 0x10, 0x3E, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x2A, 0x1C, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x04, 0x3E, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,

	/* Semigraphics 6 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0,
	0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
	0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
	0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f,
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f,
	0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0,
	0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	/* Block Graphics (Semigraphics 4 Graphics ) */
	0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
	0x00,0x00,0x00,0x00,0x00,0x00, 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
	0x00,0x00,0x00,0x00,0x00,0x00, 0xff,0xff,0xff,0xff,0xff,0xff,
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, 0x00,0x00,0x00,0x00,0x00,0x00,
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, 0xff,0xff,0xff,0xff,0xff,0xff,
	0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0x00,0x00,0x00,0x00,0x00,0x00,
	0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
	0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
	0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff, 0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff, 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
	0xff,0xff,0xff,0xff,0xff,0xff, 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
	0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff
};

/* --------------------------------------------------
 * Initialization and termination
 * -------------------------------------------------- */

void m6847_vh_normalparams(struct m6847_init_params *params)
{
	memset(params, '\0', sizeof(struct m6847_init_params));
	params->version = M6847_VERSION_ORIGINAL;
	params->artifactdipswitch = -1;
}

void m6847_vh_init_palette(unsigned char *sys_palette, unsigned short *sys_colortable,const unsigned char *color_prom)
{
	assert((sizeof(artifactfactors) / (sizeof(artifactfactors[0]) * 3)) == M6847_ARTIFACT_COLOR_COUNT);

	memcpy(sys_palette, palette, sizeof(palette));

	setup_artifact_palette(sys_palette, sizeof(palette) / (sizeof(palette[0])*3) + (M6847_ARTIFACT_COLOR_COUNT*0),
		0, 1, artifactfactors, M6847_ARTIFACT_COLOR_COUNT, 0);
	setup_artifact_palette(sys_palette, sizeof(palette) / (sizeof(palette[0])*3) + (M6847_ARTIFACT_COLOR_COUNT*1),
		0, 1, artifactfactors, M6847_ARTIFACT_COLOR_COUNT, 1);
	setup_artifact_palette(sys_palette, sizeof(palette) / (sizeof(palette[0])*3) + (M6847_ARTIFACT_COLOR_COUNT*2),
		0, 5, artifactfactors, M6847_ARTIFACT_COLOR_COUNT, 0);
	setup_artifact_palette(sys_palette, sizeof(palette) / (sizeof(palette[0])*3) + (M6847_ARTIFACT_COLOR_COUNT*3),
		0, 5, artifactfactors, M6847_ARTIFACT_COLOR_COUNT, 1);

	/* I'm taking advantage about how 'sys_palette' will be valid for the
	 * entire emulation; check src/palette.c for details
	 */
	game_palette = sys_palette;
}

static struct rastertrack_interface m6847_rastertrack_intf =
{
	263,
	m6847_rastertrack_newscreen,
	NULL,
	internal_m6847_rastertrack_endcontent,
	m6847_rastertrack_getvideomode,
	0
};

int internal_m6847_vh_start(const struct m6847_init_params *params, struct rastertrack_interface *intf, int dirtyramsize)
{
	struct rastertrack_initvars vars;

	the_state.initparams = *params;
	the_state.modebits = 0;
	the_state.videooffset = 0;
	the_state.latched_videooffset = 0;
	the_state.rowsize = 12;
	the_state.fs = 1;
	the_state.hs = 1;

	videoram_size = dirtyramsize;
	if (generic_vh_start())
		return 1;

	vars.intf = intf;
	vars.vram = params->ram;
	vars.vramwrap = params->ramsize;
	rastertrack_init(&vars);

	return 0;
}

int m6847_vh_start(const struct m6847_init_params *params)
{
	int result;

	result = internal_m6847_vh_start(params, &m6847_rastertrack_intf, MAX_VRAM);
	if (result)
		return result;

	state_save_register_func_postload(schedule_full_refresh);
	return 0;
}

void m6847_vh_stop(void)
{
	generic_vh_stop();
}

int m6847_vh_interrupt(void)
{
	return rastertrack_hblank();
}

/* --------------------------------------------------
 * Timing
 *
 * This M6847 code attempts to emulate the tricky timing of the M6847.  There
 * are two signals in question:  HS (Horizontal Sync) and FS (Field Sync).
 *
 * MAME/MESS timing will call us at vblank time; so all of our timing must be
 * relative to that point.
 *
 * Below are tables that show when each signal changes
 *
 * How to read these tables:
 *     "@ CLK(i) + j"  means "at clock cycle i plus j"
 *
 * HS:  Total Period: 227.5 clock cycles
 *		@ CLK(0) + DHS_F			- falling edge (high to low)
 *		@ CLK(16.5) + DHS_R			- rising edge (low to high)
 *		@ CLK(227.5) + DHS_F		- falling edge (high to low)
 *		...
 *
 * FS:	Total Period 262*227.5 clock cycles
 *		@ CLK(0) + DFS_F			- falling edge (high to low)
 *      @ CLK(32*227.5) + DFS_R		- rising edge (low to high)
 *		@ CLK(262*227.5) + DFS_F	- falling edge (high to low) (262.5 for the M6847Y)
 *
 * Source: Motorola M6847 Manual
 * -------------------------------------------------- */

#define DHS_F	TIME_IN_NSEC(550)
#define DHS_R	TIME_IN_NSEC(740)
#define DFS_F	TIME_IN_NSEC(520)
#define DFS_R	TIME_IN_NSEC(600)

/* TO BE RESOLVED:  The M6847 Manual says that HSYNCs occur every 227.5 clock
 * cycles; however every indication with the CoCo 3 seems to imply that HSYNCs
 * happen every 228 clock cycles.  To be honest, I'm not sure what the truth
 * really is... maybe they were different?  (Remember that the CoCo 3 did not
 * actually use the m6847 */

/* The reason we have a delay is because of a very fine point in MAME/MESS's
 * emulation.  In the CoCo, fs/hs are tied to interrupts, and the game "Popcorn"
 * polls the interrupt sync flag (on $ff03 in PIA0), waiting for fs to trigger
 * an interrupt.  In a real CoCo, CPU instructions are executed among different
 * clock cycles, and it is possible for fs to be changed while an instruction is
 * happening.  In MAME/MESS, the change doesn't occur until the instruction is
 * over, and it jumps right to the interrupt handler, which clears the interrupt
 * sync flag.  Thus, the main program never sees the interrupt flag change, and
 * the emulation waits for ever.  Since MAME/MESS will most likely never split
 * instructions up, this delay is an attempt to delay the interrupts and allow
 * the program to see fs change before the interrupt handler is invoked
 */
struct callback_info {
	mem_write_handler callback;
	int value;
};

static void do_invoke(int ci_int)
{
	struct callback_info *ci;
	ci = (struct callback_info *) ci_int;
	ci->callback(0, ci->value);
	free(ci);
}

static void invoke_callback(mem_write_handler callback, double delay, int value)
{
	struct callback_info *ci;

	if (callback) {
		ci = (struct callback_info *) malloc(sizeof(struct callback_info));
		if (!ci)
			return;
		ci->callback = callback;
		ci->value = value;
		timer_set(delay, (int) ci, do_invoke);
	}
}

static void hs_fall(int dummy)
{
	the_state.hs = 0;
	invoke_callback(the_state.initparams.hs_func, the_state.initparams.callback_delay, the_state.hs);

#if LOG_HS
	logerror("hs_fall(): hs=0 time=%g\n", timer_get_time());
#endif
}

static void hs_rise(int dummy)
{
	the_state.hs = 1;
	invoke_callback(the_state.initparams.hs_func, the_state.initparams.callback_delay, the_state.hs);

#if LOG_HS
	logerror("hs_rise(): hs=1 time=%g\n", timer_get_time());
#endif
}

static void fs_fall(int dummy)
{
	the_state.fs = 0;
	invoke_callback(the_state.initparams.fs_func, the_state.initparams.callback_delay, the_state.fs);

#if LOG_FS
	logerror("fs_fall(): fs=0 time=%g\n", timer_get_time());
#endif
}

static void fs_rise(int dummy)
{
	the_state.fs = 1;
	invoke_callback(the_state.initparams.fs_func, the_state.initparams.callback_delay, the_state.fs);

#if LOG_FS
	logerror("fs_rise(): fs=1 time=%g\n", timer_get_time());
#endif
}

int m6847_rastertrack_hblank(void)
{
	timer_set(DHS_F, 0, hs_fall);
	timer_set(DHS_R + (TIME_IN_HZ(3588545.0) * 16.5), 0, hs_rise);
	return ignore_interrupt();
}

void internal_m6847_rastertrack_endcontent(void)
{
	timer_set(DFS_F, 0, fs_fall);
}

/* --------------------------------------------------
 * The meat
 * -------------------------------------------------- */

void m6847_set_ram_size(int ramsize)
{
	the_state.initparams.ramsize = ramsize;
}

static UINT8 *mapper_alphanumeric(UINT8 *mem, int param, int *fg, int *bg, int *attr)
{
	UINT8 b;
	UINT8 *character;
	int bgc = 0, fgc = 0;

	b = *mem;

	/* Give the host machine a chance to pull our strings */
	the_state.initparams.charproc(b);

	if (the_state.modebits & M6847_MODEBIT_AS) {
		/* Semigraphics */
		bgc = 8;

		if ((the_state.modebits & M6847_MODEBIT_INTEXT) && (the_state.initparams.version != M6847_VERSION_M6847T1)) {
			/* Semigraphics 6 */
			character = &fontdata8x12[(96 + (b & 0x3f)) * 12];
			fgc = ((b >> 6) & 0x3);
			if (the_state.modebits & M6847_MODEBIT_CSS)
				fgc += 4;
		}
		else {
			/* Semigraphics 4 */
			switch(b & 0x0f) {
			case 0:
				bgc = 8;
				character = NULL;
				break;
			case 15:
				bgc = (b >> 4) & 0x7;
				character = NULL;
				break;
			default:
				bgc = 8;
				fgc = (b >> 4) & 0x7;
				character = &fontdata8x12[(160 + (b & 0x0f)) * 12];
				break;
			}
		}

	}
	else {
		/* Text */
		fgc = (the_state.modebits & M6847_MODEBIT_CSS) ? 15 : 13;

		/* Inverse the character, if appropriate */
		if (the_state.modebits & M6847_MODEBIT_INV)
			fgc ^= 1;

		if (the_state.initparams.version == M6847_VERSION_M6847T1) {
			/* M6847T1 specific features */

			/* Lowercase */
			if ((the_state.modebits & M6847_MODEBIT_GM0) && (b < 0x20))
				b += 0x40;
			else
				b &= 0x3f;

			/* Inverse (The following was verified in Rainbow Magazine 12/86) */
			if (the_state.modebits & M6847_MODEBIT_GM1)
				fgc ^= 1;
		}
		else {
			b &= 0x3f;
		}


		if (b == 0x20) {
			character = NULL;
		}
		else {
			character = &fontdata8x12[b * 12];
			if (param)
				character += 1;	/* Skew up */
		}
		bgc = fgc ^ 1;
	}

	*bg = bgc;
	*fg = fgc;
	return character;
}

/* This is a refresh function used by the Dragon/CoCo as well as the CoCo 3 when in lo-res
 * mode.  Internally, it treats the colors like a CoCo 3 and uses the pens array to
 * translate those colors to the proper palette.
 *
 * video_vmode
 *     bit 4	1=graphics, 0=text
 *     bit 3    resolution high bit
 *     bit 2    resolution low bit
 *     bit 1    1=b/w graphics, 0=color graphics
 *     bit 0	color set
 */
void internal_m6847_rastertrack_getvideomode(struct rastertrack_hvars *hvars,
	UINT32 *pens, int skew_up, int border_pen, int wf,
	int artifact_value, int artifact_palettebase,
	void (*getcolorrgb)(int c, UINT8 *red, UINT8 *green, UINT8 *blue))
{
	hvars->mode.height = 192 / the_state.rowsize;
	hvars->mode.offset = 0;
	hvars->frame_width = 256 * wf;
	hvars->frame_height = 192;
	hvars->border_pen = Machine->pens[border_pen];

	if (the_state.modebits & M6847_MODEBIT_AG)
	{
		hvars->mode.flags = RASTERBITS_FLAG_GRAPHICS;
		hvars->mode.flags = RASTERBITS_FLAG_GRAPHICS;

		if (the_state.modebits & M6847_MODEBIT_GM0)
		{
			/* Resolution modes */
			hvars->mode.bytesperrow = ((the_state.modebits & (M6847_MODEBIT_GM2|M6847_MODEBIT_GM1)) == (M6847_MODEBIT_GM2|M6847_MODEBIT_GM1)) ? 32 : 16;
			hvars->mode.width = hvars->mode.bytesperrow * 8;
			hvars->mode.depth = 1;
			if (the_state.modebits & M6847_MODEBIT_CSS) {
				hvars->mode.pens[0] = pens[10];
				hvars->mode.pens[1] = pens[11];
			}
			else {
				hvars->mode.pens[0] = pens[8];
				hvars->mode.pens[1] = pens[9];
			}

			if (artifact_value && (hvars->mode.bytesperrow == 32)) {
				/* I am here because we are doing PMODE 4 artifact colors */

				hvars->mode.flags |= RASTERBITS_FLAG_ARTIFACT;
				if (artifact_palettebase < 0) {
					hvars->mode.u.artifact.flags = RASTERBITS_ARTIFACT_STATICPALLETTE;
					hvars->mode.u.artifact.u.staticpalette = game_palette;
				}
				else {
					hvars->mode.u.artifact.flags = RASTERBITS_ARTIFACT_DYNAMICPALETTE;
					hvars->mode.u.artifact.u.dynamicpalettebase = artifact_palettebase;
				}

				if (artifact_value >= 2)
					hvars->mode.u.artifact.flags |= RASTERBITS_ARTIFACT_REVERSE;

				hvars->mode.u.artifact.colorfactors = artifactfactors;
				hvars->mode.u.artifact.numfactors = M6847_ARTIFACT_COLOR_COUNT;
				hvars->mode.u.artifact.getcolorrgb = getcolorrgb;
			}
		}
		else
		{
			/* Color modes */
			hvars->mode.bytesperrow = ((the_state.modebits & (M6847_MODEBIT_GM2|M6847_MODEBIT_GM1)) != 0) ? 32 : 16;
			hvars->mode.width = hvars->mode.bytesperrow * 4;
			hvars->mode.depth = 2;
			if (the_state.modebits & M6847_MODEBIT_CSS) {
				hvars->mode.pens[0] = pens[4];
				hvars->mode.pens[1] = pens[5];
				hvars->mode.pens[2] = pens[6];
				hvars->mode.pens[3] = pens[7];
			}
			else {
				hvars->mode.pens[0] = pens[0];
				hvars->mode.pens[1] = pens[1];
				hvars->mode.pens[2] = pens[2];
				hvars->mode.pens[3] = pens[3];
			}
		}
	}
	else
	{
		hvars->mode.flags = RASTERBITS_FLAG_TEXT | RASTERBITS_FLAG_TEXTMODULO;
		hvars->mode.bytesperrow = 32;
		hvars->mode.width = 32;
		hvars->mode.depth = 8;
		memcpy(hvars->mode.pens, pens, sizeof(hvars->mode.pens));
		hvars->mode.u.text.mapper = mapper_alphanumeric;
		hvars->mode.u.text.mapper_param = skew_up;
		hvars->mode.u.text.fontheight = 12;
		hvars->mode.u.text.underlinepos = -1;
	}
}

int m6847_get_bordercolor(void)
{
	int bordercolor;

	if (the_state.modebits & M6847_MODEBIT_AG) {
		if (the_state.modebits & M6847_MODEBIT_CSS)
			bordercolor = M6847_BORDERCOLOR_WHITE;
		else
			bordercolor = M6847_BORDERCOLOR_GREEN;
	}
	else {
		if ((the_state.initparams.version == M6847_VERSION_M6847T1)
				&& ((the_state.modebits & (M6847_MODEBIT_GM2|M6847_MODEBIT_GM1)) == M6847_MODEBIT_GM2)) {
			/* We are on the new VDG; and we have a colored border */
			if (the_state.modebits & M6847_MODEBIT_CSS)
				bordercolor = M6847_BORDERCOLOR_ORANGE;
			else
				bordercolor = M6847_BORDERCOLOR_GREEN;
		}
		else {
			bordercolor = M6847_BORDERCOLOR_BLACK;
		}
	}
	return bordercolor;
}

static int m6847_bordercolor(void)
{
	int pen = 0;

	switch(m6847_get_bordercolor()) {
	case M6847_BORDERCOLOR_BLACK:
		pen = 0;
		break;
	case M6847_BORDERCOLOR_GREEN:
		pen = 1;
		break;
	case M6847_BORDERCOLOR_WHITE:
		pen = 5;
		break;
	case M6847_BORDERCOLOR_ORANGE:
		pen = 12;
		break;
	}
	return pen;
}

static void m6847_rastertrack_getvideomode(struct rastertrack_hvars *hvars)
{
	int artifact_value;
	static UINT32 m6847_metapalette[] = {
		1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 0, 5, 9, 10, 11, 12
	};

	artifact_value = (the_state.initparams.artifactdipswitch == -1) ? 0 : (readinputport(the_state.initparams.artifactdipswitch) & 3);
	internal_m6847_rastertrack_getvideomode(hvars, m6847_metapalette, 0, m6847_bordercolor(), 1, artifact_value, -1, NULL);
}

void internal_m6847_rastertrack_newscreen(struct rastertrack_vvars *vvars, struct rastertrack_hvars *hvars,
	int border_top, int rows, int baseoffset, int use_m6847_offset, void (*getvideomode)(struct rastertrack_hvars *))
{
	timer_set(DFS_R, 0, fs_rise);

	the_state.latched_videooffset = the_state.videooffset;

	if (vvars) {
		vvars->bordertop = border_top;
		vvars->rows = rows;
		vvars->baseaddress = (use_m6847_offset ? the_state.latched_videooffset : 0) + baseoffset;
	}
	if (hvars) {
		getvideomode(hvars);
	}
}

static void m6847_rastertrack_newscreen(struct rastertrack_vvars *vvars, struct rastertrack_hvars *hvars)
{
	internal_m6847_rastertrack_newscreen(vvars, hvars, 38, 192, 0, TRUE, m6847_rastertrack_getvideomode);
}

void m6847_vh_update(struct osd_bitmap *bitmap, int full_refresh)
{
	rastertrack_refresh(bitmap, full_refresh);
}

/* --------------------------------------------------
 * Petty accessors
 * -------------------------------------------------- */

void m6847_set_video_offset(int offset)
{
#if LOG_M6847
	logerror("m6847_set_video_offset(): offset=$%04x\n", offset);
#endif

	offset %= the_state.initparams.ramsize;
	if (offset != the_state.videooffset) {
		the_state.videooffset = offset;
		schedule_full_refresh();
	}
}

int m6847_get_video_offset(void)
{
	return the_state.videooffset;
}

void m6847_touch_vram(int offset)
{
	offset -= the_state.latched_videooffset;
	offset %= the_state.initparams.ramsize;

	if (offset < videoram_size)
		dirtybuffer[offset] = 1;
}

void m6847_set_row_height(int rowheight)
{
	if (rowheight != the_state.rowsize) {
		the_state.rowsize = rowheight;
		rastertrack_touchvideomode();
	}
}

void m6847_set_cannonical_row_height(void)
{
	static const int graphics_rowheights[] = { 3, 3, 3, 2, 2, 1, 1, 1 };
	int rowheight;

	if (the_state.modebits & M6847_MODEBIT_AG) {
		rowheight = graphics_rowheights[the_state.modebits & (M6847_MODEBIT_GM2|M6847_MODEBIT_GM1|M6847_MODEBIT_GM0)];
	}
	else {
		rowheight = 12;
	}
	m6847_set_row_height(rowheight);
}

READ_HANDLER( m6847_ag_r )		{ return (the_state.modebits & M6847_MODEBIT_AG) ? 1 : 0; }
READ_HANDLER( m6847_as_r )		{ return (the_state.modebits & M6847_MODEBIT_AS) ? 1 : 0; }
READ_HANDLER( m6847_intext_r )	{ return (the_state.modebits & M6847_MODEBIT_INTEXT) ? 1 : 0; }
READ_HANDLER( m6847_inv_r )		{ return (the_state.modebits & M6847_MODEBIT_INV) ? 1 : 0; }
READ_HANDLER( m6847_css_r )		{ return (the_state.modebits & M6847_MODEBIT_CSS) ? 1 : 0; }
READ_HANDLER( m6847_gm2_r )		{ return (the_state.modebits & M6847_MODEBIT_GM2) ? 1 : 0; }
READ_HANDLER( m6847_gm1_r )		{ return (the_state.modebits & M6847_MODEBIT_GM1) ? 1 : 0; }
READ_HANDLER( m6847_gm0_r )		{ return (the_state.modebits & M6847_MODEBIT_GM0) ? 1 : 0; }
READ_HANDLER( m6847_fs_r )		{ return the_state.fs; }
READ_HANDLER( m6847_hs_r )		{ return the_state.hs; }

static void write_modebits(int data, int mask, int causesrefresh)
{
	int newmodebits;

	if (data)
		newmodebits = the_state.modebits | mask;
	else
		newmodebits = the_state.modebits & ~mask;

	if (newmodebits != the_state.modebits) {
		the_state.modebits = newmodebits;
		if (causesrefresh)
			rastertrack_touchvideomode();
	}
}

WRITE_HANDLER( m6847_ag_w )		{ write_modebits(data, M6847_MODEBIT_AG, 1); }
WRITE_HANDLER( m6847_as_w )		{ write_modebits(data, M6847_MODEBIT_AS, 0); }
WRITE_HANDLER( m6847_intext_w )	{ write_modebits(data, M6847_MODEBIT_INTEXT, 0); }
WRITE_HANDLER( m6847_inv_w )	{ write_modebits(data, M6847_MODEBIT_INV, 0); }
WRITE_HANDLER( m6847_css_w )	{ write_modebits(data, M6847_MODEBIT_CSS, 1); }
WRITE_HANDLER( m6847_gm2_w )	{ write_modebits(data, M6847_MODEBIT_GM2, 1); }
WRITE_HANDLER( m6847_gm1_w )	{ write_modebits(data, M6847_MODEBIT_GM1, 1); }
WRITE_HANDLER( m6847_gm0_w )	{ write_modebits(data, M6847_MODEBIT_GM0, 1); }
