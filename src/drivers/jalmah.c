/*******************************************************************************************

MJ-8956 HW games (c) 1989 Jaleco / NMK

driver by Angelo Salese, based on early work by David Haywood

Similar to the NMK16 board.

TODO:
-I think that the 0xf0000-0xf03ff area isn't work ram,because:
 -The first version of the MCU programs (daireika/mjzoomin) jump in that area.
  Almost surely the MCU shares that area and upload a 68k code,also because of the strange
  RAM checks that follows...
 -input ports located there.Program doesn't check these locations at P.O.S.T. and doesn't
  give any work ram error.

-Inputs doesn't work during gameplay because they are MCU protected.

-Fix daireika vblank irq (NOT lv 2).

-Fix kakumei2 GFX3 rom region.

-Fix kakumei & suchipi priorities.

-Fix suchipi sound banking.

-Merge this with NMK16 driver.

============================================================================================
Debug cheats:

-(kakumei)During start button screen,set $f0400 to 3 to enter into gameplay.

-(suchipi)During gameplay,set $f0400 to 6 then set $f07d4 to 1 to advance to next
level.


============================================================================================
daireika 68k irq table vectors
lev 1 : 0x64 : 0000 049e -
lev 2 : 0x68 : 0000 04ae -
lev 3 : 0x6c : 0000 049e -
lev 4 : 0x70 : 0000 091a -
lev 5 : 0x74 : 0000 0924 -
lev 6 : 0x78 : 0000 092e -
lev 7 : 0x7c : 0000 0938 -

mjzoomin 68k irq table vectors
lev 1 : 0x64 : 0000 048a -
lev 2 : 0x68 : 0000 049a - vblank
lev 3 : 0x6c : 0000 048a -
lev 4 : 0x70 : 0000 09ba - "write to Text RAM" (?)
lev 5 : 0x74 : 0000 09c4 - "write to Text RAM" (?)
lev 6 : 0x78 : 0000 09ce - "write to Text RAM" (?)
lev 7 : 0x7c : 0000 09d8 - "write to Text RAM" (?)

kakumei/kakumei2/suchipi 68k irq table vectors
lev 1 : 0x64 : 0000 0506 - rte
lev 2 : 0x68 : 0000 050a - vblank
lev 3 : 0x6c : 0000 051c - rte
lev 4 : 0x70 : 0000 0520 - rte
lev 5 : 0x74 : 0000 0524 - rte
lev 6 : 0x78 : 0000 0524 - rte
lev 7 : 0x7c : 0000 0524 - rte

Board:	MJ-8956

CPU:	68000-8
		M50747 (not dumped)
Sound:	M6295
OSC:	12.000MHz
		4.000MHz
*******************************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

static struct tilemap *tx_tilemap, *fg_tilemap,*md_tilemap,*bg_tilemap;
data16_t *jm_txvideoram, *jm_fgvideoram,*jm_mdvideoram,*jm_bgvideoram;
data16_t *jm_regs,*jm_ram;
data16_t *jm_scrollram,*jm_priorityram;
static UINT16 fgbank;

static int respcount;

static UINT32 bg_scan(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

static void jm_get_fg_tile_info(int tile_index)
{
	int code = jm_fgvideoram[tile_index];
	SET_TILE_INFO(
			3,
			(code & 0xfff) + ((fgbank & 0xf) << 12),
			code >> 12,
			0)
}

static void jm_get_tx_tile_info(int tile_index)
{
	int code = jm_txvideoram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0xfff,
			code >> 12,
			0)
}

static void jm_get_bg_tile_info(int tile_index)
{
	int code = jm_bgvideoram[tile_index];
	SET_TILE_INFO(
			1,
			code & 0xfff,
			code >> 12,
			0)
}

static void jm_get_md_tile_info(int tile_index)
{
	int code = jm_mdvideoram[tile_index];
	SET_TILE_INFO(
			2,
			code & 0xfff,
			code >> 12,
			0)
}

VIDEO_START( jalmah )
{
	bg_tilemap = tilemap_create(jm_get_bg_tile_info,bg_scan,TILEMAP_TRANSPARENT,16,16,256,32);
	fg_tilemap = tilemap_create(jm_get_fg_tile_info,bg_scan,TILEMAP_OPAQUE,16,16,256,32);
	tx_tilemap = tilemap_create(jm_get_tx_tile_info,tilemap_scan_cols,TILEMAP_TRANSPARENT,8,8,256,32);
	md_tilemap = tilemap_create(jm_get_md_tile_info,bg_scan,TILEMAP_TRANSPARENT,16,16,256,32);

	if(!bg_tilemap || !tx_tilemap || !fg_tilemap || !md_tilemap)
		return 1;

	jm_scrollram = auto_malloc(0x80);
	jm_priorityram = auto_malloc(0x40);

	if (!jm_scrollram)
		return 1;

	tilemap_set_transparent_pen(bg_tilemap,15);
//	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_transparent_pen(tx_tilemap,15);
	tilemap_set_transparent_pen(md_tilemap,15);

	return 0;
}

#define MCU_READ(_number_,_bit_,_offset_,_retval_)\
if((0xffff - input_port_##_number_##_word_r(0,0)) & _bit_) { jm_ram[_offset_-0x200] = _retval_; }


VIDEO_UPDATE( jalmah )
{
	tilemap_set_scrollx( fg_tilemap, 0, jm_scrollram[0] );
	tilemap_set_scrollx( md_tilemap, 0, jm_scrollram[1] );
	tilemap_set_scrollx( bg_tilemap, 0, jm_scrollram[2] );
	tilemap_set_scrollx( tx_tilemap, 0, jm_scrollram[3] );
	tilemap_set_scrolly( md_tilemap, 0, jm_scrollram[4] );
	tilemap_set_scrolly( fg_tilemap, 0, jm_scrollram[5] );
	tilemap_set_scrolly( bg_tilemap, 0, jm_scrollram[6] );
	tilemap_set_scrolly( tx_tilemap, 0, jm_scrollram[7] );

//	usrintf_showmessage("%04x %04x %04x %04x",jm_priorityram[0],jm_priorityram[1],jm_priorityram[2],jm_priorityram[3]);

//	MCU_READ(2,0x1000,0x44a/2,0x10);/*REACH in suchipi*/

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,md_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
}


WRITE16_HANDLER( jm_fgvideoram_w )
{
	int oldword = jm_fgvideoram[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	if (oldword != newword)
	{
		jm_fgvideoram[offset] = newword;
		tilemap_mark_tile_dirty(fg_tilemap,offset);
	}
}

WRITE16_HANDLER( jm_mdvideoram_w )
{
	int oldword = jm_mdvideoram[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	if (oldword != newword)
	{
		jm_mdvideoram[offset] = newword;
		tilemap_mark_tile_dirty(md_tilemap,offset);
	}
}

WRITE16_HANDLER( jm_txvideoram_w )
{
	int oldword = jm_txvideoram[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	if (oldword != newword)
	{
		jm_txvideoram[offset] = newword;
		tilemap_mark_tile_dirty(tx_tilemap,offset);
	}
}

WRITE16_HANDLER( jm_bgvideoram_w )
{
	int oldword = jm_bgvideoram[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	if (oldword != newword)
	{
		jm_bgvideoram[offset] = newword;
		tilemap_mark_tile_dirty(bg_tilemap,offset);
	}
}

WRITE16_HANDLER( jalmah_tilebank_w )
{
	/*
	 xxxx ---- fg bank (used by suchipi)
	 ---- xxxx ?
	*/
//	usrintf_showmessage("Write to tilebank %02x",data);
	if (ACCESSING_LSB)
	{
		if (fgbank != (data & 0xff))
		{
			fgbank = (data & 0xf0) >> 4;
			tilemap_mark_all_tiles_dirty(fg_tilemap);
		}
	}
}

/*1bc8*/
/*For debug purpose*/
static READ16_HANDLER( jalmah_reg_r )
{
	switch(offset)
	{
		case (0x200/2):
	//		usrintf_showmessage("%04x %04x",activecpu_get_pc(),offset*2);
			return input_port_2_word_r(0,0);
		case (0x202/2):
	//		usrintf_showmessage("%04x %04x",activecpu_get_pc(),offset*2);
			return input_port_3_word_r(0,0);
		case (0x204/2):
	//		usrintf_showmessage("%04x %04x",activecpu_get_pc(),offset*2);
			return input_port_4_word_r(0,0);
	}
	usrintf_showmessage("%06x: Read to input area %04x",activecpu_get_pc(),offset*2);
	return jm_regs[offset];
}



static WRITE16_HANDLER( jalmah_reg_w )
{
	COMBINE_DATA(&jm_regs[offset]);
	switch(offset)
	{
		case (0x200/2):
		case (0x202/2):
		case (0x204/2):
			break;
	}
	usrintf_showmessage("Write to input area %06x",activecpu_get_pc());
}

static WRITE16_HANDLER( jalmah_scroll_w )
{
	switch(offset+(0x10))
	{
		/*Probably these are just video regs*/
		case (0x24/2): jm_priorityram[0] = data; break;
		case (0x2c/2): jm_priorityram[1] = data; break;
		case (0x34/2): jm_priorityram[2] = data; break;
		case (0x3c/2): jm_priorityram[3] = data; break;
		case (0x20/2): jm_scrollram[0] = data; break;
		case (0x28/2): jm_scrollram[1] = data; break;
		case (0x30/2): jm_scrollram[2] = data; break;
		case (0x38/2): jm_scrollram[3] = data; break;
		case (0x22/2): jm_scrollram[5] = data; break;
		case (0x2a/2): jm_scrollram[4] = data; break;
		case (0x32/2): jm_scrollram[6] = data; break;
		case (0x3a/2): jm_scrollram[7] = data; break;
	}
}

static ADDRESS_MAP_START( jalmah, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x080002, 0x080003) AM_READ(input_port_1_word_r)
	//       0x080004, 0x080005  MCU read,different for each game
	AM_RANGE(0x080006, 0x080007) AM_READ(input_port_2_word_r)
	AM_RANGE(0x080008, 0x080009) AM_READ(input_port_3_word_r)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(jalmah_tilebank_w)
	AM_RANGE(0x080020, 0x08003f) AM_WRITE(jalmah_scroll_w)
//	AM_RANGE(0x080006, 0x0800ff) AM_READ(jalmah_unk_r)
//	AM_RANGE(0x080000, 0x08003f) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x080040, 0x080041) AM_READWRITE(OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w)
	AM_RANGE(0x088000, 0x0887ff) AM_READWRITE(MRA16_RAM, paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE(&paletteram16) /* Palette RAM */
	AM_RANGE(0x090000, 0x093fff) AM_READWRITE(MRA16_RAM, jm_fgvideoram_w) AM_BASE(&jm_fgvideoram)
	AM_RANGE(0x094000, 0x097fff) AM_READWRITE(MRA16_RAM, jm_mdvideoram_w) AM_BASE(&jm_mdvideoram)
	AM_RANGE(0x098000, 0x09bfff) AM_READWRITE(MRA16_RAM, jm_bgvideoram_w) AM_BASE(&jm_bgvideoram)
	AM_RANGE(0x09c000, 0x09ffff) AM_READWRITE(MRA16_RAM, jm_txvideoram_w) AM_BASE(&jm_txvideoram)
	AM_RANGE(0x0f0000, 0x0f03ff) AM_READ(jalmah_reg_r) AM_WRITE(jalmah_reg_w) AM_BASE(&jm_regs)
	AM_RANGE(0x0f0400, 0x0fffff) AM_RAM AM_BASE(&jm_ram)
ADDRESS_MAP_END

INPUT_PORTS_START( jalmah )
	/*System port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Dip-SW port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin / 99 Credits" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Mahjong Panel ports*/
	PORT_START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 START") 	PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 RON")   	PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 KAN")   	PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 REACH") 	PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 FF")  	PORT_CODE(KEYCODE_2) //???
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 CHI") 	PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 M")   	PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 PON") 	PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 N")   	PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 K")   	PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 I")   	PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 L")   	PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 J")		PORT_CODE(KEYCODE_J)
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 G")   	PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 E")   	PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 H")   	PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 F")		PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 C")		PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 A")		PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 D")		PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 B")		PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static struct GfxLayout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static struct GfxLayout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static struct GfxDecodeInfo jalmah_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &charlayout, 0x300, 16 },
	{ REGION_GFX2, 0, &tilelayout, 0x200, 16 },
	{ REGION_GFX3, 0, &tilelayout, 0x100, 16 },
	{ REGION_GFX4, 0, &tilelayout, 0x000, 16 },
	{ -1 } /* end of array */
};

static MACHINE_INIT (daireika)
{
	respcount = 0;
}

static struct OKIM6295interface m6295_interface =
{
	1,              	/* 1 chip */
	{ 4000000/165 },	/* unknown,but this sounds decently */
	{ REGION_SOUND1 },	/* memory region */
	{ 100 }				/* volume */
};

static MACHINE_DRIVER_START( jalmah )
	MDRV_CPU_ADD(M68000, 12000000/2)
	MDRV_CPU_PROGRAM_MAP(jalmah,0)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_GFXDECODE(jalmah_gfxdecodeinfo)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_VISIBLE_AREA(0*8, 40*8-1, 1*8, 30*8-1)
	MDRV_PALETTE_LENGTH(0x400)
	MDRV_MACHINE_INIT(daireika)

	MDRV_VIDEO_START(jalmah)
	MDRV_VIDEO_UPDATE(jalmah)

	MDRV_SOUND_ADD(OKIM6295, m6295_interface)
MACHINE_DRIVER_END




/*

Mahjong Daireikai (JPN Ver.)
(c)1989 Jaleco / NMK

*/

ROM_START( daireika )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj1.bin", 0x00001, 0x20000, CRC(3b4e8357) SHA1(1ad3e40ec6b6ff4f1c9c09d7b530f67b460151d8) )
	ROM_RELOAD(                 0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj2.bin", 0x00000, 0x20000, CRC(c54d2f9b) SHA1(d59fc5a9e5bbb96b3b6a43378f4f2215c368b671) )
	ROM_RELOAD(                 0x40000, 0x20000 )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "mj3.bin", 0x00000, 0x80000, CRC(65bb350c) SHA1(e77866f2d612a0973adc616717e7c89a37d6c48e) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "mj14.bin", 0x00000, 0x10000, CRC(c84c5577) SHA1(6437368d3be39739d62158590ecd373aa070a9b2) )

	ROM_REGION( 0x10000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "mj13.bin", 0x00000, 0x10000, CRC(c54bca14) SHA1(ee9c99858817aedd70bd6266b7a71c3c5ad00607) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "mj12.bin", 0x00000, 0x20000, CRC(236f809f) SHA1(9e15dd8a810a9d4f7f75f084d6bd277ea7d0e40a) )
	ROM_LOAD( "mj11.bin", 0x20000, 0x20000, CRC(14867c51) SHA1(b282b5048a55c9ad72ceb0d23f010a0fee78704f) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "mj10.bin", 0x00000, 0x80000, CRC(1f5509a5) SHA1(4dcdee0e159956cf73f5f85ce278479be2a9ca9f) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Channel Zoom In (JPN Ver.)
(c)1990 Jaleco

*/

ROM_START( mjzoomin )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "zoomin-1.bin", 0x00001, 0x20000, CRC(b8b04d30) SHA1(abb163a9965421b4d92114bba974ccb13bb57f5a) )
	ROM_RELOAD(                      0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "zoomin-2.bin", 0x00000, 0x20000, CRC(c7eb982c) SHA1(9006ded2aa1fef38bde114110d76b20747c32658) )
	ROM_RELOAD(                      0x40000, 0x20000 )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "zoomin-3.bin", 0x00000, 0x80000, CRC(07d7b8cd) SHA1(e05ce80ffb945b04f93f8c49d0c840b0bff6310b) )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "zoomin14.bin", 0x00000, 0x20000, CRC(4e32aa45) SHA1(450a3449ca8b4f0dfe8b62cceaee9366eaf3dc3d) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "zoomin13.bin", 0x00000, 0x20000, CRC(888d79fe) SHA1(eb9671d4c7608edd1231dc0cae47aab2430cbd66) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "zoomin12.bin", 0x00000, 0x40000, CRC(b0b94554) SHA1(10490b7475810910140ce075e62f604b914e5511) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "zoomin10.bin", 0x00000, 0x80000, CRC(40aec575) SHA1(ef7a3c7a94523c5967ab774936b873c9629e0e44) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei (JPN Ver.)
(c)1990 Jaleco


*/

ROM_START( kakumei )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-re-1.bin", 0x00001, 0x20000, CRC(b90215be) SHA1(10384237f734836acefb4b5f53a6ddd9054d63ff) )
	ROM_RELOAD(                     0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj-re-2.bin", 0x00000, 0x20000, CRC(37eff266) SHA1(1d9e88c0270daadfafff1f73eb617e77b1d199d6) )
	ROM_RELOAD(                     0x40000, 0x20000 )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "rom3.bin", 0x00000, 0x40000, CRC(c9b7a526) SHA1(edec57e66d4ff601c8fdef7b1405af84a3f3d883) )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "rom14.bin", 0x00000, 0x20000, CRC(63e88dd6) SHA1(58734c8caf1b1ddc4cf0437ffd8109292b76c4e1) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "rom13.bin", 0x00000, 0x20000, CRC(9bef4fc2) SHA1(6598ab9dba513efcda01e47cc7752b47a97f2c6a) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "rom12.bin", 0x00000, 0x40000, CRC(31620a61) SHA1(11593ca7760e1a628e63aa48d9ad3800cf7af275) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "rom10.bin", 0x00000, 0x80000, CRC(88366377) SHA1(163a08415a631c8a09a0a55bc2819988d850f2ad) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei2 Princess League (JPN Ver.)
(c)1992 Jaleco

*/

ROM_START( kakumei2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-8956.1", 0x00001, 0x40000, CRC(db4ce32f) SHA1(1ae13627b9922143f462b1c3bbed87374f6e1667) )
	ROM_LOAD16_BYTE( "mj-8956.2", 0x00000, 0x40000, CRC(0f942507) SHA1(7ec2fbeb9a34dfc80c4df3de8397388db13f5c7c) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "92000-01.3", 0x00000, 0x80000, CRC(4b0ed440) SHA1(11961d217a41f92b60d5083a5e346c245f7db620) )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "mj-8956.14", 0x00000, 0x20000, CRC(2b2fe999) SHA1(d9d601e2c008791f5bff6e7b1340f754dd094201) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "mj-8956.13", 0x00000, 0x20000, CRC(afe93cf4) SHA1(1973dc5821c6df68e20f8a84b5c9ae281dd3f85f) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "mj-8956.12", 0x00000, 0x20000, CRC(43f7853d) SHA1(54fb523b27e79aa295900c478f09cc080fea0adf) )
	/*0x20000-0x40000 used*/

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "92000-02.10", 0x00000, 0x80000, CRC(338fa9b2) SHA1(05ba4b3c44249cf92be238bf53d6345dc49b0881) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Idol Janshi Su-Chi-Pi Special
(c)Jaleco 1994

CPU  : M68000P10
Sound: OKI M6295
OSC  : 12.000MHz 4.000MHz

MJ-8956
YSP-40101 171

1.bin - Main program ver.1.2 (27c2001)
2.bin - Main program ver.1.2 (27c2001)

3.bin - Sound data (27c4000)
4.bin - Sound data (27c4000)

7.bin  (27c4000) \
8.bin  (27c4000) |
9.bin  (27c4000) |
10.bin (27c4000) |
                 |- Graphics
12.bin (27c2001) |
                 |
13.bin (27c1001) |
                 |
14.bin (27c1001) /

pr92000a.prm (82s129) \
pr92000b.prm (82s129) |- Not dumped
pr93035.prm  (82s123) /

Custom chips:
GS-9000406 9345K5005 (80pin QFP) x4
GS-9000404 9248EP004 (44pin QFP)

Other chips:
MO-92000 (64pin DIP)
NEC D65012GF303 9050KX016 (80pin QFP) x4

*/

ROM_START( suchipi )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x40000, CRC(e37cc745) SHA1(73b3314d27a0332068e0d2bbc08d7401e371da1b) )
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x40000, CRC(42ecf88a) SHA1(7bb85470bc9f94c867646afeb91c4730599ea299) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "3.bin", 0x00000, 0x80000, CRC(691b5387) SHA1(b8bc9f904eab7653566042b18d89276d537ba586) )
	ROM_LOAD( "4.bin", 0x80000, 0x80000, CRC(3fe932a1) SHA1(9e768b901738ee9eba207a67c4fd19efb0035a68) )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "14.bin", 0x00000, 0x20000, CRC(e465a540) SHA1(10e19599ab90b0c0b6ef6ee41f16620bd1ba6800) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(99466044) SHA1(ca31b58a5d4656f95d80ddb9bc1f9a53f5f2446c) )

	ROM_REGION( 0x80000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "12.bin", 0x00000, 0x40000, CRC(146596eb) SHA1(f85e92e6dc9ebef5e67d28f1d450225cd2a2abaa) )

	ROM_REGION( 0x200000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "7.bin",  0x000000, 0x80000, CRC(18caf6f3) SHA1(3df6b257867487adcba1a05c8745413d9a15c3d7) )
	ROM_LOAD( "8.bin",  0x080000, 0x80000, CRC(0403399a) SHA1(8d39a68b3a1a431afe93ff485e837389a4502d0c) )
	ROM_LOAD( "9.bin",  0x100000, 0x80000, CRC(8a348246) SHA1(13516c48bdbe8d78e7517473ef2835a4dea2ce93) )
	ROM_LOAD( "10.bin", 0x180000, 0x80000, CRC(2b0d1afd) SHA1(40009b450901567052aa63c4629a2f7a10343e63) )

	/* the 3 missing proms should be the same as the other games */
	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END


/******************************************************************************************

MCU simulations

******************************************************************************************/

static READ16_HANDLER( daireika_mcu_r )
{
	static int resp[] = {	0x99, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x9c, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x23, 0x63, 0x00};
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	//usrintf_showmessage("%04x: mcu_r %02x",activecpu_get_pc(),res);

	return res;
}

static READ16_HANDLER( mjzoomin_mcu_r )
{
	static int resp[] = {	0x9c, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x99, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x35, 0x75, 0x00,
							0x36, 0x36, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	logerror("%04x: mcu_r %02x\n",activecpu_get_pc(),res);

	return res;
}

static READ16_HANDLER( kakumei_mcu_r )
{
	static int resp[] = {	0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	//usrintf_showmessage("%04x: mcu_r %02x",activecpu_get_pc(),res);

	return res;
}

static READ16_HANDLER( suchipi_mcu_r )
{
	static int resp[] = {	0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00,
							0x3e, 0x7e, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	//usrintf_showmessage("%04x: mcu_r %02x",activecpu_get_pc(),res);

	return res;
}


static DRIVER_INIT( daireika )
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, daireika_mcu_r );
}

static DRIVER_INIT( mjzoomin )
{
	UINT16 *ROM = (data16_t *)memory_region(REGION_CPU1);

	/*temp patch,program jumps to a RAM address???*/
	ROM[0x2a5c/2] = 0x4e71;
	ROM[0x2a5e/2] = 0x4e71;
	ROM[0x2a60/2] = 0x4e71;
	ROM[0x0a02/2] = 0x4e71;
	ROM[0x0a04/2] = 0x4e71;
	ROM[0x0a06/2] = 0x4e71;

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, mjzoomin_mcu_r );
}

static DRIVER_INIT( kakumei )
{
	UINT16 *RAM = (data16_t *)memory_region(REGION_CPU1);

	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, kakumei_mcu_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0200, 0xf0201, 0, 0, input_port_2_word_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0202, 0xf0203, 0, 0, input_port_3_word_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0204, 0xf0205, 0, 0, input_port_4_word_r );
//	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xf0200, 0xf0205, 0, 0, MWA16_NOP );

	RAM[0x227e/2] = 0x4e71;
}

static DRIVER_INIT( kakumei2 )
{
	UINT16 *RAM = (data16_t *)memory_region(REGION_CPU1);

	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, kakumei_mcu_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0200, 0xf0201, 0, 0, input_port_2_word_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0202, 0xf0203, 0, 0, input_port_3_word_r );
//	memory_install_read16_handler(0,  ADDRESS_SPACE_PROGRAM, 0xf0204, 0xf0205, 0, 0, input_port_4_word_r );
//	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xf0200, 0xf0205, 0, 0, MWA16_NOP );

	RAM[0x229a/2] = 0x4e71;
}

static DRIVER_INIT( suchipi )
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, suchipi_mcu_r );
}

/*First version of the MCU,not yet working*/
GAMEX( 1989, daireika, 0, jalmah, jalmah, daireika, ROT0, "Jaleco / NMK", "Mahjong Daireikai",					  GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAMEX( 1990, mjzoomin, 0, jalmah, jalmah, mjzoomin, ROT0, "Jaleco",       "Mahjong Channel Zoom In",              GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

/*Second version of the MCU,protection not fully understood,because of that inputs doesn't work*/
GAMEX( 1990, kakumei,  0, jalmah, jalmah, kakumei,  ROT0, "Jaleco",       "Mahjong Kakumei",                      GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAMEX( 1992, kakumei2, 0, jalmah, jalmah, kakumei2, ROT0, "Jaleco",       "Mahjong Kakumei 2 - Princess League",  GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAMEX( 1993, suchipi,  0, jalmah, jalmah, suchipi,  ROT0, "Jaleco",       "Idol Janshi Su-Chi-Pi Special",        GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
