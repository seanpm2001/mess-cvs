/******************************************************************************
	Motorola Evaluation Kit 6800 D2
	MEK6800D2

	machine Driver

	Juergen Buchmueller <pullmoll@t-online.de>, Jan 2000

******************************************************************************/
#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "vidhrdw/generic.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#if VERBOSE
#define LOG(x)	if( errorlog ) fprintf x
#else
#define LOG(x)						   /* x */
#endif

void init_mekd2(void)
{
	UINT8 *dst;
	int x, y, i;

	static char *seg7 =
	"....aaaaaaaaaaaaa." \
	"...f.aaaaaaaaaaa.b" \
	"...ff.aaaaaaaaa.bb" \
	"...fff.........bbb" \
	"...fff.........bbb" \
	"...fff.........bbb" \
	"..fff.........bbb." \
	"..fff.........bbb." \
	"..fff.........bbb." \
	"..ff...........bb." \
	"..f.ggggggggggg.b." \
	"..gggggggggggggg.." \
	".e.ggggggggggg.c.." \
	".ee...........cc.." \
	".eee.........ccc.." \
	".eee.........ccc.." \
	".eee.........ccc.." \
	"eee.........ccc..." \
	"eee.........ccc..." \
	"eee.........ccc..." \
	"ee.ddddddddd.cc..." \
	"e.ddddddddddd.c..." \
	".ddddddddddddd...." \
	"..................";


	static char *keys[24] =
	{
		"........................" \
		"........................" \
		"..........cccc.........." \
		"........cccccccc........" \
		"........cc....cc........" \
		".......cc......cc......." \
		".......cc.....ccc......." \
		".......cc....cccc......." \
		".......cc...cc.cc......." \
		".......cc..cc..cc......." \
		".......cc.cc...cc......." \
		".......cccc....cc......." \
		".......ccc.....cc......." \
		"........cc....cc........" \
		"........cccccccc........" \
		"..........cccc.........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		"...........cc..........." \
		"..........ccc..........." \
		".........cccc..........." \
		"........cc.cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"...........cc..........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		"........cccccccc........" \
		".......cccccccccc......." \
		".......cc......cc......." \
		"...............cc......." \
		"..............ccc......." \
		"............cccc........" \
		".........cccccc........." \
		"........cccc............" \
		".......ccc.............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccccccc......." \
		".......cccccccccc......." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".........cccccc........." \
		"........cccccccc........" \
		".......ccc....ccc......." \
		".......cc......cc......." \
		"...............cc......." \
		"..............ccc......." \
		"..........cccccc........" \
		"..........cccccc........" \
		"..............ccc......." \
		"...............cc......." \
		".......cc......cc......." \
		".......ccc....ccc......." \
		"........cccccccc........" \
		".........cccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		"..............c........." \
		".............cc........." \
		"............ccc........." \
		"...........cccc........." \
		"..........cc.cc........." \
		".........cc..cc........." \
		"........cc...cc........." \
		".......cc....cc........." \
		"......ccccccccccc......." \
		"......ccccccccccc......." \
		".............cc........." \
		".............cc........." \
		".............cc........." \
		".............cc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccccc......." \
		".......cccccccccc......." \
		".......cc..............." \
		".......cc..............." \
		".......cccccccc........." \
		".......ccccccccc........" \
		"..............ccc......." \
		"...............cc......." \
		"...............cc......." \
		"...............cc......." \
		".......cc......cc......." \
		".......ccc....ccc......." \
		"........cccccccc........" \
		".........cccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".........cccccc........." \
		"........cccccccc........" \
		".......ccc....ccc......." \
		".......cc..............." \
		".......cc..............." \
		".......cc.cccc.........." \
		".......ccccccccc........" \
		".......cccc...cc........" \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......ccc....ccc......." \
		"........cccccccc........" \
		".........cccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccccc......." \
		".......cccccccccc......." \
		"...............cc......." \
		"..............ccc......." \
		".............ccc........" \
		"............ccc........." \
		"...........ccc.........." \
		"..........ccc..........." \
		".........ccc............" \
		"........ccc............." \
		".......ccc.............." \
		"......ccc..............." \
		"......ccc..............." \
		"......cc................" \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".........cccccc........." \
		"........cccccccc........" \
		".......ccc....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		"........cc....cc........" \
		".........cccccc........." \
		"........cccccccc........" \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......ccc....ccc......." \
		"........cccccccc........" \
		".........cccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".........cccccc........." \
		"........cccccccc........" \
		".......ccc....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		"........ccc...ccc......." \
		"........ccccccccc......." \
		"..........cccc.cc......." \
		"...............cc......." \
		"...............cc......." \
		".......ccc....ccc......." \
		"........cccccccc........" \
		".........cccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		"...........cc..........." \
		"..........cccc.........." \
		"..........cccc.........." \
		".........cccccc........." \
		".........cc..cc........." \
		".........cc..cc........." \
		"........cccccccc........" \
		"........cccccccc........" \
		"........cc....cc........" \
		".......ccc....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		"......ccc......ccc......" \
		"......cc........cc......" \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccc........." \
		".......ccccccccc........" \
		".......cc.....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc.....cc........" \
		".......cccccccc........." \
		".......ccccccccc........" \
		".......cc.....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc.....ccc......." \
		".......ccccccccc........" \
		".......cccccccc........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		"........cccccccc........" \
		".......cccccccccc......." \
		".......cc......cc......." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc......cc......." \
		".......cccccccccc......." \
		"........cccccccc........" \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......ccccccc.........." \
		".......cccccccc........." \
		".......cc....ccc........" \
		".......cc.....cc........" \
		".......cc.....ccc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc......cc......." \
		".......cc.....ccc......." \
		".......cc.....cc........" \
		".......cc....ccc........" \
		".......cccccccc........." \
		".......ccccccc.........." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccccc......." \
		".......cccccccccc......." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccc..........." \
		".......cccccc..........." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccccccc......." \
		".......cccccccccc......." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccccc......." \
		".......cccccccccc......." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccc..........." \
		".......cccccc..........." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		".....ccc.......ccc......" \
		".....cccc.....cccc......" \
		".....cc.cc...cc.cc......" \
		".....cc..cc.cc..cc......" \
		".....cc...ccc...cc......" \
		".....cc....c....cc......" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		".....cc.........cc......" \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".......cccccccccc......." \
		".......cccccccccc......." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccc..........." \
		".......cccccc..........." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cc..............." \
		".......cccccccccc......." \
		".......cccccccccc......." \
		"........................" \
		"........................",

		"........................" \
        "........................" \
        "......ccccccccc........." \
        "......cccccccccc........" \
        "......cc......ccc......." \
        "......cc.......cc......." \
        "......cc.......cc......." \
        "......cc......ccc......." \
        "......cccccccccc........" \
        "......ccccccccc........." \
        "......cc...ccc.........." \
        "......cc....ccc........." \
        "......cc.....ccc........" \
        "......cc......ccc......." \
        "......cc.......ccc......" \
        "......cc........ccc....." \
        "........................" \
        "........................",

		"........................" \
        "........................" \
        "........ccccccccc......." \
        ".......cccccccccc......." \
        "......ccc......cc......." \
        "......cc.......cc......." \
        "......cc................" \
        "......cc................" \
        "......cc...cccccc......." \
        "......cc...cccccc......." \
        "......cc.......cc......." \
        "......cc.......cc......." \
        "......cc.......cc......." \
        "......ccc.....ccc......." \
        ".......cccccccccc......." \
        ".........ccccc.cc......." \
        "........................" \
        "........................",

		"........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................" \
        "........................",

        "........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................" \
		"........................",

		"........................" \
		"........................" \
		".bbbbbbbbbbbbbbbbbbbbbb." \
		".b....................b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".b.cccccccc...........b." \
		".bbbbbbbbbbbbbbbbbbbbbb." \
		"........................"};

	dst = memory_region(REGION_GFX1);
	memset(dst, 0, 128 * 24 * 24 / 8);
	for (i = 0; i < 128; i++)
	{
		for (y = 0; y < 24; y++)
		{
			for (x = 0; x < 18; x++)
			{
				switch (seg7[y * 18 + x])
				{
				case 'a':
					if (i & 1)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'b':
					if (i & 2)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'c':
					if (i & 4)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'd':
					if (i & 8)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'e':
					if (i & 16)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'f':
					if (i & 32)
						*dst |= 0x80 >> (x & 7);
					break;
				case 'g':
					if (i & 64)
						*dst |= 0x80 >> (x & 7);
					break;
				}
				if ((x & 7) == 7)
					dst++;
			}
			dst++;
		}
	}

	dst = memory_region(2);
	memset(dst, 0, 24 * 18 * 24 / 8);
	for (i = 0; i < 24; i++)
	{
		for (y = 0; y < 18; y++)
		{
			for (x = 0; x < 24; x++)
			{
				switch (keys[i][y * 24 + x])
				{
				case 'a':
					*dst |= 0x80 >> ((x & 3) * 2);
					break;
				case 'b':
					*dst |= 0x40 >> ((x & 3) * 2);
					break;
				case 'c':
					*dst |= 0xc0 >> ((x & 3) * 2);
					break;
				}
				if ((x & 3) == 3)
					dst++;
			}
		}
	}
}

void mekd2_init_machine(void)
{
}

int mekd2_rom_load(int id)
{
	const char magic[] = "MEK6800D2";
	char buff[9];
	void *file;

	//if (name && name[0])
	//{
		file = image_fopen(IO_CARTSLOT, id, OSD_FILETYPE_IMAGE_RW, 0);
		if (file)
		{
			UINT16 addr, size;
			UINT8 ident, *RAM = memory_region(REGION_CPU1);

			osd_fread(file, buff, sizeof (buff));
			if (memcmp(buff, magic, sizeof (buff)))
			{
				LOG((errorlog, "mekd2_rom_load: magic '%s' not found\n", magic));
				return 1;
			}
			osd_fread_lsbfirst(file, &addr, 2);
			osd_fread_lsbfirst(file, &size, 2);
			osd_fread(file, &ident, 1);
			LOG((errorlog, "mekd2_rom_load: $%04X $%04X $%02X\n", addr, size, ident));
			while (size-- > 0)
				osd_fread(file, &RAM[addr++], 1);
			osd_fclose(file);
		}
	//}

	return 0;
}

int mekd2_rom_id(int id)
{
	const char magic[] = "MEK6800D2";
	char buff[9];
	void *file;

	file = image_fopen(IO_CARTSLOT, id, OSD_FILETYPE_IMAGE_RW, 0);
	if (file)
	{
		osd_fread(file, buff, sizeof (buff));
		if (memcmp(buff, magic, sizeof (buff)) == 0)
		{
			LOG((errorlog, "mekd2_rom_id: magic '%s' found\n", magic));
			return 1;
		}
	}
	return 0;
}

int mekd2_interrupt(void)
{
	return ignore_interrupt();
}

