/******************************************************************************
 *	Sharp MZ700
 *
 *	variables and function prototypes
 *
 *	Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ******************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "mess/includes/pit8253.h"

/* from src/mess/machine/mz700.c */
extern void init_mz700(void);
extern void mz700_init_machine(void);
extern void mz700_stop_machine(void);

extern int mz700_cassette_id(int id);
extern int mz700_cassette_init(int id);
extern void mz700_cassette_exit(int id);

extern int mz700_interrupt(void);

READ_HANDLER ( mz700_mmio_r );
WRITE_HANDLER ( mz700_mmio_w );
WRITE_HANDLER ( mz700_bank_w );

/* from src/mess/vidhrdw/mz700.c */

extern char mz700_frame_message[64+1];
extern int mz700_frame_time;

extern void mz700_init_colors (unsigned char *palette, unsigned short *colortable, const unsigned char *color_prom);
extern int mz700_vh_start (void);
extern void mz700_vh_stop (void);
extern void mz700_vh_screenrefresh (struct osd_bitmap *bitmap, int full_refresh);

/* from src/mess/sndhrdw/mz700.c */
extern int mz700_sh_start(const struct MachineSound *msound);
extern void mz700_sh_stop(void);
extern void mz700_sh_update(void);
extern void mz700_sh_set_clock(int clock);

