/***************************************************************************

  gb.c

  Machine file to handle emulation of the Nintendo GameBoy.

***************************************************************************/
#define __MACHINE_GB_C

#include "driver.h"
#include "machine/gb.h"
#include "cpu/z80gb/z80gb.h"
#include "includes/gb.h"

static UINT8 MBCType;				   /* MBC type: 0 for none                        */
static UINT8 CartType;				   /* Cart Type (battery, ram, timer etc)         */
static UINT8 *ROMMap[512];			   /* Addresses of ROM banks                      */
static UINT16 ROMBank;				   /* Number of ROM bank currently used           */
static UINT8 ROMMask;				   /* Mask for the ROM bank number                */
static int ROMBanks;				   /* Total number of ROM banks                   */
static UINT8 *RAMMap[256];			   /* Addresses of RAM banks                      */
static UINT8 RAMBank;				   /* Number of RAM bank currently used           */
static UINT8 RAMMask;				   /* Mask for the RAM bank number                */
static int RAMBanks;				   /* Total number of RAM banks                   */
static UINT32 TCount, TStep;		   /* Timer counter and increment            */
static UINT32 SIOCount;				   /* Serial I/O counter                     */

#define Verbose 0x00
#define SGB 0
#define CheckCRC 1
#define LineDelay 0
#define IFreq 60

UINT8 *gb_ram;

void gb_init_machine (void)
{
	int I;

	gb_ram = memory_region (REGION_CPU1);

	ROMBank = 1;
	RAMBank = 0;
	cpu_setbank (1, ROMMap[ROMBank] ? ROMMap[ROMBank] : gb_ram + 0x4000);
	cpu_setbank (2, RAMMap[RAMBank] ? RAMMap[RAMBank] : gb_ram + 0xA000);

	TStep = 32768;
	TCount = 0;

	gb_chrgen = gb_ram + 0x8800;
	gb_bgdtab = gb_wndtab = gb_ram + 0x9800;
	LCDCONT = 0x81;
	LCDSTAT = 0x00;
	SCROLLX = SCROLLY = 0x00;
	WNDPOSX = WNDPOSY = 0x00;
	CURLINE = 0x00;
	CMPLINE = 0x00;
	BGRDPAL = 0xFC;
	SPR0PAL = SPR1PAL = 0xFF;
	IFLAGS = ISWITCH = 0x00;
	TIMECNT = TIMEMOD = TIMEFRQ = 0x00;
	SIODATA = 0x00;
	SIOCONT = 0x7E;

	for (I = 0; I < 4; I++)
	{
		gb_bpal[I] = gb_wpal[I] = I;
		gb_spal0[I] = I + 4;
		gb_spal1[I] = I + 8;
	}

	BGRDPAL = SPR0PAL = SPR1PAL = 0xE4;

	/* Initialise the timer */
	gb_w_io (0x07, gb_ram [0xFF07]);

	/* Initialise the Sound Registers */
	gameboy_sound_w(0xFF26,0xF1); /*Gameboy, F0 for SGB*/ /* set this first */
	gameboy_sound_w(0xFF10,0x80);
	gameboy_sound_w(0xFF11,0xBF);
	gameboy_sound_w(0xFF12,0xF3);
	gameboy_sound_w(0xFF14,0xBF);
	gameboy_sound_w(0xFF16,0x3F);
	gameboy_sound_w(0xFF17,0x00);
	gameboy_sound_w(0xFF19,0xBF);
	gameboy_sound_w(0xFF1A,0x7F);
	gameboy_sound_w(0xFF1B,0xFF);
	gameboy_sound_w(0xFF1C,0x9F);
	gameboy_sound_w(0xFF1E,0xBF);
	gameboy_sound_w(0xFF20,0xFF);
	gameboy_sound_w(0xFF21,0x00);
	gameboy_sound_w(0xFF22,0x00);
	gameboy_sound_w(0xFF23,0xBF);
	gameboy_sound_w(0xFF24,0x77);
	gameboy_sound_w(0xFF25,0xF3);
}

void gb_shutdown_machine(void)
{
	int I;
	void *f;
	char filename[19];
	char cartname[16];

	/* Don't save if there was no battery */
	if( !(CartType & BATTERY) )
		return;

	/* NOTE: The reason we save the carts RAM this way instead of using MAME's
	   built in macros is because they force the filename to be the name of
	   the machine.  We need to have a separate name for each game.
	   We'll put "gb_" in front to signify it's a Gameboy NVRAM file. */

	/* Build the filename */
	strncpy(cartname, (char *)&gb_ram[0x0134], 16);
	cartname[16] = '\0';
	sprintf( filename, "gb_%s", cartname );

	/* Save the RAM */
	if((f = osd_fopen(filename, 0, OSD_FILETYPE_NVRAM, 1)) != 0 )
	{
		for( I = 0; I < RAMBanks; I++ )
		{
			osd_fwrite( f, RAMMap[I], 0x2000 );
		}
		osd_fclose(f);
	}
}

WRITE_HANDLER ( gb_rom_bank_select )
{
	/* No need to bank switch if there is no controller */
	if( !MBCType )
		return;

	data &= ROMMask;

	/* Selecting bank 0 == selecting bank 1 except with an MBC5 */
	if( !data && MBCType != MBC5 )
		data = 1;

	if( ROMMask )
	{
		switch( MBCType )
		{
		case MBC1:	/* Need to handle different modes */
		case MBC3:
		case HUC1:	/* Probably wrong */
		case HUC3:	/* Probably wrong */
			ROMBank = data;
			break;
		case MBC2:
			/* The least significant bit of the upper address byte must be 1 */
			if( offset & 0x0100 )
				ROMBank = data;
			break;
		case MBC5:
			/* MBC5 has a 9 bit bank select */
			if( offset < 0x1000 )
			{
				ROMBank = (ROMBank & 0x100 ) | data;
			}
			else
			{
				ROMBank = (ROMBank & 0xFF ) | (((INT16)data & 0x1) << 8);
			}
			break;
		}

		/* Switch banks */
		cpu_setbank (1, ROMMap[ROMBank] ? ROMMap[ROMBank] : gb_ram + 0x4000);
		if (Verbose & 0x08)
			printf ("ROM: Bank %d selected\n", ROMBank);
	}
}

WRITE_HANDLER ( gb_ram_bank_select )
{
	/* No need to bank switch if there is no controller */
	if( !MBCType )
		return;

	data &= RAMMask;
	if( RAMMask )
	{
		switch( MBCType )
		{
		case MBC1:	/* Need to handle different modes */
		case MBC3:	/* Need to handle RTC */
		case MBC5:
			RAMBank = data;
			break;
		}

		/* Switch banks */
		cpu_setbank (2, RAMMap[RAMBank] ? RAMMap[RAMBank] : gb_ram + 0xA000);
		if (Verbose & 0x08)
			printf ("RAM: Bank %d selected\n", RAMBank);
	}
}

/*
WRITE_HANDLER ( gb_w_ie )
{
	gb_ram[0xFFFF] = data & 0x1F;
}*/

WRITE_HANDLER ( gb_w_io )
{
	static UINT8 timer_shifts[4] =
	{10, 4, 6, 8};
	UINT8 *P;
	static UINT8 bit_count = 0, byte_count = 0, start = 0, rest = 0;
	static UINT8 sgb_data[16];
	static UINT8 controller_no = 0, controller_mode = 0;

	offset += 0xFF00;

	/*logerror("Hardware hit %04x, %02x\n", offset, data);*/
	switch (offset)
	{
	case 0xFF00:
		if (SGB)
		{
			switch (data & 0x30)
			{
			case 0x00:				   /* start condition */
				if (start)
					puts ("SGB: Start condition before end of transfer ??");
				bit_count = 0;
				byte_count = 0;
				start = 1;
				rest = 0;
				JOYPAD = 0x0F & ((readinputport (0) >> 4) | readinputport (0) | 0xF0);
				break;
			case 0x10:				   /* data true */
				if (rest)
				{
					if (byte_count == 16)
					{
						puts ("SGB: end of block is not zero!");
						start = 0;
					}
					sgb_data[byte_count] >>= 1;
					sgb_data[byte_count] |= 0x80;
					bit_count++;
					if (bit_count == 8)
					{
						bit_count = 0;
						byte_count++;
					}
					rest = 0;
				}
				JOYPAD = 0x1F & ((readinputport (0) >> 4) | 0xF0);
				break;
			case 0x20:				   /* data false */
				if (rest)
				{
					if (byte_count == 16)
					{
						printf
								("SGB: command: %02X packets: %d data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
								sgb_data[0] >> 3, sgb_data[0] & 0x07, sgb_data[1], sgb_data[2], sgb_data[3],
								sgb_data[4], sgb_data[5], sgb_data[6], sgb_data[7],
								sgb_data[8], sgb_data[9], sgb_data[10], sgb_data[11],
								sgb_data[12], sgb_data[13], sgb_data[14], sgb_data[15]);
						if ((sgb_data[0] >> 3) == 0x11)
						{
							printf ("multicontroller command, data= %02X\n", sgb_data[1]);
							if (sgb_data[1] == 0x00)
								controller_mode = 0;
							else if (sgb_data[1] == 0x01)
								controller_mode = 2;
						}
						start = 0;
/*						Trace=1; */
					}
					sgb_data[byte_count] >>= 1;
					bit_count++;
					if (bit_count == 8)
					{
						bit_count = 0;
						byte_count++;
					}
					rest = 0;
				}
				JOYPAD = 0x2F & (readinputport (0) | 0xF0);
				break;
			case 0x30:				   /* rest condition */
				if (start)
					rest = 1;
				if (controller_mode)
				{
					controller_no++;
					if (controller_no == controller_mode)
						controller_no = 0;
					JOYPAD = 0x3F - controller_no;
				}
				else
					JOYPAD = 0x3F;
				break;
			}
/*                   printf("%d%d\n", (data&0x10)? 1:0, (data&0x20)? 1:0); */
		}
		else
		{
			JOYPAD = 0xCF | data;
			if (!(data & 0x20))
				JOYPAD &= (readinputport (0) >> 4) | 0xF0;
			if (!(data & 0x10))
				JOYPAD &= readinputport (0) | 0xF0;
		}
		return;
	case 0xFF01:						/* SB - Serial transfer data */
		break;
	case 0xFF02:						/* SC - SIO control */
		if ((data & 0x81) == 0x81)	   /* internal clock && enable */
		{
			SIODATA = 0xFF;
			SIOCount = 8;
		}
		else						   /* external clock || disable */
			SIOCount = 0;
		break;
	case 0xFF04:						/* DIV - Divider register */
		gb_divcount = 0;
		return;
	case 0xFF05:						/* TIMA - Timer counter */
		gb_timer_count = data << gb_timer_shift;
	case 0xFF07:						/* TAC - Timer control */
		gb_timer_shift = timer_shifts[data & 0x03];
		data |= 0xF8;
		break;
	case 0xFF0F:						/* IF - Interrupt flag */
		data &= 0x1F;
		break;
	case 0xFF40:
		gb_chrgen = gb_ram + ((data & 0x10) ? 0x8000 : 0x8800);
		gb_tile_no_mod = (data & 0x10) ? 0x00 : 0x80;
		gb_bgdtab = gb_ram + ((data & 0x08) ? 0x9C00 : 0x9800);
		gb_wndtab = gb_ram + ((data & 0x40) ? 0x9C00 : 0x9800);
		break;
	case 0xFF41:
		data = (data & 0xF8) | (LCDSTAT & 0x07);
		break;
	case 0xFF44:
		data = 0;
		break;
	case 0xFF46:
		P = gb_ram + 0xFE00;
		offset = (UINT16) data << 8;
		for (data = 0; data < 0xA0; data++)
			*P++ = cpu_readmem16 (offset++);
		return;
#if 1
	case 0xFF47:
		gb_bpal[0] = Machine->remapped_colortable[(data & 0x03)];
		gb_bpal[1] = Machine->remapped_colortable[(data & 0x0C) >> 2];
		gb_bpal[2] = Machine->remapped_colortable[(data & 0x30) >> 4];
		gb_bpal[3] = Machine->remapped_colortable[(data & 0xC0) >> 6];
		/* This is so we can assign different colours to window tiles,
		   even though the window shares the same palette data as the
		   background */
		gb_wpal[0] = Machine->remapped_colortable[(data & 0x03) + 12];
		gb_wpal[1] = Machine->remapped_colortable[((data & 0x0C) >> 2) + 12];
		gb_wpal[2] = Machine->remapped_colortable[((data & 0x30) >> 4) + 12];
		gb_wpal[3] = Machine->remapped_colortable[((data & 0xC0) >> 6) + 12];
		break;
	case 0xFF48:
		gb_spal0[0] = Machine->remapped_colortable[(data & 0x03) + 4];
		gb_spal0[1] = Machine->remapped_colortable[((data & 0x0C) >> 2) + 4];
		gb_spal0[2] = Machine->remapped_colortable[((data & 0x30) >> 4) + 4];
		gb_spal0[3] = Machine->remapped_colortable[((data & 0xC0) >> 6) + 4];
		break;
	case 0xFF49:
		gb_spal1[0] = Machine->remapped_colortable[(data & 0x03) + 8];
		gb_spal1[1] = Machine->remapped_colortable[((data & 0x0C) >> 2) + 8];
		gb_spal1[2] = Machine->remapped_colortable[((data & 0x30) >> 4) + 8];
		gb_spal1[3] = Machine->remapped_colortable[((data & 0xC0) >> 6) + 8];
		break;
#else
	case 0xFF47:
		gb_bpal[0] = Machine->pens[(data & 0x03)];
		gb_bpal[1] = Machine->pens[(data & 0x0C) >> 2];
		gb_bpal[2] = Machine->pens[(data & 0x30) >> 4];
		gb_bpal[3] = Machine->pens[(data & 0xC0) >> 6];
		break;
	case 0xFF48:
		gb_spal0[0] = Machine->pens[(data & 0x03) ];
		gb_spal0[1] = Machine->pens[((data & 0x0C) >> 2) ];
		gb_spal0[2] = Machine->pens[((data & 0x30) >> 4) ];
		gb_spal0[3] = Machine->pens[((data & 0xC0) >> 6) ];
		break;
	case 0xFF49:
		gb_spal1[0] = Machine->pens[(data & 0x03) ];
		gb_spal1[1] = Machine->pens[((data & 0x0C) >> 2) ];
		gb_spal1[2] = Machine->pens[((data & 0x30) >> 4) ];
		gb_spal1[3] = Machine->pens[((data & 0xC0) >> 6) ];
		break;
#endif
	case 0xFFFF:
		data &= 0x1F;
		break;
	default:

		/* Sound Registers */
		if ((offset >= 0xFF10) && (offset <= 0xFF26))
		{
			gb_ram [offset] = data;
			gameboy_sound_w(offset, data);
			return;
		}

		/*Pre defined Waveform Area */
		if ((offset >= 0xFF30) && (offset <= 0xFF3F))
		{
			gb_ram [offset] = data;
			return;
		}
	}
	gb_ram [offset] = data;
}

READ_HANDLER ( gb_ser_regs )
{
	offset += 0xFF00;

	switch(offset)
	{
		case 0xFF00:
						/*logerror("Location read 0xff00\n");*/
						break;
		case 0xFF01:
						/*logerror("Location read 0xff01\n");*/
						break;
		case 0xFF02:
						/*logerror("Location read 0xff02\n");*/
						break;
		case 0xFF03:
						/*logerror("Location read 0xff03\n");*/
						break;
	}

	return gb_ram[offset];

}

READ_HANDLER ( gb_r_divreg )
{
	return ((gb_divcount >> 8) & 0xFF);
}

READ_HANDLER ( gb_r_timer_cnt )
{
	return (gb_timer_count >> gb_timer_shift);
}

int gb_load_rom (int id)
{
	UINT8 *ROM = memory_region(REGION_CPU1);
	static char *CartTypes[] =
	{
		"ROM ONLY",
		"ROM+MBC1",
		"ROM+MBC1+RAM",
		"ROM+MBC1+RAM+BATTERY",
        "UNKNOWN",
		"ROM+MBC2",
		"ROM+MBC2+BATTERY",
        "UNKNOWN",
		"ROM+RAM",
		"ROM+RAM+BATTERY",
        "UNKNOWN",
		"ROM+MMM01",
		"ROM+MMM01+SRAM",
		"ROM+MMM01+SRAM+BATTERY",
        "UNKNOWN",
		"ROM+MBC3+TIMER+BATTERY",
		"ROM+MBC3+TIMER+RAM+BATTERY",
		"ROM+MBC3",
		"ROM+MBC3+RAM",
		"ROM+MBC3+RAM+BATTERY",
        "UNKNOWN",
        "UNKNOWN",
        "UNKNOWN",
        "UNKNOWN",
        "UNKNOWN",
		"ROM+MBC5",
		"ROM+MBC5+RAM",
		"ROM+MBC5+RAM+BATTERY",
		"ROM+MBC5+RUMBLE",
		"ROM+MBC5+RUMBLE+SRAM",
		"ROM+MBC5+RUMBLE+SRAM+BATTERY",
		"Pocket Camera",
		"Bandai TAMA5",
		/* Need heaps of unknowns here */
		"Hudson HuC-3",
		"Hudson HuC-1"
	};

  /*** Following are some known manufacturer codes *************************/
	static struct
	{
		UINT16 Code;
		char *Name;
	}
	Companies[] =
	{
		{0x3301, "Nintendo"},
		{0x7901, "Accolade"},
		{0xA400, "Konami"},
		{0x6701, "Ocean"},
		{0x5601, "LJN"},
		{0x9900, "ARC?"},
		{0x0101, "Nintendo"},
		{0x0801, "Capcom"},
		{0x0100, "Nintendo"},
		{0xBB01, "SunSoft"},
		{0xA401, "Konami"},
		{0xAF01, "Namcot?"},
		{0x4901, "Irem"},
		{0x9C01, "Imagineer"},
		{0xA600, "Kawada?"},
		{0xB101, "Nexoft"},
		{0x5101, "Acclaim"},
		{0x6001, "Titus"},
		{0xB601, "HAL"},
		{0x3300, "Nintendo"},
		{0x0B00, "Coconuts?"},
		{0x5401, "Gametek"},
		{0x7F01, "Kemco?"},
		{0xC001, "Taito"},
		{0xEB01, "Atlus"},
		{0xE800, "Asmik?"},
		{0xDA00, "Tomy?"},
		{0xB100, "ASCII?"},
		{0xEB00, "Atlus"},
		{0xC000, "Taito"},
		{0x9C00, "Imagineer"},
		{0xC201, "Kemco?"},
		{0xD101, "Sofel?"},
		{0x6101, "Virgin"},
		{0xBB00, "SunSoft"},
		{0xCE01, "FCI?"},
		{0xB400, "Enix?"},
		{0xBD01, "Imagesoft"},
		{0x0A01, "Jaleco?"},
		{0xDF00, "Altron?"},
		{0xA700, "Takara?"},
		{0xEE00, "IGS?"},
		{0x8300, "Lozc?"},
		{0x5001, "Absolute?"},
		{0xDD00, "NCS?"},
		{0xE500, "Epoch?"},
		{0xCB00, "VAP?"},
		{0x8C00, "Vic Tokai"},
		{0xC200, "Kemco?"},
		{0xBF00, "Sammy?"},
		{0x1800, "Hudson Soft"},
		{0xCA01, "Palcom/Ultra"},
		{0xCA00, "Palcom/Ultra"},
		{0xC500, "Data East?"},
		{0xA900, "Technos Japan?"},
		{0xD900, "Banpresto?"},
		{0x7201, "Broderbund?"},
		{0x7A01, "Triffix Entertainment?"},
		{0xE100, "Towachiki?"},
		{0x9300, "Tsuburava?"},
		{0xC600, "Tonkin House?"},
		{0xCE00, "Pony Canyon"},
		{0x7001, "Infogrames?"},
		{0x8B01, "Bullet-Proof Software?"},
		{0x5501, "Park Place?"},
		{0xEA00, "King Records?"},
		{0x5D01, "Tradewest?"},
		{0x6F01, "ElectroBrain?"},
		{0xAA01, "Broderbund?"},
		{0xC301, "SquareSoft"},
		{0x5201, "Activision?"},
		{0x5A01, "Bitmap Brothers/Mindscape"},
		{0x5301, "American Sammy"},
		{0x4701, "Spectrum Holobyte"},
		{0x1801, "Hudson Soft"},
		{0x0000, NULL}
	};

	int Checksum, I, J;
	char *P, S[50];
	void *F;
	int rambanks[5] = {0, 1, 1, 4, 16};

	for (I = 0; I < 256; I++)
		RAMMap[I] = ROMMap[I] = NULL;

	if(device_filename(IO_CARTSLOT,id)==NULL)
	{
		printf("Cartridge name not specified!\n");
		return INIT_FAIL;
	}
	if( new_memory_region(REGION_CPU1, 0x10000,0) )
	{
		logerror("Memory allocation failed reading roms!\n");
        return 1;
    }

	ROM = gb_ram = memory_region(REGION_CPU1);
	memset (ROM, 0, 0x10000);

	/* FIXME should check first if a file is given, should give a more clear error */
	if (!(F = image_fopen (IO_CARTSLOT, id, OSD_FILETYPE_IMAGE, OSD_FOPEN_READ)))
	{
		logerror("image_fopen failed in gb_load_rom.\n");
		return 1;
	}

/* some tricks since we don't have a lseek, the filesize can't
   be determined easily. So we just keep reading into the same buffer untill
   the reads fails and then check if we have 512 bytes too much, so its a file
   with header or not */

    for (J = 0x4000; J == 0x4000;)
		J = osd_fread (F, gb_ram, 0x4000);

	osd_fclose (F);

	/* FIXME should check first if a file is given, should give a more clear error */
	if (!(F = image_fopen (IO_CARTSLOT, id, OSD_FILETYPE_IMAGE, OSD_FOPEN_READ)))
	{
		logerror("image_fopen failed in gb_load_rom.\n");
        return 1;
	}

	if (J == 512)
	{
		logerror("ROM-header found skipping\n");
		osd_fread (F, gb_ram, 512);
	}

	if (osd_fread (F, gb_ram, 0x4000) != 0x4000)
	{
		logerror("Error while reading from file: %s\n", device_filename(IO_CARTSLOT,id));
		osd_fclose (F);
		return 1;
	}

	ROMMap[0] = gb_ram;
	ROMBanks = 2 << gb_ram[0x0148];
	RAMBanks = rambanks[gb_ram[0x0149] & 3];
	Checksum = ((UINT16) gb_ram[0x014E] << 8) + gb_ram[0x014F];
	/* File in our cart details */
	switch( gb_ram[0x0147] )
	{
		case 0x00:
			MBCType = 0;
			CartType = 0;
			break;
		case 0x01:
			MBCType = MBC1;
			CartType = 0;
			break;
		case 0x02:
			MBCType = MBC1;
			CartType = RAM;
			break;
		case 0x03:
			MBCType = MBC1;
			CartType = RAM | BATTERY;
			break;
		case 0x05:
			MBCType = MBC2;
			CartType = 0;
			break;
		case 0x06:
			MBCType = MBC2;
			CartType = BATTERY;
			break;
		case 0x08:
			MBCType = 0;
			CartType = RAM;
		case 0x09:
			MBCType = 0;
			CartType = RAM | BATTERY;
			break;
		case 0x0F:
			MBCType = MBC3;
			CartType = TIMER | BATTERY;
			break;
		case 0x10:
			MBCType = MBC3;
			CartType = TIMER | RAM | BATTERY;
			break;
		case 0x11:
			MBCType = MBC3;
			CartType = 0;
			break;
		case 0x12:
			MBCType = MBC3;
			CartType = RAM;
			break;
		case 0x13:
			MBCType = MBC3;
			CartType = RAM | BATTERY;
			break;
		case 0x19:
			MBCType = MBC5;
			CartType = 0;
			break;
		case 0x1A:
			MBCType = MBC5;
			CartType = RAM;
			break;
		case 0x1B:
			MBCType = MBC5;
			CartType = RAM | BATTERY;
			break;
		case 0x1C:
			MBCType = MBC5;
			CartType = RUMBLE;
			break;
		case 0x1D:
			MBCType = MBC5;
			CartType = RUMBLE | SRAM;
			break;
		case 0x1E:
			MBCType = MBC5;
			CartType = RUMBLE | SRAM | BATTERY;
			break;
		case 0xFE:
			MBCType = HUC3;
			CartType = 0;
			break;
		case 0xFF:
			MBCType = HUC1;
			CartType = 0;
			break;
		default:
			MBCType = 0;
			CartType = UNKNOWN;
	}

	if ( CartType & UNKNOWN )
	{
		logerror("Error loading cartridge: Unknown ROM type\n");
		osd_fclose (F);
		return 1;
	}

	if (Verbose)
	{
		strncpy (S, (char *)&gb_ram[0x0134], 16);
		S[16] = '\0';
		logerror("OK\n  Name: %s\n", S);
		logerror("  Type: %s [%Xh]\n", CartTypes[gb_ram[0x0147]], gb_ram[0x0147] );
		logerror("  Color GB: %s [%Xh]\n", (gb_ram[0x0143] == 0x80 || gb_ram[0x0143] == 0xc0) ? "Yes" : "No", gb_ram[0x0143] );
		logerror("  Super GB: %s [%Xh]\n", (gb_ram[0x0146] == 0x03) ? "Yes" : "No", gb_ram[0x0146] );
		logerror("  ROM Size: %d 16kB Banks [%X]\n", ROMBanks, gb_ram[0x0148]);
		J = (gb_ram[0x0149] & 0x03) * 2;
		J = J ? (1 << (J - 1)) : 0;
		logerror("  RAM Size: %d kB [%X]\n", J, gb_ram[0x0149]);
		logerror("  License code %X%Xh\n", gb_ram[0x0145], gb_ram[0x0144] );
		J = ((UINT16) gb_ram[0x014B] << 8) + gb_ram[0x014A];
		for (I = 0, P = NULL; !P && Companies[I].Name; I++)
			if (J == Companies[I].Code)
				P = Companies[I].Name;
		logerror("  Manufacturer ID: %Xh", J);
		logerror(" [%s]\n", P ? P : "?");

		logerror("  Version Number: %Xh\n", gb_ram[0x014C]);
		logerror("  Complement Check: %Xh\n", gb_ram[0x014D]);
		logerror("  Checksum: %Xh\n", Checksum);
		J = ((UINT16) gb_ram[0x0103] << 8) + gb_ram[0x0102];
		logerror("  Start Address: %Xh\n", J);
	}

	Checksum += gb_ram[0x014E] + gb_ram[0x014F];
	for (I = 0; I < 0x4000; I++)
		Checksum -= gb_ram[I];

	if (Verbose)
		logerror("Loading %dx16kB ROM banks:.", ROMBanks);
	for (I = 1; I < ROMBanks; I++)
	{
		if ((ROMMap[I] = malloc (0x4000)))
		{
			if (osd_fread (F, ROMMap[I], 0x4000) == 0x4000)
			{
				for (J = 0; J < 0x4000; J++)
					Checksum -= ROMMap[I][J];
				if (Verbose)
					putchar ('.');
			}
			else
			{
				logerror("Error while reading from file: %s\n", device_filename(IO_CARTSLOT,id));
				break;
			}
		}
		else
		{
			logerror("Error allocating memory\n");
			break;
		}
	}

	osd_fclose (F);
	if (I < ROMBanks)
		return 1;

	if (CheckCRC && (Checksum & 0xFFFF))
	{
		logerror("Error loading cartridge: Checksum is wrong");
		return 1;
	}

	/* MBC2 has 512 * 4bits (8kb) internal RAM */
	if( MBCType == MBC2 )
		RAMBanks = 1;

	if (RAMBanks && MBCType)
	{
		for (I = 0; I < RAMBanks; I++)
		{
			if ((RAMMap[I] = malloc (0x2000)))
				memset (RAMMap[I], 0, 0x2000);
			else
			{
				logerror("Error alocating memory\n");
				return 1;
			}
		}
	}

	/* Load the saved RAM if this cart has a battery */
	if( CartType & BATTERY )
	{
		void * f;
		char filename[19];
		char cartname[16];

		/* Build the filename */
		strncpy(cartname, (char *)&gb_ram[0x0134], 16);
		cartname[16] = '\0';
		sprintf( filename, "gb_%s", cartname );

		f = osd_fopen(filename, 0, OSD_FILETYPE_NVRAM, 0);
		if( f )
		{
			for( I = 0; I < RAMBanks; I++ )
			{
				osd_fread( f, RAMMap[I], 0x2000 );
			}
			osd_fclose(f);
		}
	}

	if (ROMBanks < 3)
		ROMMask = 0;
	else
	{
		for (I = 1; I < ROMBanks; I <<= 1) ;
		ROMMask = I - 1;
	}
	if (!RAMMap[0])
		RAMMask = 0;
	else
	{
		for (I = 1; I < RAMBanks; I <<= 1) ;
		RAMMask = I - 1;
	}

	return 0;
}

int gb_scanline_interrupt (void)
{
	/* test ! */
	static UINT8 count = 0;

	count = (count + 1) % 3;
	switch (count)
	{
	case 0:
		/* continue */
		break;
	case 1:
		gb_scanline_interrupt_set_mode2 (0);
		return ignore_interrupt();
	case 2:
		gb_scanline_interrupt_set_mode3 (0);
		return ignore_interrupt();
	}

	/* first let's draw the current scanline */
	if (CURLINE < 144)
		gb_refresh_scanline ();

	/* the rest only makes sense if the display is enabled */
	if (LCDCONT & 0x80)
	{
		if (CURLINE == CMPLINE)
		{
			LCDSTAT |= 0x04;
			/* generate lcd interrupt if requested */
			if( LCDSTAT & 0x40 )
				cpu_set_irq_line(0, LCD_INT, HOLD_LINE);
		}
		else
			LCDSTAT &= 0xFB;

		CURLINE = (CURLINE + 1) % 154;

		/*gb_ram[0xFF44] = CURLINE; */

		if (CURLINE < 144)
		{
			/* first  lcdstate change after aprox 49 uS */
			timer_set (48.6 / 1000000.0, 0, gb_scanline_interrupt_set_mode2);

			/* second lcdstate change after aprox 69 uS */
			timer_set (67.6 / 1000000.0, 0, gb_scanline_interrupt_set_mode3);

			/* modify lcdstate */
			LCDSTAT = LCDSTAT & 0xFC;

			/* generate lcd interrupt if requested */
			if( LCDSTAT & 0x08 )
			{
				/*logerror("generating lcd interrupt\n");*/
				cpu_set_irq_line(0, LCD_INT, HOLD_LINE);
			}
		}
		else
		{
			/* generate VBlank interrupt */
			if (CURLINE == 144)
			{
				/* cause VBlank interrupt */
				cpu_set_irq_line(0, VBL_INT, HOLD_LINE);
				/* Set VBlank lcdstate */
				LCDSTAT = (LCDSTAT & 0xFC) | 0x01;
				/* generate lcd interrupt if requested */
				if( LCDSTAT & 0x10 )
					cpu_set_irq_line(0, LCD_INT, HOLD_LINE);
			}
		}

		/* Generate serial IO interrupt */
		if (SIOCount)
		{
			SIODATA = (SIODATA << 1) | 0x01;
			if (!--SIOCount)
			{
				SIOCONT &= 0x7F;
				cpu_set_irq_line(0, SIO_INT, HOLD_LINE);
			}
		}
	}

	/* Return No interrupt, we cause them ourselves since multiple int's can
	 * occur at the same time */
	return ignore_interrupt();
}

void gb_scanline_interrupt_set_mode2 (int param)
{
	/* modify lcdstate */
	LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
	/* generate lcd interrupt if requested */
	if (LCDSTAT & 0x20)
		cpu_set_irq_line(0, LCD_INT, HOLD_LINE);
}

void gb_scanline_interrupt_set_mode3 (int param)
{
	/* modify lcdstate */
	LCDSTAT = (LCDSTAT & 0xFC) | 0x03;
}
