/******************************************************************************

        nc.c

        NC100/NC150/NC200 Notepad computer

        system driver


		Thankyou to:
			Cliff Lawson, Russell Marks and Tim Surtel

        Documentation:

		NC100:
			NC100 I/O Specification by Cliff Lawson,
			NC100EM by Russell Marks
		NC200:
			Dissassembly of the NC200 ROM + e-mail
			exchange with Russell Marks


		NC100:

        Hardware:
            - Z80 CPU, 6mhz
            - memory powered by lithium batterys!
            - 2 channel tone (programmable frequency beep's)
            - LCD screen
            - laptop/portable computer
            - qwerty keyboard
            - serial/parallel connection
            - Amstrad custom ASIC chip
			- tc8521 real time clock
			- intel 8251 compatible uart
			- PCMCIA Memory cards supported, up to 1mb!

		NC200:

        Hardware:
			- Z80 CPU
			- Intel 8251 compatible uart
            - nec765 compatible floppy disc controller
			- mc146818 real time clock?
			- 720k floppy disc drive (compatible with MS-DOS)
			(disc drive can be not ready).
			- PCMCIA Memory cards supported, up to 1mb!

        TODO:
           - find out what the unused key bits are for
		   (checked all unused bits on nc200! - do not seem to have any use!)
           - complete serial (xmodem protocol!)
		   - overlay would be nice!
		   - finish NC200 disc drive emulation (closer!)
		   - add NC150 driver - ROM needed!!! 
			- on/off control
			- check values read from other ports that are not described!
			- what is read from unmapped ports?
			- what is read from uart when it is off?
			- check if uart ints are generated when it is off?
			- are ints cancelled if uart is turned off after int has been caused?

		PCMCIA memory cards are stored as a direct dump of the contents. There is no header
		information. No distinction is made between RAM and ROM cards.

		Memory card sizes are a power of 2 in size (e.g. 1mb, 512k, 256k etc),
		however the code allows any size, but it limits access to power of 2 sizes, with
		minimum access being 16k. If a file which is less than 16k is used, no memory card
		will be present. If a file which is greater than 1mb is used, only 1mb will be accessed.

		Interrupt system of NC100:

		The IRQ mask is used to control the interrupt sources that can interrupt.
		
		The IRQ status can be read to determine which devices are interrupting.
		Some devices, e.g. serial, cannot be cleared by writing to the irq status
		register. These can only be cleared by performing an operation on the 
		device (e.g. reading a data register).

		Self Test:

		- requires memory save and real time clock save to be working!
		(i.e. for MESS nc100 driver, nc100.nv can be created)
		- turn off nc (use NMI button)
		- reset+FUNCTION+SYMBOL must be pressed together.

		Note: NC200 Self test does not test disc hardware :(



		Kevin Thacker [MESS driver]

 ******************************************************************************/
#include "driver.h"
#include "includes/nc.h"
/* for NC100 real time clock */
#include "includes/tc8521.h"
/* for NC100 uart */
#include "includes/msm8251.h"
/* for NC200 real time clock */
#include "includes/mc146818.h"
/* for NC200 disk drive interface */
#include "includes/nec765.h"
/* for NC200 disk image */
#include "includes/pc_flopp.h"
/* for serial data transfers */
#include "includes/serial.h"
/* uncomment for verbose debugging information */
//#define VERBOSE

//#define NC200_DEBUG

#include "includes/centroni.h"
#include "printer.h"

static void nc_printer_update(int);

static unsigned long nc_memory_size;
UINT8 nc_type;

static char nc_memory_config[4];
unsigned long nc_display_memory_start;
static void *nc_keyboard_timer = NULL;
static void *dummy_timer = NULL;

static int nc_membank_rom_mask;
static int nc_membank_internal_ram_mask;
int nc_membank_card_ram_mask;

/*
	Port 0x00:
	==========

	Display memory start:

	NC100 & NC200:
			bit 7           A15
			bit 6           A14
			bit 5           A13
			bit 4           A12
			bits 3-0        Not Used

	Port 0x010-0x013:
	=================

	Memory management control:

	NC100 & NC200:

        10              controls 0000-3FFF
        11              controls 4000-7FFF
        12              controls 8000-BFFF
        13              controls C000-FFFF

	Port 0x030:
	===========

	NC100:
			bit 7     select card register 1=common, 0=attribute
			bit 6     parallel interface Strobe signal
			bit 5     Not Used
			bit 4     uPD4711 line driver, 1=off, 0=on
			bit 3     UART clock and reset, 1=off, 0=on

			bits 2-0  set the baud rate as follows

					000 = 150
					001 = 300
					010 = 600
					011 = 1200
					100 = 2400
					101 = 4800
					110 = 9600
					111 = 19200
	NC200:
			bit 7     select card register 1=common, 0=attribute
			bit 6     parallel interface Strobe signal

			bit 5     used in disc interface

			bit 4     uPD4711 line driver, 1=off, 0=on
			bit 3     UART clock and reset, 1=off, 0=on

			bits 2-0  set the baud rate as follows

					000 = 150
					001 = 300
					010 = 600
					011 = 1200
					100 = 2400
					101 = 4800
					110 = 9600
					111 = 19200
				
	
	Port 0x0a0:
	===========

	NC100:
			bit 7: memory card present 0 = yes, 1 = no
			bit 6: memory card write protected 1 = yes, 0 = no
			bit 5: input voltage = 1, if >= to 4 volts
			bit 4: mem card battery: 0 = battery low
			bit 3: alkaline batteries. 0 if >=3.2 volts
			bit 2: lithium battery 0 if >= 2.7 volts
			bit 1: parallel interface busy (0 if busy)
			bit 0: parallel interface ack (1 if ack)

	NC200:

			bit 7: memory card present 0 = yes, 1 = no
			bit 6: memory card write protected 1 = yes, 0 = no
			bit 5: lithium battery 0 if >= 2.7 volts
			bit 4: input voltage = 1, if >= to 4 volts
			bit 3: ??
			bit 2: alkaline batteries. 0 if >=3.2 volts
			bit 1: ??
			bit 0: battery power: if 1: batteries are too low for disk usage, if 0: batteries ok for disc usage


	Port 0x060 (IRQ MASK), Port 0x090 (IRQ STATUS):
	===============================================

	NC100:
			bit 7: not used
			bit 6: not used
			bit 5: not used
			bit 4: not used
			Bit 3: Key scan interrupt (10ms)
			Bit 2: ACK from parallel interface
			Bit 1: Tx Ready
			Bit 0: Rx Ready

	NC200:
			bit 7: ???
			bit 6: RTC alarm?
			bit 5: FDC interrupt
			bit 4: FDD Index interrupt????
			Bit 3: Key scan interrupt (10ms)
			Bit 2: serial interrupt (tx ready/rx ready combined)
			Bit 1: not used
			Bit 0: ACK from parallel interface
*/


static UINT8 nc_poweroff_control;

/* this is not a real register, it is used to record card status */
/* bit 1: card is present */
/* bit 0: card is write protected */
static int nc_card_status = 0;

/* set pcmcia card present state */
void nc_set_card_present_state(int state)
{
	if (state)
	{
		nc_card_status |= (1<<1);
	}
	else
	{
		nc_card_status &= ~(1<<1);
	}
}

/* internal ram */
unsigned char    *nc_memory = NULL;
/* card ram */
unsigned char    *nc_card_ram = NULL;

static unsigned char nc_uart_control;

static int nc_irq_mask;
static int nc_irq_status;


/* latched interrupts are interrupts that cannot be cleared by writing to the irq
mask. latched interrupts can only be cleared by accessing the interrupting
device e.g. serial chip, fdc */
static int nc_irq_latch;
/* this is a mask of irqs that are latched, and it is different for nc100 and
nc200 */
static int nc_irq_latch_mask;
static int nc_sound_channel_periods[2];

static void nc_update_interrupts(void)
{
		nc_irq_status &= ~nc_irq_latch_mask;
		nc_irq_status |= nc_irq_latch;

        /* any ints set and they are not masked? */
        if (
                (((nc_irq_status & nc_irq_mask) & 0x3f)!=0)
                )
        {
				
				logerror("int set %02x\n",nc_irq_status & nc_irq_mask);
                /* set int */
                cpu_set_irq_line(0,0, HOLD_LINE);
        }
        else
        {
                /* clear int */
                cpu_set_irq_line(0,0, CLEAR_LINE);
        }
}

static void nc_keyboard_timer_callback(int dummy)
{
#ifdef VERBOSE
		logerror("keyboard int\n");
#endif

        /* set int */
        nc_irq_status |= (1<<3);

        /* update ints */
        nc_update_interrupts();

        /* don't trigger again, but don't free it */
        timer_reset(nc_keyboard_timer, TIME_NEVER);
}


static mem_read_handler nc_bankhandler_r[]={
MRA_BANK1, MRA_BANK2, MRA_BANK3, MRA_BANK4};

static mem_write_handler nc_bankhandler_w[]={
MWA_BANK5, MWA_BANK6, MWA_BANK7, MWA_BANK8};

static void nc_refresh_memory_bank_config(int bank)
{
        int mem_type;
        int mem_bank;

        mem_type = (nc_memory_config[bank]>>6) & 0x03;
        mem_bank = nc_memory_config[bank] & 0x03f;

        memory_set_bankhandler_r(bank+1, 0, nc_bankhandler_r[bank]);

        switch (mem_type)
        {
                /* ROM */
                case 0:
                {
                   unsigned char *addr;

                   mem_bank = mem_bank & nc_membank_rom_mask;

                   addr = (memory_region(REGION_CPU1)+0x010000) + (mem_bank<<14);

                   cpu_setbank(bank+1, addr);

                   memory_set_bankhandler_w(bank+5, 0, MWA_NOP);
#ifdef VERBOSE
                   logerror("BANK %d: ROM %d\n",bank,mem_bank);
#endif
                }
                break;

                /* internal RAM */
                case 1:
                {
                   unsigned char *addr;

                   mem_bank = mem_bank & nc_membank_internal_ram_mask;

                   addr = nc_memory + (mem_bank<<14);

                   cpu_setbank(bank+1, addr);
                   cpu_setbank(bank+5, addr);

                   memory_set_bankhandler_w(bank+5, 0, nc_bankhandler_w[bank]);
#ifdef VERBOSE
                   logerror("BANK %d: RAM\n",bank);
#endif
                }
                break;

                /* card RAM */
                case 2:
                {
					/* card connected? */
					if (((nc_card_status & (1<<1))!=0) && (nc_card_ram!=NULL))
					{
						unsigned char *addr;

						mem_bank = mem_bank & nc_membank_card_ram_mask;
						addr = nc_card_ram + (mem_bank<<14);

						cpu_setbank(bank+1, addr);

						/* write enabled? */
						if (readinputport(10) & 0x02)
						{
							/* yes */
							cpu_setbank(bank+5, addr);

							memory_set_bankhandler_w(bank+5, 0, nc_bankhandler_w[bank]);
						}
						else
						{
							/* no */
							memory_set_bankhandler_w(bank+5, 0, MWA_NOP);
						}

#ifdef VERBOSE
						logerror("BANK %d: CARD-RAM\n",bank);
#endif
					}
					else
					{
						/* if no card connected, then writes fail */
						memory_set_bankhandler_r(bank+1, 0, MRA_NOP);
						memory_set_bankhandler_w(bank+5, 0, MWA_NOP);
					}
                }
                break;

                /* ?? */
                default:
                case 3:
                {
#ifdef VERBOSE
						logerror("Invalid memory selection\n");
#endif
						memory_set_bankhandler_r(bank+1, 0, MRA_NOP);
						memory_set_bankhandler_w(bank+5, 0, MWA_NOP);
                }
                break;


        }


}

static void nc_refresh_memory_config(void)
{
        nc_refresh_memory_bank_config(0);
        nc_refresh_memory_bank_config(1);
        nc_refresh_memory_bank_config(2);
        nc_refresh_memory_bank_config(3);
}


static void *file;

/* restore a block of memory from the nvram file */
static void nc_common_restore_memory_from_stream(void)
{
	if (!file)
		return;

    if (nc_memory!=NULL)
    {
		unsigned long stored_size;
		unsigned long restore_size;
		
#ifdef VERBOSE
		logerror("restoring nc memory\n");
#endif
		/* get size of memory data stored */
		osd_fread(file, &stored_size, sizeof(unsigned long));

		if (stored_size>nc_memory_size)
		{
			restore_size = nc_memory_size;
		}
		else
		{
			restore_size = stored_size;
		}
		/* read as much as will fit into memory */
		osd_fread(file, nc_memory, restore_size);
		/* seek over remaining data */    
		osd_fseek(file, SEEK_CUR,stored_size - restore_size);
	}
}

/* store a block of memory to the nvram file */
static void nc_common_store_memory_to_stream(void)
{
	if (!file)
		return;

    if (nc_memory!=NULL)
    {
#ifdef VERBOSE
		logerror("storing nc memory\n");
#endif
		/* write size of memory data */
		osd_fwrite(file, &nc_memory_size, sizeof(unsigned long));

		/* write data block */
		osd_fwrite(file, nc_memory, nc_memory_size);
    }
}

static void nc_common_open_stream_for_reading(void)
{
	char filename[13];

	sprintf(filename,"%s.nv", Machine->gamedrv->name);

	file = osd_fopen(Machine->gamedrv->name, filename, OSD_FILETYPE_MEMCARD, OSD_FOPEN_READ);
}

static void nc_common_open_stream_for_writing(void)
{
    char filename[13];

    sprintf(filename,"%s.nv", Machine->gamedrv->name);

    file = osd_fopen(Machine->gamedrv->name, filename, OSD_FILETYPE_MEMCARD, OSD_FOPEN_WRITE);
}


static void	nc_common_close_stream(void)
{
	if (file)
	{
		osd_fclose(file);
	}
}



static int	previous_inputport_10_state;

static void dummy_timer_callback(int dummy)
{
	int inputport_10_state;
	int changed_bits;

    inputport_10_state = readinputport(10);

	changed_bits = inputport_10_state^previous_inputport_10_state;

	/* on/off button changed state? */
	if (changed_bits & 0x01)
	{
        if (inputport_10_state & 0x01)
        {
#ifdef VERBOSE
            logerror("nmi triggered\n");
#endif
            cpu_set_nmi_line(0, PULSE_LINE);
        }
	}

	/* memory card write enable/disable state changed? */
	if (changed_bits & 0x02)
	{
		/* yes refresh memory config */
		nc_refresh_memory_config();
	}

	previous_inputport_10_state = inputport_10_state;
}


void nc_common_init_machine(void)
{
	/* setup reset state */
	nc_display_memory_start = 0;

	/* setup reset state */
    nc_memory_config[0] = 0;
    nc_memory_config[1] = 0;
    nc_memory_config[2] = 0;
    nc_memory_config[3] = 0;

	previous_inputport_10_state = readinputport(10);

    /* setup reset state ints are masked */
    nc_irq_mask = 0;
    /* setup reset state no ints wanting servicing */
    nc_irq_status = 0;
    /* at reset set to 0x0ffff */
    
	nc_irq_latch = 0;
	nc_irq_latch_mask = 0;

	/* setup reset state */
	nc_sound_channel_periods[0] = (nc_sound_channel_periods[1] = 0x0ffff);

    /* at reset set to 1 */
    nc_poweroff_control = 1;

    nc_refresh_memory_config();
	nc_update_interrupts();

    nc_keyboard_timer = timer_set(TIME_IN_MSEC(10), 0, nc_keyboard_timer_callback);

    dummy_timer = timer_pulse(TIME_IN_HZ(50), 0, dummy_timer_callback);

	/* at reset set to 0x0ff */
	nc_uart_control = 0x0ff;
}

void nc_common_shutdown_machine(void)
{
#ifdef VERBOSE
	logerror("shutdown machine\n");
#endif
	msm8251_stop();

    if (nc_memory!=NULL)
    {
        free(nc_memory);
        nc_memory = NULL;
    }

    if (nc_keyboard_timer!=NULL)
    {
            timer_remove(nc_keyboard_timer);
            nc_keyboard_timer = NULL;
    }

    if (dummy_timer!=NULL)
    {
            timer_remove(dummy_timer);
            dummy_timer = NULL;
    }
}


MEMORY_READ_START( readmem_nc )
    {0x00000, 0x03fff, MRA_BANK1},
    {0x04000, 0x07fff, MRA_BANK2},
    {0x08000, 0x0bfff, MRA_BANK3},
    {0x0c000, 0x0ffff, MRA_BANK4},
MEMORY_END


MEMORY_WRITE_START( writemem_nc )
    {0x00000, 0x03fff, MWA_BANK5},
    {0x04000, 0x07fff, MWA_BANK6},
    {0x08000, 0x0bfff, MWA_BANK7},
    {0x0c000, 0x0ffff, MWA_BANK8},
MEMORY_END


READ_HANDLER(nc_memory_management_r)
{
        return nc_memory_config[offset];
}

WRITE_HANDLER(nc_memory_management_w)
{
#ifdef VERBOSE
	logerror("Memory management W: %02x %02x\n",offset,data);
#endif
        nc_memory_config[offset] = data;

        nc_refresh_memory_config();
}

WRITE_HANDLER(nc_irq_mask_w)
{
#ifdef VERBOSE
	logerror("irq mask w: %02x\n", data);
#endif
#ifdef NC200_DEBUG
	logerror("irq mask nc200 w: %02x\n",data & ((1<<4) | (1<<5) | (1<<6) | (1<<7)));
#endif

/* writing mask clears ints that are to be masked? */

        nc_irq_mask = data;

        nc_update_interrupts();
}

WRITE_HANDLER(nc_irq_status_w)
{
#ifdef VERBOSE
	logerror("irq status w: %02x\n", data);
#endif
        data = data^0x0ff;

/* writing to status will clear int, will this re-start the key-scan? */
#if 0
        if (
                /* clearing keyboard int? */
                ((data & (1<<3))!=0) &&
                /* keyboard int request? */
                ((nc_irq_status & (1<<3))!=0)
           )
        {
           if (nc_keyboard_timer!=NULL)
           {
                timer_remove(nc_keyboard_timer);
                nc_keyboard_timer = NULL;
           }

           /* set timer to occur again */
           nc_keyboard_timer = timer_set(TIME_IN_MSEC(10), 0, nc_keyboard_timer_callback);

        }
#endif
        nc_irq_status &=~data;

        nc_update_interrupts();
}

READ_HANDLER(nc_irq_status_r)
{
        return ~nc_irq_status;
}

WRITE_HANDLER(nc_display_memory_start_w)
{
        /* bit 7: A15 */
        /* bit 6: A14 */
        /* bit 5: A13 */
        /* bit 4: A12 */
        /* bit 3-0: not used */
        nc_display_memory_start = (data & 0x0f0)<<(12-4);

#ifdef VERBOSE
        logerror("disp memory w: %04x\n", nc_display_memory_start);
#endif

}


READ_HANDLER(nc_key_data_in_r)
{
        if (offset==9)
        {
			/* reading 0x0b9 will clear int and re-start scan procedure! */
           nc_irq_status &= ~(1<<3);

           if (nc_keyboard_timer!=NULL)
           {
                timer_remove(nc_keyboard_timer);
                nc_keyboard_timer = NULL;
           }

           /* set timer to occur again */
           nc_keyboard_timer = timer_set(TIME_IN_MSEC(10), 0, nc_keyboard_timer_callback);

           nc_update_interrupts();
        }

        return readinputport(offset);

}


static void nc_sound_update(int channel)
{
        int on;
        int frequency;
        int period;

        period = nc_sound_channel_periods[channel];

        /* if top bit is 0, sound is on */
        on = ((period & (1<<15))==0);

        /* calculate frequency from period */
        frequency = (int)(1000000.0f/((float)((period & 0x07fff)<<1) * 1.6276f));

        /* set state */
        beep_set_state(channel, on);
        /* set frequency */
        beep_set_frequency(channel, frequency);
}

WRITE_HANDLER(nc_sound_w)
{
#ifdef VERBOSE
	logerror("sound w: %04x %02x\n", offset, data);
#endif
	switch (offset)
	{
		case 0x0:
		{
		   /* update period value */
		   nc_sound_channel_periods[0]  =
				(nc_sound_channel_periods[0] & 0x0ff00) | (data & 0x0ff);

		   nc_sound_update(0);
		}
		break;

		case 0x01:
		{
		   nc_sound_channel_periods[0] =
				(nc_sound_channel_periods[0] & 0x0ff) | ((data & 0x0ff)<<8);

		   nc_sound_update(0);
		}
		break;

		case 0x02:
		{
		   /* update period value */
		   nc_sound_channel_periods[1]  =
				(nc_sound_channel_periods[1] & 0x0ff00) | (data & 0x0ff);

		   nc_sound_update(1);
		}
		break;

		case 0x03:
		{
		   nc_sound_channel_periods[1] =
				(nc_sound_channel_periods[1] & 0x0ff) | ((data & 0x0ff)<<8);

		   nc_sound_update(1);
		}
		break;

		default:
		 break;
	}
}

static unsigned long baud_rate_table[]=
{
	150,
    300,
    600,
    1200,
    2400,
    4800,
    9600,
    19200
};

WRITE_HANDLER(nc_uart_control_w)
{
	/* update printer state */
	nc_printer_update(data);

	/* on/off changed state? */
	if (((nc_uart_control ^ data) & (1<<3))!=0)
	{
		/* changed uart from off to on */
		if ((data & (1<<3))==0)
		{
			msm8251_reset();
		}
	}

	nc_uart_control = data;

	msm8251_set_baud_rate(baud_rate_table[(data & 0x07)]);

}

/* NC100 printer emulation */
/* port 0x040 (write only) = 8-bit printer data */
/* port 0x030 bit 6 = printer strobe */

/* same for nc100 and nc200 */
static WRITE_HANDLER(nc_printer_data_w)
{
#ifdef VERBOSE
	logerror("printer write %02x\n",data);
#endif
	centronics_write_data(0,data);
}

/* same for nc100 and nc200 */
static void	nc_printer_update(int port0x030)
{
	int handshake = 0;

	if (port0x030 & (1<<6))
	{
		handshake = CENTRONICS_STROBE;
	}
	/* assumption: select is tied low */
	centronics_write_handshake(0, CENTRONICS_SELECT | CENTRONICS_NO_RESET, CENTRONICS_SELECT| CENTRONICS_NO_RESET);
	centronics_write_handshake(0, handshake, CENTRONICS_STROBE);
}



/********************************************************************************************************/
/* NC100 hardware */

static int previous_alarm_state;


WRITE_HANDLER(nc100_uart_control_w)
{
	nc_uart_control_w(offset,data);

//	/* is this correct?? */
//	if (data & (1<<3))
//	{	
//		/* clear latched irq's */
//		nc_irq_latch &= ~3;
//		nc_update_interrupts();
//	}
}


void	nc100_tc8521_alarm_callback(int state)
{
	/* I'm assuming that the nmi is edge triggered */
	/* a interrupt from the fdc will cause a change in line state, and
	the nmi will be triggered, but when the state changes because the int
	is cleared this will not cause another nmi */
	/* I'll emulate it like this to be sure */

	if (state!=previous_alarm_state)
	{
		if (state)
		{
			/* I'll pulse it because if I used hold-line I'm not sure
			it would clear - to be checked */
			cpu_set_nmi_line(0, PULSE_LINE);
		}
	}

	previous_alarm_state = state;
}

static void nc100_txrdy_callback(int state)
{
	nc_irq_latch &= ~(1<<1);

	/* uart on? */
	if ((nc_uart_control & (1<<3))==0)
	{
		if (state)
		{
//	#ifdef VERBOSE
			logerror("tx ready\n");
//	#endif
			nc_irq_latch |= (1<<1);
		}
	}

	nc_update_interrupts();
}

static void nc100_rxrdy_callback(int state)
{
	nc_irq_latch &= ~(1<<0);

	if ((nc_uart_control & (1<<3))==0)
	{
		if (state)
		{
//#ifdef VERBOSE
			logerror("rx ready\n");
//#endif	
			nc_irq_latch |= (1<<0);
		}
	}

	nc_update_interrupts();
}


static struct tc8521_interface nc100_tc8521_interface=
{
	nc100_tc8521_alarm_callback,
};

static struct msm8251_interface nc100_uart_interface=
{
	nc100_txrdy_callback,
	NULL,
	nc100_rxrdy_callback
};

static void nc100_printer_handshake_in(int number, int data, int mask)
{
	nc_irq_status &= ~(1<<2);

	if (mask & CENTRONICS_ACKNOWLEDGE)
	{
		if (data & CENTRONICS_ACKNOWLEDGE)
		{
			nc_irq_status|=(1<<2);
		}
	}
	/* trigger an int if the irq is set */
	nc_update_interrupts();
}

static CENTRONICS_CONFIG nc100_cent_config[1]={
	{
		PRINTER_CENTRONICS,
		nc100_printer_handshake_in
	},
};



void nc100_init_machine(void)
{
    nc_type = NC_TYPE_1xx;

    nc_memory_size = 64*1024;

    /* 256k of rom */
    nc_membank_rom_mask = 0x0f;

    nc_memory = (unsigned char *)malloc(nc_memory_size);
    nc_membank_internal_ram_mask = 3;

    nc_membank_card_ram_mask = 0x03f;

    nc_common_init_machine();

	tc8521_init(&nc100_tc8521_interface);

	msm8251_init(&nc100_uart_interface);

	centronics_config(0, nc100_cent_config);
	/* assumption: select is tied low */
	centronics_write_handshake(0, CENTRONICS_SELECT | CENTRONICS_NO_RESET, CENTRONICS_SELECT| CENTRONICS_NO_RESET);

	nc_common_open_stream_for_reading();
	tc8521_load_stream(file);

	nc_common_restore_memory_from_stream();

	nc_common_close_stream();

	/* serial */
	nc_irq_latch_mask = (1<<0) | (1<<1);

}

void	nc100_shutdown_machine(void)
{
	nc_common_open_stream_for_writing();
	tc8521_save_stream(file);
	nc_common_store_memory_to_stream();
	nc_common_close_stream();

	nc_common_shutdown_machine();
    tc8521_stop();
}


WRITE_HANDLER(nc100_poweroff_control_w)
{
        /* bits 7-1: not used */
        /* bit 0: 1 = no effect, 0 = power off */
        nc_poweroff_control = data;
#ifdef VERBOSE
		logerror("nc poweroff control: %02x\n",data);
#endif
}


/* nc100 version of card/battery status */
READ_HANDLER(nc100_card_battery_status_r)
{
	int nc_card_battery_status = 0x0ff;

	int printer_handshake;

	if (nc_card_status & (1<<1))
	{
		/* card present */
		nc_card_battery_status &=~(1<<7);
	}

	if (readinputport(10) & 0x02)
	{
		/* card write enable */
		nc_card_battery_status &=~(1<<6);
	}


    /* enough power - see bit assignments where
    nc card battery status is defined */
    /* keep card status bits in case card has been inserted and
    the machine is then reset! */
	nc_card_battery_status |= (1<<5);
	nc_card_battery_status &= ~((1<<2) | (1<<3));

	
	/* assumption: select is tied low */
	centronics_write_handshake(0, CENTRONICS_SELECT | CENTRONICS_NO_RESET, CENTRONICS_SELECT| CENTRONICS_NO_RESET);

	printer_handshake = centronics_read_handshake(0);

	nc_card_battery_status |=(1<<1);

	/* if printer is not online, it is busy */
	if ((printer_handshake & CENTRONICS_ONLINE)!=0)
	{
		nc_card_battery_status &=~(1<<1);
	}

	nc_card_battery_status &=~(1<<0);
	if (printer_handshake & CENTRONICS_ACKNOWLEDGE)
	{
		nc_card_battery_status |=(1<<0);
	}

    return nc_card_battery_status;
}

WRITE_HANDLER(nc100_memory_card_wait_state_w)
{
#ifdef VERBOSE
	logerror("nc100 memory card wait state: %02x\n",data);
#endif
}

PORT_READ_START( readport_nc100 )
    {0x010, 0x013, nc_memory_management_r},
    {0x0a0, 0x0a0, nc100_card_battery_status_r},
    {0x0b0, 0x0b9, nc_key_data_in_r},
    {0x090, 0x090, nc_irq_status_r},
	{0x0c0, 0x0c0, msm8251_data_r},
	{0x0c1, 0x0c1, msm8251_status_r},
    {0x0d0, 0x0df, tc8521_r},
PORT_END

PORT_WRITE_START( writeport_nc100 )
    {0x000, 0x000, nc_display_memory_start_w},
    {0x010, 0x013, nc_memory_management_w},
	{0x020, 0x020, nc100_memory_card_wait_state_w},
	{0x030, 0x030, nc100_uart_control_w},
	{0x040, 0x040, nc_printer_data_w},
    {0x060, 0x060, nc_irq_mask_w},
    {0x070, 0x070, nc100_poweroff_control_w},
    {0x090, 0x090, nc_irq_status_w},
	{0x0c0, 0x0c0, msm8251_data_w},
	{0x0c1, 0x0c1, msm8251_control_w},
    {0x0d0, 0x0df, tc8521_w},
    {0x050, 0x053, nc_sound_w},
PORT_END



INPUT_PORTS_START(nc100)
        /* 0 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "LEFT SHIFT", KEYCODE_LSHIFT, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RIGHT SHIFT", KEYCODE_RSHIFT, IP_JOY_NONE)
        PORT_BIT (0x004, 0x00, IPT_UNUSED)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "LEFT/RED", KEYCODE_LEFT, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RETURN", KEYCODE_ENTER, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BIT (0x040, 0x00, IPT_UNUSED)
        PORT_BIT (0x080, 0x00, IPT_UNUSED)
        /* 1 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "YELLOW/FUNCTION", KEYCODE_RALT, IP_JOY_NONE) 
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "CONTROL", KEYCODE_LCONTROL, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "CONTROL", KEYCODE_RCONTROL, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ESCAPE/STOP", KEYCODE_ESC, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "SPACE", KEYCODE_SPACE, IP_JOY_NONE)
        PORT_BIT (0x010, 0x00, IPT_UNUSED)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "5 %", KEYCODE_5, IP_JOY_NONE)
        PORT_BIT (0x080, 0x00, IPT_UNUSED)
        /* 2 */
        PORT_START
		PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ALT", KEYCODE_LALT, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "SYMBOL", KEYCODE_HOME, IP_JOY_NONE) 
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "1 !", KEYCODE_1, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "TAB", KEYCODE_TAB, IP_JOY_NONE)
	    PORT_BIT (0x010, 0x00, IPT_UNUSED)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BIT (0x040, 0x00, IPT_UNUSED)
        PORT_BIT (0x080, 0x00, IPT_UNUSED)
        /* 3 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "3", KEYCODE_3, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "2 \" ", KEYCODE_2, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Q", KEYCODE_Q, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "W", KEYCODE_W, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "E", KEYCODE_E, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "S", KEYCODE_S, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "D", KEYCODE_D, IP_JOY_NONE)
        /* 4 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "4 $", KEYCODE_4, IP_JOY_NONE)
        PORT_BIT (0x002, 0x00, IPT_UNUSED)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Z", KEYCODE_Z, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "X", KEYCODE_X, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "A", KEYCODE_A, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "R", KEYCODE_R, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "F", KEYCODE_F, IP_JOY_NONE)
        /* 5 */
        PORT_START
        PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BIT (0x002, 0x00, IPT_UNUSED)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "B", KEYCODE_B, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "V", KEYCODE_V, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "T", KEYCODE_T, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Y", KEYCODE_Y, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "G", KEYCODE_G, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "C", KEYCODE_C, IP_JOY_NONE)
        /* 6 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "6 ^", KEYCODE_6, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "DOWN/BLUE", KEYCODE_DOWN, IP_JOY_NONE)
        PORT_BIT (0x004, 0x00, IPT_UNUSED)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RIGHT/GREEN", KEYCODE_RIGHT, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "#", KEYCODE_TILDE, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "?", KEYCODE_SLASH, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "H", KEYCODE_H, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "N", KEYCODE_N, IP_JOY_NONE)
        /* 7 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "+ =", KEYCODE_EQUALS,IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "7 & ", KEYCODE_7, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "/ |", KEYCODE_BACKSLASH, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "UP", KEYCODE_UP, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "MENU", KEYCODE_PGUP, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "U", KEYCODE_U, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "M", KEYCODE_M, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "K", KEYCODE_K, IP_JOY_NONE)
        /* 8 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "8 *", KEYCODE_8, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "- _", KEYCODE_MINUS, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "} ]", KEYCODE_CLOSEBRACE, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "{ [", KEYCODE_OPENBRACE, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "@", KEYCODE_QUOTE, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "I", KEYCODE_I, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "J", KEYCODE_J, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, ",", KEYCODE_COMMA, IP_JOY_NONE)
        /* 9 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "0 )", KEYCODE_0, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "9 (", KEYCODE_9, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "DEL", KEYCODE_BACKSPACE, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "P", KEYCODE_P, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, ": ;", KEYCODE_COLON, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "L", KEYCODE_L, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "O", KEYCODE_O, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, ".", KEYCODE_STOP,IP_JOY_NONE)

        /* these are not part of the nc100 keyboard */
        /* extra */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ON BUTTON", KEYCODE_END, IP_JOY_NONE)
		/* pcmcia memory card setting */
		PORT_BITX(0x002, 0x002, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "PCMCIA Memory card write enable", IP_KEY_NONE, IP_JOY_NONE)
		PORT_DIPSETTING(0x000, DEF_STR( Off) )
		PORT_DIPSETTING(0x002, DEF_STR( On) )


INPUT_PORTS_END

/**********************************************************************************************************/
/* NC150 hardware */
/* to be completed! */

#if 0
void nc150_init_machine(void)
{
        nc_memory = (unsigned char *)malloc(nc_memory_size);
        nc_membank_internal_ram_mask = 7;

        nc_membank_card_ram_mask = 0x03f;

        nc_common_init_machine();
}
#endif



/**********************************************************************************************************/
/* NC200 hardware */


static void nc200_printer_handshake_in(int number, int data, int mask)
{
	nc_irq_status &= ~(1<<0);

	if (mask & CENTRONICS_ACKNOWLEDGE)
	{
		if (data & CENTRONICS_ACKNOWLEDGE)
		{
			nc_irq_status|=(1<<0);
		}
	}
	/* trigger an int if the irq is set */
	nc_update_interrupts();
}

static CENTRONICS_CONFIG nc200_cent_config[1]={
	{
		PRINTER_CENTRONICS,
		nc200_printer_handshake_in
	},
};



/* assumption. nc200 uses the same uart chip. The rxrdy and txrdy are combined
together with a or to generate a single interrupt */
static UINT8 nc200_uart_interrupt_irq;

static void nc200_refresh_uart_interrupt(void)
{
	nc_irq_latch &=~(1<<2);

	/* uart enabled? */
	if ((nc_uart_control & (1<<3))==0)
	{
		if ((nc200_uart_interrupt_irq & 0x03)!=0)
		{
			nc_irq_latch |= (1<<2);
		}
	}
	nc_update_interrupts();
}

static void nc200_txrdy_callback(int state)
{
//	nc200_uart_interrupt_irq &=~(1<<0);
//
//	if (state)
//	{
//		nc200_uart_interrupt_irq |=(1<<0);
//	}
//
//	nc200_refresh_uart_interrupt();
}

static void nc200_rxrdy_callback(int state)
{
	nc200_uart_interrupt_irq &=~(1<<1);

	if (state)
	{
		nc200_uart_interrupt_irq |=(1<<1);
	}

	nc200_refresh_uart_interrupt();
}

static struct msm8251_interface nc200_uart_interface=
{
	nc200_rxrdy_callback,
	NULL,
	nc200_txrdy_callback,
};


static void nc200_fdc_interrupt(int state)
{
    nc_irq_latch &=~(1<<5);

    if (state)
    {
            nc_irq_latch |=(1<<5);
    }

    nc_update_interrupts();
}

static struct nec765_interface nc200_nec765_interface=
{
    nc200_fdc_interrupt,
    NULL,
};

static void nc200_floppy_drive_index_callback(int drive_id)
{
#ifdef NC200_DEBUG
	logerror("nc200 index pulse\n");
#endif
	nc_irq_status |= (1<<4);

	nc_update_interrupts();
}

void nc200_init_machine(void)
{
    nc_type = NC_TYPE_200;

	/* 512k of rom */
	nc_membank_rom_mask = 0x1f;


    nc_memory_size = 128*1024;
    nc_memory = (unsigned char *)malloc(nc_memory_size);
    nc_membank_internal_ram_mask = 7;

    nc_membank_card_ram_mask = 0x03f;

    nc_common_init_machine();

    nec765_init(&nc200_nec765_interface, NEC765A);
    /* double sided, 80 track drive */
	floppy_drive_set_geometry(0, FLOPPY_DRIVE_DS_80);
	floppy_drive_set_index_pulse_callback(0, nc200_floppy_drive_index_callback);

	mc146818_init(MC146818_STANDARD);

	nc200_uart_interrupt_irq = 0;
	msm8251_init(&nc200_uart_interface);

	centronics_config(0, nc200_cent_config);
	/* assumption: select is tied low */
	centronics_write_handshake(0, CENTRONICS_SELECT | CENTRONICS_NO_RESET, CENTRONICS_SELECT| CENTRONICS_NO_RESET);

	nc_common_open_stream_for_reading();
	if (file)
	{
		mc146818_load_stream(file);
	}
	nc_common_restore_memory_from_stream();
	nc_common_close_stream();

	/* fdc, serial */
	nc_irq_latch_mask = (1<<5) | (1<<2);
}


void	nc200_shutdown_machine(void)
{
	nc_common_open_stream_for_writing();
	if (file)
	{
		mc146818_save_stream(file);
	}
	nc_common_store_memory_to_stream();
	nc_common_close_stream();

	nc_common_shutdown_machine();
	mc146818_close();
}

/*
NC200:

		bit 7: memory card present 0 = yes, 1 = no
		bit 6: memory card write protected 1=yes 0=no
		bit 5: lithium battery 0 if >= 2.7 volts
		bit 4: input voltage = 1, if >= to 4 volts
		bit 3: ??
		bit 2: alkaline batteries. 0 if >=3.2 volts
		bit 1: ??
		bit 0: battery power: if 1: batteries are too low for disk usage, if 0: batteries ok for disc usage
*/


/* nc200 version of card/battery status */
READ_HANDLER(nc200_card_battery_status_r)
{
	int nc_card_battery_status = 0x0ff;

	/* enough power */

	/* input voltage ok */
	nc_card_battery_status |=(1<<4);
	/* lithium batteries and alkaline batteries have enough power,
	and there is enough power for disk usage */
	nc_card_battery_status &=~((1<<5) | (1<<2) | (1<<0));

	if (nc_card_status & (1<<1))
	{
		/* card present */
		nc_card_battery_status&=~(1<<7);
	}

	if (readinputport(10) & 0x02)
	{
		/* card write enable */
		nc_card_battery_status &=~(1<<6);
	}

	return nc_card_battery_status;
}


/* port &80:

  bit 0: Parallel interface BUSY
 */

READ_HANDLER(nc200_printer_status_r)
{
	unsigned char nc200_printer_status = 0x0ff;

	int printer_handshake;
	
	/* assumption: select is tied low */
	centronics_write_handshake(0, CENTRONICS_SELECT | CENTRONICS_NO_RESET, CENTRONICS_SELECT| CENTRONICS_NO_RESET);

	printer_handshake = centronics_read_handshake(0);

	nc200_printer_status |=(1<<0);

	/* if printer is not online, it is busy */
	if ((printer_handshake & CENTRONICS_ONLINE)!=0)
	{
		nc200_printer_status &=~(1<<0);
	}

    return nc200_printer_status;
}


WRITE_HANDLER(nc200_uart_control_w)
{
	int reset_fdc;

	reset_fdc = (nc_uart_control^data) & (1<<5);

	nc_uart_control_w(offset,data);

	if (data & (1<<3))
	{
		nc200_uart_interrupt_irq &=~3;
	
		nc200_refresh_uart_interrupt();
	}

	/* bit 5 is used in disk interface */
#ifdef NC200_DEBUG
	logerror("bit 5: PC: %04x %02x\n",cpu_get_pc(), data & (1<<5));
#endif
}


/* bit 7: same as nc100 */
/* bit 2: ?? */
/* bit 1: ?? */
/* %10000110 = 0x086 */
/* %10000010 = 0x082 */
/* %10000011 = 0x083 */
/* writes 86,82 */

/* bit 7: nc200 power control: 1=on, 0=off */
/* bit 2: ?? */
/* bit 1: ?? */
/* bit 0: NEC765 Terminal Count input */

WRITE_HANDLER(nc200_memory_card_wait_state_w)
{
#ifdef NC200_DEBUG
	logerror("nc200 memory card wait state: PC: %04x %02x\n",cpu_get_pc(),data);
#endif
	floppy_drive_set_motor_state(0,1);
	floppy_drive_set_ready_state(0,1,1);

	nec765_set_tc_state((data & 0x01));
}

/* bit 1 cleared to zero in disk code */
/* bit 0 seems to be the same as nc100 */
WRITE_HANDLER(nc200_poweroff_control_w)
{
#ifdef NC200_DEBUG
	logerror("nc200 power off: PC: %04x %02x\n", cpu_get_pc(),data);
#endif
}

PORT_READ_START( readport_nc200 )
	{0x010, 0x013, nc_memory_management_r},
	{0x080, 0x080, nc200_printer_status_r},
	{0x0b0, 0x0b9, nc_key_data_in_r},
	{0x090, 0x090, nc_irq_status_r},
	{0x0a0, 0x0a0, nc200_card_battery_status_r},
	{0x0c1, 0x0c1, msm8251_status_r},
	{0x0c0, 0x0c0, msm8251_data_r},
	{0x0d0, 0x0d1, mc146818_port_r },
	{0x0e0, 0x0e0, nec765_status_r},
	{0x0e1, 0x0e1, nec765_data_r},
PORT_END

PORT_WRITE_START( writeport_nc200 )
	{0x000, 0x000, nc_display_memory_start_w},
	{0x010, 0x013, nc_memory_management_w},
	{0x020, 0x020, nc200_memory_card_wait_state_w},
	{0x040, 0x040, nc_printer_data_w},
	{0x030, 0x030, nc200_uart_control_w},
	{0x060, 0x060, nc_irq_mask_w},
	{0x070, 0x070, nc200_poweroff_control_w},
	{0x090, 0x090, nc_irq_status_w},
	{0x0c0, 0x0c0, msm8251_data_w},
	{0x0c1, 0x0c1, msm8251_control_w},
	{0x0d0, 0x0d1, mc146818_port_w },
	{0x050, 0x053, nc_sound_w},
    {0x0e1, 0x0e1, nec765_data_w},
PORT_END


INPUT_PORTS_START(nc200)
        /* 0 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "LEFT SHIFT", KEYCODE_LSHIFT, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RIGHT SHIFT", KEYCODE_RSHIFT, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "4 $", KEYCODE_4, IP_JOY_NONE)
	    PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "LEFT/RED", KEYCODE_LEFT, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RETURN", KEYCODE_ENTER, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BIT (0x040, 0x00, IPT_UNUSED)
		PORT_BIT (0x080, 0x00, IPT_UNUSED)
	    /* 1 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "YELLOW/FUNCTION", KEYCODE_RALT, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "CONTROL", KEYCODE_LCONTROL, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "CONTROL", KEYCODE_RCONTROL, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ESCAPE/STOP", KEYCODE_ESC, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "SPACE", KEYCODE_SPACE, IP_JOY_NONE)
        PORT_BIT (0x010, 0x00, IPT_UNUSED)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BIT (0x040, 0x00, IPT_UNUSED)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "9 (", KEYCODE_9, IP_JOY_NONE)
        /* 2 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ALT", KEYCODE_LALT, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "SYMBOL", KEYCODE_HOME, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "1 !", KEYCODE_1, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "TAB", KEYCODE_TAB, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "5", KEYCODE_5, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "6 ^", KEYCODE_6, IP_JOY_NONE)
        PORT_BIT (0x080, 0x00, IPT_UNUSED)
	    /* 3 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "3", KEYCODE_3, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "2 \"", KEYCODE_2, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Q", KEYCODE_Q, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "W", KEYCODE_W, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "E", KEYCODE_E, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "S", KEYCODE_S, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "D", KEYCODE_D, IP_JOY_NONE)
        /* 4 */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "8 *", KEYCODE_8, IP_JOY_NONE)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "7 &", KEYCODE_7, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Z", KEYCODE_Z, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "X", KEYCODE_X, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "A", KEYCODE_A, IP_JOY_NONE)
        PORT_BIT (0x020, 0x00, IPT_UNUSED)
	    PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "R", KEYCODE_R, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "F", KEYCODE_F, IP_JOY_NONE)
        /* 5 */
        PORT_START
        PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BIT (0x002, 0x00, IPT_UNUSED)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "B", KEYCODE_B, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "V", KEYCODE_V, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "T", KEYCODE_T, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "Y", KEYCODE_Y, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "G", KEYCODE_G, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "C", KEYCODE_C, IP_JOY_NONE)
        /* 6 */
        PORT_START        
		PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "DOWN/BLUE", KEYCODE_DOWN, IP_JOY_NONE)
        PORT_BIT (0x004, 0x00, IPT_UNUSED)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "RIGHT/GREEN", KEYCODE_RIGHT, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "#", KEYCODE_TILDE, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "?", KEYCODE_SLASH, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "H", KEYCODE_H, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "N", KEYCODE_N, IP_JOY_NONE)
        /* 7 */
        PORT_START
        PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "+ = ", KEYCODE_EQUALS, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "/ |", KEYCODE_BACKSLASH, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "UP", KEYCODE_UP, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "MENU", KEYCODE_PGUP, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "U", KEYCODE_U, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "M", KEYCODE_M, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, "K", KEYCODE_K, IP_JOY_NONE)
        /* 8 */
        PORT_START
        PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "- _", KEYCODE_MINUS, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "} ]", KEYCODE_CLOSEBRACE, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "{ [", KEYCODE_OPENBRACE, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, "@", KEYCODE_QUOTE, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "I", KEYCODE_I, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "J", KEYCODE_J, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, ",", KEYCODE_COMMA, IP_JOY_NONE)
        /* 9 */
        PORT_START
        PORT_BIT (0x001, 0x00, IPT_UNUSED)
        PORT_BITX(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD, "0 )", KEYCODE_0, IP_JOY_NONE)
        PORT_BITX(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD, "DEL", KEYCODE_BACKSPACE, IP_JOY_NONE)
        PORT_BITX(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD, "P", KEYCODE_P, IP_JOY_NONE)
        PORT_BITX(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD, ": ;", KEYCODE_COLON, IP_JOY_NONE)
        PORT_BITX(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD, "L", KEYCODE_L, IP_JOY_NONE)
        PORT_BITX(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD, "O", KEYCODE_O, IP_JOY_NONE)
        PORT_BITX(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD, ".", KEYCODE_STOP,IP_JOY_NONE)

        /* not part of the nc200 keyboard */
        PORT_START
        PORT_BITX(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD, "ON BUTTON", KEYCODE_END, IP_JOY_NONE)
		/* pcmcia memory card setting */
		PORT_BITX(0x002, 0x002, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "PCMCIA Memory card write enable", IP_KEY_NONE, IP_JOY_NONE)
		PORT_DIPSETTING(0x000, DEF_STR( Off) )
		PORT_DIPSETTING(0x002, DEF_STR( On) )
		
INPUT_PORTS_END


/**********************************************************************************************************/

static struct beep_interface nc_beep_interface =
{
	2,
	{50,50}
};

static struct MachineDriver machine_driver_nc100 =
{
	/* basic machine hardware */
	{
		/* MachineCPU */
		{
            CPU_Z80 ,  /* type */
            6000000, /* clock: See Note Above */
            readmem_nc,                   /* MemoryReadAddress */
            writemem_nc,                  /* MemoryWriteAddress */
            readport_nc100,                  /* IOReadPort */
            writeport_nc100,                 /* IOWritePort */
			0,						   /* VBlank Interrupt */
			0  ,				   /* vblanks per frame */
            0, 0,   /* every scanline */
		},
	},
	50,                                                     /* frames per second */
	DEFAULT_60HZ_VBLANK_DURATION,	   /* vblank duration */
	1,								   /* cpu slices per frame */
	nc100_init_machine,                      /* init machine */
	nc100_shutdown_machine,
	/* video hardware */
	640/*NC_SCREEN_WIDTH*/, /* screen width */
	480/*NC_SCREEN_HEIGHT*/,  /* screen height */
	{0, (640/*NC_SCREEN_WIDTH*/ - 1), 0, (480/*NC_SCREEN_HEIGHT*/ - 1)},        /* rectangle: visible_area */
	0,							   /* graphics
										* decode info */
	NC_NUM_COLOURS,                                                        /* total colours */
	NC_NUM_COLOURS,                                                        /* color table len */
	nc_init_palette,                      /* init palette */

	VIDEO_TYPE_RASTER,                                  /* video attributes */
	0,                                                                 /* MachineLayer */
	nc_vh_start,
	nc_vh_stop,
	nc_vh_screenrefresh,

		/* sound hardware */
	0,								   /* sh init */
	0,								   /* sh start */
	0,								   /* sh stop */
	0,								   /* sh update */
    {
        {
           SOUND_BEEP,
           &nc_beep_interface
        }
    }
};



static struct MachineDriver machine_driver_nc200 =
{
	/* basic machine hardware */
	{
		/* MachineCPU */
		{
			CPU_Z80 ,  /* type */
			6000000, /* clock: See Note Above */
			readmem_nc,                   /* MemoryReadAddress */
			writemem_nc,                  /* MemoryWriteAddress */
			readport_nc200,                  /* IOReadPort */
			writeport_nc200,                 /* IOWritePort */
			0,						   /* VBlank Interrupt */
			0,				   /* vblanks per frame */
			0, 0,   /* every scanline */
		},
	},
        50,                                                     /* frames per second */
	DEFAULT_60HZ_VBLANK_DURATION,	   /* vblank duration */
	1,								   /* cpu slices per frame */
	nc200_init_machine,                      /* init machine */
	nc200_shutdown_machine,
	/* video hardware */
	NC200_SCREEN_WIDTH, /* screen width */
	NC200_SCREEN_HEIGHT,  /* screen height */
	{0, (NC200_SCREEN_WIDTH - 1), 0, (NC200_SCREEN_HEIGHT - 1)},        /* rectangle: visible_area */
	0,								   /* graphics
										* decode info */
	NC200_NUM_COLOURS,                                                        /* total colours */
	NC200_NUM_COLOURS,                                                        /* color table len */
	nc_init_palette,                      /* init palette */

	VIDEO_TYPE_RASTER,                                  /* video attributes */
	0,                                                                 /* MachineLayer */
	nc_vh_start,
	nc_vh_stop,
	nc_vh_screenrefresh,

	/* sound hardware */
	0,								   /* sh init */
	0,								   /* sh start */
	0,								   /* sh stop */
	0,								   /* sh update */
	{
		{
		   SOUND_BEEP,
		   &nc_beep_interface
		}
	}
};


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(nc100)
        ROM_REGION(((64*1024)+(256*1024)), REGION_CPU1,0)
        ROM_LOAD("nc100.rom", 0x010000, 0x040000, 0x0a699eca3)
ROM_END

ROM_START(nc100a)
        ROM_REGION(((64*1024)+(256*1024)), REGION_CPU1,0)
        ROM_LOAD("nc100a.rom", 0x010000, 0x040000, 0x0849884f9)
ROM_END

ROM_START(nc200)
        ROM_REGION(((64*1024)+(512*1024)), REGION_CPU1,0)
        ROM_LOAD("nc200.rom", 0x010000, 0x080000, 0x0bb8180e7)
ROM_END


static const struct IODevice io_nc100[] =
{
	{
			IO_CARTSLOT,           /* type */
			1,                     /* count */
			"crd\0card\0",               /* file extensions */
			IO_RESET_NONE,			/* reset if file changed */
			nc_pcmcia_card_id,   /* id */
			nc_pcmcia_card_load, /* load */
			nc_pcmcia_card_exit, /* exit */
			NULL,                   /* info */
			NULL,                   /* open */
			NULL,                   /* close */
			NULL,                   /* status */
			NULL,                   /* seek */
			NULL,                   /* tell */
			NULL,                   /* input */
			NULL,                   /* output */
			NULL,                   /* input chunk */
			NULL,                   /* output chunk */
	},
	{
			IO_SERIAL,           /* type */
			1,                     /* count */
			"txt\0",               /* file extensions */
			IO_RESET_NONE,			/* reset if file changed */
			NULL,   /* id */
			nc_serial_init, /* load */
			serial_device_exit, /* exit */
			NULL,                   /* info */
			NULL,                   /* open */
			NULL,                   /* close */
			NULL,                   /* status */
			NULL,                   /* seek */
			NULL,                   /* tell */
			NULL,                   /* input */
			NULL,                   /* output */
			NULL,                   /* input chunk */
			NULL,                   /* output chunk */
	},		
	IO_PRINTER_PORT(1,"\0"),
	{IO_END}
};

static const struct IODevice io_nc200[] =
{
	{
			IO_CARTSLOT,           /* type */
			1,                     /* count */
			"crd\0card\0",               /* file extensions */
			IO_RESET_NONE,			/* reset if file changed */
			nc_pcmcia_card_id,   /* id */
			nc_pcmcia_card_load, /* load */
			nc_pcmcia_card_exit, /* exit */
			NULL,                   /* info */
			NULL,                   /* open */
			NULL,                   /* close */
			NULL,                   /* status */
			NULL,                   /* seek */
			NULL,                   /* tell */
			NULL,                   /* input */
			NULL,                   /* output */
			NULL,                   /* input chunk */
			NULL,                   /* output chunk */
	},
	{
			IO_FLOPPY,
			1,
			"dsk\0",
			IO_RESET_NONE,
			NULL,
			pc_floppy_init,
			pc_floppy_exit,
			NULL,                   /* info */
			NULL,                   /* open */
			NULL,                   /* close */
			floppy_status,                   /* status */
			NULL,                   /* seek */
			NULL,                   /* tell */
			NULL,                   /* input */
			NULL,                   /* output */
			NULL,                   /* input chunk */
			NULL,                   /* output chunk */
	},
	{
			IO_SERIAL,           /* type */
			1,                     /* count */
			"txt\0",               /* file extensions */
			IO_RESET_NONE,			/* reset if file changed */
			NULL,   /* id */
			nc_serial_init, /* load */
			serial_device_exit, /* exit */
			NULL,                   /* info */
			NULL,                   /* open */
			NULL,                   /* close */
			NULL,                   /* status */
			NULL,                   /* seek */
			NULL,                   /* tell */
			NULL,                   /* input */
			NULL,                   /* output */
			NULL,                   /* input chunk */
			NULL,                   /* output chunk */
	},	
	IO_PRINTER_PORT(1,"\0"),
	{IO_END}
};


#define io_nc100a io_nc100

/*	  YEAR	NAME	 PARENT	MACHINE INPUT 	INIT COMPANY        FULLNAME */
COMP( 1992, nc100,   0,     nc100,  nc100,  0,   "Amstrad plc", "NC100")
COMP( 1992, nc100a,  nc100, nc100,  nc100,  0,   "Amstrad plc", "NC100 (Version 1.09)")
COMP( 1993, nc200,   0,     nc200,  nc200,  0,   "Amstrad plc", "NC200")
