/*

Gumbo (c)1994 Min Corp (Main Corp written on PCB)
Miss Puzzle (c)1994 Min Corp

argh they has the same music as news (news.c)

*/

/* working notes (Gumbo)

68k interrupts
lev 1 : 0x64 : 0000 0142 -
lev 2 : 0x68 : 0000 0142 -
lev 3 : 0x6c : 0000 0142 -
lev 4 : 0x70 : 0000 0142 -
lev 5 : 0x74 : 0000 0142 -
lev 6 : 0x78 : 0000 0142 -
lev 7 : 0x7c : 0000 0142 -

PCB Layout
----------

    M6295    U210     6264   U512
                      6264   U511
  ACTEL
  A1020A   14.31818MHz
DSW1          6116
              6116
                             6116
                             6116
  6264  6264
   U1    U2
   68000P10           U421   U420

*/

#include "driver.h"

data16_t *gumbo_bg_videoram;
data16_t *gumbo_fg_videoram;

WRITE16_HANDLER( gumbo_bg_videoram_w );
WRITE16_HANDLER( gumbo_fg_videoram_w );
VIDEO_START( gumbo );
VIDEO_UPDATE( gumbo );

static ADDRESS_MAP_START( gumbo_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1b0000, 0x1b03ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1c0100, 0x1c0101) AM_READ(input_port_0_word_r)
	AM_RANGE(0x1c0200, 0x1c0201) AM_READ(input_port_1_word_r)
	AM_RANGE(0x1c0300, 0x1c0301) AM_READ(OKIM6295_status_0_lsb_r)
	AM_RANGE(0x1e0000, 0x1e0fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1f0000, 0x1f3fff) AM_READ(MRA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gumbo_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_WRITE(MWA16_RAM) // main ram
	AM_RANGE(0x1c0300, 0x1c0301) AM_WRITE(OKIM6295_data_0_lsb_w)
	AM_RANGE(0x1b0000, 0x1b03ff) AM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x1e0000, 0x1e0fff) AM_WRITE(gumbo_bg_videoram_w) AM_BASE(&gumbo_bg_videoram) // bg tilemap
	AM_RANGE(0x1f0000, 0x1f3fff) AM_WRITE(gumbo_fg_videoram_w) AM_BASE(&gumbo_fg_videoram) // fg tilemap
ADDRESS_MAP_END

/* Miss Puzzle has a different memory map */

static ADDRESS_MAP_START( mspuzzle_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x103fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1a0000, 0x1a03ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1b0100, 0x1b0101) AM_READ(input_port_0_word_r)
	AM_RANGE(0x1b0200, 0x1b0201) AM_READ(input_port_1_word_r)
	AM_RANGE(0x1b0300, 0x1b0301) AM_READ(OKIM6295_status_0_lsb_r)
	AM_RANGE(0x190000, 0x197fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1c0000, 0x1c1fff) AM_READ(MRA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mspuzzle_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x103fff) AM_WRITE(MWA16_RAM) // main ram
	AM_RANGE(0x1b0300, 0x1b0301) AM_WRITE(OKIM6295_data_0_lsb_w)
	AM_RANGE(0x1a0000, 0x1a03ff) AM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x190000, 0x197fff) AM_WRITE(gumbo_fg_videoram_w) AM_BASE(&gumbo_fg_videoram) // fg tilemap
	AM_RANGE(0x1c0000, 0x1c1fff) AM_WRITE(gumbo_bg_videoram_w) AM_BASE(&gumbo_bg_videoram) // bg tilemap
ADDRESS_MAP_END

static ADDRESS_MAP_START( dblpoint_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1c0100, 0x1c0101) AM_READ(input_port_0_word_r)
	AM_RANGE(0x1c0200, 0x1c0201) AM_READ(input_port_1_word_r)
	AM_RANGE(0x1c0300, 0x1c0301) AM_READ(OKIM6295_status_0_lsb_r)
	AM_RANGE(0x1e0000, 0x1e3fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x1f0000, 0x1f0fff) AM_READ(MRA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dblpoint_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_WRITE(MWA16_RAM) // main ram
	AM_RANGE(0x1c0300, 0x1c0301) AM_WRITE(OKIM6295_data_0_lsb_w)
	AM_RANGE(0x1b0000, 0x1b03ff) AM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x1e0000, 0x1e3fff) AM_WRITE(gumbo_fg_videoram_w) AM_BASE(&gumbo_fg_videoram) // fg tilemap
	AM_RANGE(0x1f0000, 0x1f0fff) AM_WRITE(gumbo_bg_videoram_w) AM_BASE(&gumbo_bg_videoram) // bg tilemap
ADDRESS_MAP_END

INPUT_PORTS_START( gumbo )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// "Rotate" - also IPT_START1
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// "Help"
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// "Rotate" - also IPT_START2
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// "Help"
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Helps" )			// "Power Count" in test mode
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPNAME( 0x0800, 0x0800, "Bonus Bar Level" )
	PORT_DIPSETTING(      0x0800, "Normal" )
	PORT_DIPSETTING(      0x0000, "High" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, "Easy" )
	PORT_DIPSETTING(      0x3000, "Normal" )
	PORT_DIPSETTING(      0x1000, "Hard" )
	PORT_DIPSETTING(      0x0000, "Hardest" )
	PORT_DIPNAME( 0x4000, 0x4000, "Picture View" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

INPUT_PORTS_START( mspuzzle )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0200, "Time Mode" )
	PORT_DIPSETTING(      0x0300, "0" )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Test" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "View Staff Credits" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Picture View" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

INPUT_PORTS_START( dblpoint )
	PORT_START	/* Inputs */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START /* DSW */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c00, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, "Very Hard" )
	PORT_DIPSETTING(      0x0400, "Hard" )
	PORT_DIPSETTING(      0x0800, "Normal" )
	PORT_DIPSETTING(      0x0c00, "Easy" )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Test" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Picture View" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

static struct GfxLayout gumbo_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2)+0, 8,RGN_FRAC(1,2)+8,  16,RGN_FRAC(1,2)+16,24,RGN_FRAC(1,2)+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static struct GfxLayout gumbo2_layout =
{
	4,4,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2)+0, 8,RGN_FRAC(1,2)+8 },
	{ 0*16, 1*16, 2*16, 3*16 },
	4*16
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &gumbo_layout,   0x0, 2  }, /* bg tiles */
	{ REGION_GFX2, 0, &gumbo2_layout,  0x0, 2  }, /* fg tiles */
	{ -1 } /* end of array */
};


static struct OKIM6295interface okim6295_interface =
{
	1,				/* 1 chip */
	{ 8500 },		/* frequency (Hz) */
	{ REGION_SOUND1 },	/* memory region */
	{ 47 }
};

static MACHINE_DRIVER_START( gumbo )
	MDRV_CPU_ADD_TAG("main", M68000, 14318180 /2)	 // or 10mhz? ?
	MDRV_CPU_PROGRAM_MAP(gumbo_readmem,gumbo_writemem)
	MDRV_CPU_VBLANK_INT(irq1_line_hold,1) // all the same

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_GFXDECODE(gfxdecodeinfo)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(gumbo)
	MDRV_VIDEO_UPDATE(gumbo)

	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(OKIM6295, okim6295_interface)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mspuzzle )
	MDRV_IMPORT_FROM(gumbo)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mspuzzle_readmem,mspuzzle_writemem)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dblpoint )
	MDRV_IMPORT_FROM(gumbo)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(dblpoint_readmem,dblpoint_writemem)
MACHINE_DRIVER_END

ROM_START( gumbo )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x20000, CRC(e09899e4) SHA1(b62876dc3ada8509b766a80f496f1227b6af0ced) )
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x20000, CRC(60e59acb) SHA1(dd11329374c8f63851ddf5af54c91f78fad4fd3d) )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(16fbe06b) SHA1(4e40e62341dc886fcabdb07f64217dc086f43c67) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 )
	ROM_LOAD( "u421.bin", 0x00000, 0x80000, CRC(42445132) SHA1(f29d09d040644c8ef12a1cfdfc0d066e8ed9b82d) )
	ROM_LOAD( "u420.bin", 0x80000, 0x80000, CRC(de1f0e2f) SHA1(3f46d19af48392794838a4b54f8c45b809c67d49) )

	ROM_REGION( 0x40000, REGION_GFX2, 0 ) /* BG Tiles */
	ROM_LOAD( "u512.bin", 0x00000, 0x20000, CRC(97741798) SHA1(3603e14511817da19f6819d5612728d333695e99) )
	ROM_LOAD( "u511.bin", 0x20000, 0x20000, CRC(1411451b) SHA1(941d5f311f727e3a8d41ecbbe35b687d48cc2cef) )
ROM_END

ROM_START( mspuzzle )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x40000, CRC(d9e63f12) SHA1(c826c604f101d68057fdebf1b231293e4b2811f0) )
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x40000, CRC(9c3fc677) SHA1(193606fe739dbf5f26962f91be968ca371b7fd74) )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(0a223a38) SHA1(e5aefbdbb09c18cc230bc852df3ea1defb1a21a8) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "u421.bin", 0x000000, 0x80000, CRC(5387ab3a) SHA1(69913fde1a323ab1356ef52bb4efbf12caed594c) )
	ROM_LOAD( "u420.bin", 0x100000, 0x80000, CRC(c3f892e6) SHA1(5e8e4ae45a0eebaf2bbad00b1208b68f3e81df0c) )
	ROM_LOAD( "u425.bin", 0x080000, 0x80000, CRC(f53a9042) SHA1(70fcc3aaef46282a888466454714dc59daeb174d) )
	ROM_LOAD( "u426.bin", 0x180000, 0x80000, CRC(c927e8da) SHA1(2219f99bce6b2b9a827177c83952813df1a32c72) )

	ROM_REGION( 0x80000, REGION_GFX2, 0 ) /* BG Tiles */
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, BAD_DUMP CRC(83588eba) SHA1(018054bd655063b838368fd872c0a3ff84de2d34) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(3d6b6c78) SHA1(3016423102b4d47c0f1296471cf1670258acc856) )
ROM_END

ROM_START( dblpoint )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d12.bin", 0x00001, 0x20000, CRC(44bc1bd9) SHA1(8b72909c53b09b9287bf90bcd8970bdf9c1b8798) )
	ROM_LOAD16_BYTE( "d13.bin", 0x00000, 0x20000, CRC(625a311b) SHA1(38fa0d240b253fcc8dc89438582a9c446410b636) )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "d11.bin", 0x00000, 0x40000, CRC(d35f975c) SHA1(03490c92afadbd24c5b75f0ab114a2681b65c10e) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 )
	ROM_LOAD( "d16.bin", 0x00000, 0x80000, CRC(afea0158) SHA1(dc97f9268533048690715f377fb35d70e7e5a53f) )
	ROM_LOAD( "d17.bin", 0x80000, 0x80000, CRC(c971dcb5) SHA1(40f15b3d61ea0325883f19f24f2b61e24bb12a98) )

	ROM_REGION( 0x80000, REGION_GFX2, 0 ) /* BG Tiles */
	ROM_LOAD( "d14.bin", 0x00000, 0x40000, CRC(41943db5) SHA1(2f245402f7bbaeca7e50161397ee45e7c7c90cfc) )
	ROM_LOAD( "d15.bin", 0x40000, 0x40000, CRC(6b899a51) SHA1(04114ec9695caaac722800ac1a4ffb563ec433c9) )
ROM_END

GAME( 1994, gumbo,    0, gumbo,    gumbo,    0, ROT0,  "Min Corp.", "Gumbo" )
GAME( 1994, mspuzzle, 0, mspuzzle, mspuzzle, 0, ROT90, "Min Corp.", "Miss Puzzle" )
GAME( 1995, dblpoint, 0, dblpoint, dblpoint, 0, ROT0,  "Dong Bang Electron", "Double Point" )
