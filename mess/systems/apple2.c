/***************************************************************************

Apple II

This family of computers bank-switches everything up the wazoo.

Remarkable features
-------------------

Apple II (original model)
-------------------------

RAM: 4/8/12/16/20/24/32/36/48 KB (according to the manual)

ROM: 8 KB mapped to $E000-$FFFF
Empty ROM sockets mapped at $D000-$D7FF (usually occupied by Programmer's
Aid #1 chip) and $D800-$DFFF (usually empty, but a couple of 3rd party
chips were produced)

HI-RES Palette has only 4 colors: 0 - black, 1 - green, 2 - purple,
3 - white

Due to an hardware bug, green/purple artifacts are present in text mode
too!

No 80 columns
No Open/Solid Apple keys
No Up/Down arrows key

Users often connected the SHIFT key to the paddle #2 button (mapped to
$C063) in order to inform properly written software that characters were
to be intended upper/lower case

*** TODO: Should MESS emulate this via a dipswitch?

Integer BASIC in ROM, AppleSoft must be loaded from disk or tape

No AutoStart ROM: once the machine was switched on, the user had to manually
perform the reset cycle pressing, guess what, RESET ;)

Apple II+
---------

RAM: 16/32/48 KB + extra 16 KB at $C000 if using Apple Language Card
ROM: 12 KB mapped to $D000-$DFFF

HI-RES Palette has four more entries: 4 - black (again), 5 - orange,
6 - blue, 7 - white (again)

No more artifact bug in text mode

No 80 columns
No Open/Solid Apple keys
NO Up/Down arrows keys

Users still did the SHIFT key mod

AppleSoft BASIC in ROM

AutoStart ROM - no more need to press RESET after switching the machine on

Apple IIe
---------

RAM: 64 KB + optional bank of 64 KB (see 80 columns card)
ROM: 16 KB

80 columns card: this card was available in two versions - one equipped
with 1 KB of memory to provide the extra RAM for the display, the other
equipped with full 64 KB of RAM - the 80 columns card is not included
in the standard configuration and is available as add-on.

Open/Solid Apple keys mapped to buttons 0 and 1 of the paddle #1
Up/Down arrows keys
Connector for an optional numeric keypad

Apple begins manifacturing its machines with the SHIFT key mod

Revision A motherboards cannot handle double-hires graphics, Revision B can

*** TODO: Should MESS emulate this via a dipswitch?

Apple IIe (enhanced)
--------------------

The enhancement consists in bugfix of the ROM code, a 65c02 instead of the
6502 and a change in the character generator ROM which now includes the
so called "MouseText" characters (thus, no flashing characters in 80
columns mode)

Double hi-res mode is supported

Apple IIe (Platinum)
--------------------

Identical to IIe enhanced except for:

The numerical keypad is integrated into the main keyboard (although the
internal connector is still present)

The CLEAR key on the keypad generates the same character of the ESC key,
but some users did an hardware modification so that it generates CTRL-X

*** TODO: Should MESS emulate this via a dipswitch?

The 64 KB 80 columns card is built in

Due to the SHIFT key mod, if the user press both SHIFT and the paddle
button where the shift key was connected, a short circuit is caused
and the power supply is shut down!

Apple IIc
---------

Same as IIe enhanced (Rev B) except for:

There are no slots in hardware. The machine however sees (for compatibility
reasons):

Two Super Serial Cards in slots 1-2
80 columns card (64 KB version) in slot 3
Mouse in slot 4
Easter Egg in slot 5 (!)
Disk II in slot 6
External 5.25 drive in slot 7

MouseText characters

No numerical keypad

Switchables keyboard layouts (the user, via an external switch, can choose
between two layouts, i.e. US and German, and in the USA QWERTY and Dvorak)

*** TODO: Should MESS emulate this?

Apple IIc (UniDisk 3.5)
-----------------------

Identical to IIc except for:

ROM: 32 KB

The disk firmware can handle up to four 3.5 disk drives or three 3.5 drives
and a 5.25 drive

Preliminary support (but not working and never completed) for AppleTalk
network in slot 7

Apple IIc (Original Memory Expansion)
-------------------------------------

ROMSET NOT DUMPED

Identical to IIc except for:

Support for Memory Expansion Board (mapped to slot 4)
This card can provide up to 1 MB of RAM in increments of 256 MB
The firmware in ROM sees the extra RAM as a RAMdisk

Since the expansion is mapped to slot 4, mouse is now mapped to slot 7

Apple IIc (Revised Memory Expansion)
------------------------------------

ROMSET NOT DUMPED

Identical to IIc (OME) except for bugfixes

Apple IIc Plus
--------------

Identical to IIc (RME) except for:

The 65c02 works at 4MHz

The machine has an internal "Apple 3.5" drive (which is DIFFERENT from the
UniDisk 3.5 drive!)

The external drive port supports not only 5.25 drives but also UniDisk and
Apple 3.5 drives, allowing via daisy-chaining any combination of UniDisk,
Apple 3.5 and Apple 5.25 drives - up to three devices

***************************************************************************/

#include "driver.h"
#include "inputx.h"
#include "vidhrdw/generic.h"
#include "includes/apple2.h"
#include "machine/ay3600.h"
#include "devices/mflopimg.h"
#include "formats/ap2_dsk.h"

static ADDRESS_MAP_START( apple2_map, ADDRESS_SPACE_PROGRAM, 8 )
ADDRESS_MAP_END


/**************************************************************************
***************************************************************************/

#define JOYSTICK_DELTA			80
#define JOYSTICK_SENSITIVITY	50

static INPUT_PORTS_START( apple2_joystick )
	PORT_START_TAG("joystick_1_x")		/* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xff) PORT_PLAYER(1) PORT_RESET
	PORT_CODE_DEC(KEYCODE_4_PAD)	PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_1_LEFT)	PORT_CODE_INC(JOYCODE_1_RIGHT)

	PORT_START_TAG("joystick_1_y")		/* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xff) PORT_PLAYER(1) PORT_RESET
	PORT_CODE_DEC(KEYCODE_8_PAD)	PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_1_UP)		PORT_CODE_INC(JOYCODE_1_DOWN)

	PORT_START_TAG("joystick_2_x")		/* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xff) PORT_PLAYER(2) PORT_RESET
	PORT_CODE_DEC(JOYCODE_2_LEFT)	PORT_CODE_INC(JOYCODE_2_RIGHT)

	PORT_START_TAG("joystick_2_y")		/* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xff) PORT_PLAYER(2) PORT_RESET
	PORT_CODE_DEC(JOYCODE_2_UP)		PORT_CODE_INC(JOYCODE_2_DOWN)
INPUT_PORTS_END



INPUT_PORTS_START( apple2 )

    PORT_START_TAG("keyb_0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")	PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\x1b")		PORT_CODE(KEYCODE_LEFT)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")		PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\x19")		PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\x18")		PORT_CODE(KEYCODE_UP)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")	PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\x1a")		PORT_CODE(KEYCODE_RIGHT)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")		PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)

    PORT_START_TAG("keyb_1")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)		PORT_CHAR(' ')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)		PORT_CHAR('\'') PORT_CHAR('\"')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)		PORT_CHAR(',') PORT_CHAR('<')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('-') PORT_CHAR('_')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)		PORT_CHAR('.') PORT_CHAR('>')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)		PORT_CHAR('/') PORT_CHAR('?')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)			PORT_CHAR('0') PORT_CHAR(')')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')

    PORT_START_TAG("keyb_2")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('@')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)			PORT_CHAR('3') PORT_CHAR('#')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)			PORT_CHAR('5') PORT_CHAR('%')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)			PORT_CHAR('6') PORT_CHAR('^')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)			PORT_CHAR('7') PORT_CHAR('&')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('*')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR('(')

    PORT_START_TAG("keyb_3")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)		PORT_CHAR(';') PORT_CHAR(':')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)		PORT_CHAR('=') PORT_CHAR('+')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('[') PORT_CHAR('{')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('\\') PORT_CHAR('|')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)	PORT_CHAR(']') PORT_CHAR('}')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)		PORT_CHAR('`') PORT_CHAR('~')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)			PORT_CHAR('A') PORT_CHAR('a')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)			PORT_CHAR('B') PORT_CHAR('b')

    PORT_START_TAG("keyb_4")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)			PORT_CHAR('C') PORT_CHAR('c')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)			PORT_CHAR('D') PORT_CHAR('d')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)			PORT_CHAR('E') PORT_CHAR('e')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)			PORT_CHAR('F') PORT_CHAR('f')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)			PORT_CHAR('G') PORT_CHAR('g')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)			PORT_CHAR('H') PORT_CHAR('h')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)			PORT_CHAR('I') PORT_CHAR('i')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)			PORT_CHAR('J') PORT_CHAR('j')

    PORT_START_TAG("keyb_5")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)			PORT_CHAR('K') PORT_CHAR('k')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)			PORT_CHAR('L') PORT_CHAR('l')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)			PORT_CHAR('M') PORT_CHAR('m')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)			PORT_CHAR('N') PORT_CHAR('n')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)			PORT_CHAR('O') PORT_CHAR('o')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)			PORT_CHAR('P') PORT_CHAR('p')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)			PORT_CHAR('Q') PORT_CHAR('q')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)			PORT_CHAR('R') PORT_CHAR('r')

    PORT_START_TAG("keyb_6")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) 			PORT_CHAR('S') PORT_CHAR('s')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) 			PORT_CHAR('T') PORT_CHAR('t')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) 			PORT_CHAR('U') PORT_CHAR('u')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) 			PORT_CHAR('V') PORT_CHAR('v')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) 			PORT_CHAR('W') PORT_CHAR('w')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) 			PORT_CHAR('X') PORT_CHAR('x')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) 			PORT_CHAR('Y') PORT_CHAR('y')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) 			PORT_CHAR('Z') PORT_CHAR('z')

    PORT_START_TAG("keyb_special")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")			PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")		PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")		PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")			PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")		PORT_CODE(KEYCODE_LALT)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")		PORT_CODE(KEYCODE_RALT)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_CODE(KEYCODE_0_PAD)		PORT_CODE(JOYCODE_1_BUTTON1)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
		PORT_CODE(KEYCODE_ENTER_PAD)	PORT_CODE(JOYCODE_1_BUTTON2)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_CODE(JOYCODE_2_BUTTON1)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)

	/* other devices */
	PORT_INCLUDE( apple2_joystick )
INPUT_PORTS_END



/* according to Steve Nickolas (author of Dapple), our original palette would
 * have been more appropriate for an Apple IIgs.  So we've substituted in the
 * Robert Munafo palette instead, which is more accurate on 8-bit Apples
 */
static const unsigned char apple2_palette[] =
{
	0x00, 0x00, 0x00,	/* Black */
	0xE3, 0x1E, 0x60,	/* Dark Red */
	0x60, 0x4E, 0xBD,	/* Dark Blue */
	0xFF, 0x44, 0xFD,	/* Purple */
	0x00, 0xA3, 0x60,	/* Dark Green */
	0x9C, 0x9C, 0x9C,	/* Dark Gray */
	0x14, 0xCF, 0xFD,	/* Medium Blue */
	0xD0, 0xC3, 0xFF,	/* Light Blue */
	0x60, 0x72, 0x03,	/* Brown */
	0xFF, 0x6A, 0x3C,	/* Orange */
	0x9C, 0x9C, 0x9C,	/* Light Grey */
	0xFF, 0xA0, 0xD0,	/* Pink */
	0x14, 0xF5, 0x3C,	/* Light Green */
	0xD0, 0xDD, 0x8D,	/* Yellow */
	0x72, 0xFF, 0xD0,	/* Aquamarine */
	0xFF, 0xFF, 0xFF	/* White */
};

static struct GfxLayout apple2_text_layout =
{
	14,8,		/* 14*8 characters */
	256,		/* 256 characters */
	1,			/* 1 bits per pixel */
	{ 0 },		/* no bitplanes; 1 bit per pixel */
	{ 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7 },   /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8			/* every char takes 8 bytes */
};

static struct GfxLayout apple2_dbltext_layout =
{
	7,8,		/* 7*8 characters */
	256,		/* 256 characters */
	1,			/* 1 bits per pixel */
	{ 0 },		/* no bitplanes; 1 bit per pixel */
	{ 1, 2, 3, 4, 5, 6, 7 },    /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8			/* every char takes 8 bytes */
};

static struct GfxDecodeInfo apple2_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x0000, &apple2_text_layout, 0, 2 },
	{ REGION_GFX1, 0x0000, &apple2_dbltext_layout, 0, 2 },
	{ -1 } /* end of array */
};

static struct GfxLayout apple2e_text_layout =
{
	14,8,		/* 14*8 characters */
	1024,		/* 256 characters */
	1,			/* 1 bits per pixel */
	{ 0 },		/* no bitplanes; 1 bit per pixel */
	{ 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1 },   /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8			/* every char takes 8 bytes */
};

static struct GfxLayout apple2e_dbltext_layout =
{
	7,8,		/* 7*8 characters */
	1024,		/* 256 characters */
	1,			/* 1 bits per pixel */
	{ 0 },		/* no bitplanes; 1 bit per pixel */
	{ 7, 6, 5, 4, 3, 2, 1 },    /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8			/* every char takes 8 bytes */
};

static struct GfxDecodeInfo apple2e_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x0000, &apple2e_text_layout, 0, 2 },
	{ REGION_GFX1, 0x0000, &apple2e_dbltext_layout, 0, 2 },
	{ -1 } /* end of array */
};

static const unsigned short apple2_colortable[] =
{
	0,	0,	0,	1,	0,	2,	0,	3,	0,	4,	0,	5,	0,	6,	0,	7,
	0,	8,	0,	9,	0,	10,	0,	11,	0,	12,	0,	13,	0,	14,	0,	15,
	1,	0,	1,	1,	1,	2,	1,	3,	1,	4,	1,	5,	1,	6,	1,	7,
	1,	8,	1,	9,	1,	10,	1,	11,	1,	12,	1,	13,	1,	14,	1,	15,
	2,	0,	2,	1,	2,	2,	2,	3,	2,	4,	2,	5,	2,	6,	2,	7,
	2,	8,	2,	9,	2,	10,	2,	11,	2,	12,	2,	13,	2,	14,	2,	15,
	3,	0,	3,	1,	3,	2,	3,	3,	3,	4,	3,	5,	3,	6,	3,	7,
	3,	8,	3,	9,	3,	10,	3,	11,	3,	12,	3,	13,	3,	14,	3,	15,
	4,	0,	4,	1,	4,	2,	4,	3,	4,	4,	4,	5,	4,	6,	4,	7,
	4,	8,	4,	9,	4,	10,	4,	11,	4,	12,	4,	13,	4,	14,	4,	15,
	5,	0,	5,	1,	5,	2,	5,	3,	5,	4,	5,	5,	5,	6,	5,	7,
	5,	8,	5,	9,	5,	10,	5,	11,	5,	12,	5,	13,	5,	14,	5,	15,
	6,	0,	6,	1,	6,	2,	6,	3,	6,	4,	6,	5,	6,	6,	6,	7,
	6,	8,	6,	9,	6,	10,	6,	11,	6,	12,	6,	13,	6,	14,	6,	15,
	7,	0,	7,	1,	7,	2,	7,	3,	7,	4,	7,	5,	7,	6,	7,	7,
	7,	8,	7,	9,	7,	10,	7,	11,	7,	12,	7,	13,	7,	14,	7,	15,
	8,	0,	8,	1,	8,	2,	8,	3,	8,	4,	8,	5,	8,	6,	8,	7,
	8,	8,	8,	9,	8,	10,	8,	11,	8,	12,	8,	13,	8,	14,	8,	15,
	9,	0,	9,	1,	9,	2,	9,	3,	9,	4,	9,	5,	9,	6,	9,	7,
	9,	8,	9,	9,	9,	10,	9,	11,	9,	12,	9,	13,	9,	14,	9,	15,
	10,	0,	10,	1,	10,	2,	10,	3,	10,	4,	10,	5,	10,	6,	10,	7,
	10,	8,	10,	9,	10,	10,	10,	11,	10,	12,	10,	13,	10,	14,	10,	15,
	11,	0,	11,	1,	11,	2,	11,	3,	11,	4,	11,	5,	11,	6,	11,	7,
	11,	8,	11,	9,	11,	10,	11,	11,	11,	12,	11,	13,	11,	14,	11,	15,
	12,	0,	12,	1,	12,	2,	12,	3,	12,	4,	12,	5,	12,	6,	12,	7,
	12,	8,	12,	9,	12,	10,	12,	11,	12,	12,	12,	13,	12,	14,	12,	15,
	13,	0,	13,	1,	13,	2,	13,	3,	13,	4,	13,	5,	13,	6,	13,	7,
	13,	8,	13,	9,	13,	10,	13,	11,	13,	12,	13,	13,	13,	14,	13,	15,
	14,	0,	14,	1,	14,	2,	14,	3,	14,	4,	14,	5,	14,	6,	14,	7,
	14,	8,	14,	9,	14,	10,	14,	11,	14,	12,	14,	13,	14,	14,	14,	15,
	15,	0,	15,	1,	15,	2,	15,	3,	15,	4,	15,	5,	15,	6,	15,	7,
	15,	8,	15,	9,	15,	10,	15,	11,	15,	12,	15,	13,	15,	14,	15,	15
};


/* Initialize the palette */
PALETTE_INIT( apple2 )
{
	palette_set_colors(0, apple2_palette, sizeof(apple2_palette) / 3);
	memcpy(colortable, apple2_colortable, sizeof(apple2_colortable));
}

static const char *apple2_floppy_getname(const struct IODevice *dev, int id, char *buf, size_t bufsize)
{
	snprintf(buf, bufsize, "Slot 6 Disk #%d", id + 1);
	return buf;
}

static struct DACinterface apple2_DAC_interface =
{
    1,          /* number of DACs */
    { 100 }     /* volume */
};

static struct AY8910interface ay8910_interface =
{
    2,  /* 2 chips */
    1022727,    /* 1.023 MHz */
    { 100, 100 },
    { 0 },
    { 0 },
    { 0 },
    { 0 }
};

static MACHINE_DRIVER_START( apple2_common )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M6502, 1021800)		/* close to actual CPU frequency of 1.020484 MHz */
	MDRV_CPU_PROGRAM_MAP(apple2_map, 0)
	MDRV_CPU_VBLANK_INT(apple2_interrupt, 192/8)
	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(1)

	MDRV_MACHINE_INIT( apple2 )

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_PIXEL_ASPECT_RATIO_1_2)
	MDRV_SCREEN_SIZE(280*2, 192)
	MDRV_VISIBLE_AREA(0, (280*2)-1,0,192-1)
	MDRV_PALETTE_LENGTH(sizeof(apple2_palette)/3)
	MDRV_COLORTABLE_LENGTH(sizeof(apple2_colortable)/sizeof(unsigned short))
	MDRV_PALETTE_INIT(apple2)

	MDRV_VIDEO_START(apple2)
	MDRV_VIDEO_UPDATE(apple2)

	/* sound hardware */
	MDRV_SOUND_ADD(DAC, apple2_DAC_interface)
	MDRV_SOUND_ADD(AY8910, ay8910_interface)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( apple2 )
	MDRV_IMPORT_FROM( apple2_common )
	MDRV_GFXDECODE(apple2_gfxdecodeinfo)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( apple2p )
	MDRV_IMPORT_FROM( apple2_common )
	MDRV_VIDEO_START(apple2p)
	MDRV_GFXDECODE(apple2_gfxdecodeinfo)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( f_ace100 )
	MDRV_IMPORT_FROM( apple2_common )
	MDRV_VIDEO_START(apple2p)
	MDRV_GFXDECODE(apple2_gfxdecodeinfo)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( apple2e )
	MDRV_IMPORT_FROM( apple2_common )
	MDRV_VIDEO_START(apple2e)
	MDRV_GFXDECODE(apple2e_gfxdecodeinfo)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( apple2ee )
	MDRV_IMPORT_FROM( apple2e )
	MDRV_CPU_REPLACE("main", M65C02, 1021800)		/* close to actual CPU frequency of 1.020484 MHz */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( apple2c )
	MDRV_IMPORT_FROM( apple2ee )
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2)
	ROM_REGION(0x0800,REGION_GFX1,0)
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD_OPTIONAL ( "progaid1.rom", 0x1000, 0x0800, CRC(4234e88a) SHA1(c9a81d704dc2f0c3416c20f9c4ab71fedda937ed))

/* The area $D800-$DFFF in Apple II is reserved for 3rd party add-ons:
   Maybe MESS should map this space to a CARTSLOT device?              */

	ROM_LOAD ( "a2.e0", 0x2000, 0x0800, CRC(c0a4ad3b) SHA1(bf32195efcb34b694c893c2d342321ec3a24b98f))
	ROM_LOAD ( "a2.e8", 0x2800, 0x0800, CRC(a99c2cf6) SHA1(9767d92d04fc65c626223f25564cca31f5248980))
	ROM_LOAD ( "a2.f0", 0x3000, 0x0800, CRC(62230d38) SHA1(f268022da555e4c809ca1ae9e5d2f00b388ff61c))
	ROM_LOAD ( "a2.f8", 0x3800, 0x0800, CRC(020a86d0) SHA1(52a18bd578a4694420009cad7a7a5779a8c00226))
	ROM_LOAD ( "disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(apple2p)
	ROM_REGION(0x0800,REGION_GFX1,0)
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD ( "a2p.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "a2p.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "a2p.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "a2p.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "a2p.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "a2p.f8", 0x3800, 0x0800, CRC(ecffd453) SHA1(07a3bdce3e34bbed5246fe09a9938d8f334a8225))
	ROM_LOAD ( "disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(f_ace100)
	ROM_REGION(0x0800,REGION_GFX1,0)
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD ( "ace100.rom", 0x1000, 0x3000, CRC(9d5ec94f) SHA1(8f2b3f2561788bebc7a805f620ec9e7ade973460))
	ROM_LOAD ( "disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(apple2e)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2ealt.chr", 0x1000, 0x1000,CRC(816a86f1) SHA1(58ad0008df72896a18601e090ee0d58155ffa5be))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD ( "a2e.cd", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "a2e.ef", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))
	ROM_LOAD ( "disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(apple2ee)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2eealt.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD ( "a2ee.cd", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6))
	ROM_LOAD ( "a2ee.ef", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e))
	ROM_LOAD ( "disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(apple2ep)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2eealt.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4700,REGION_CPU1,0)
	ROM_LOAD ("a2ept.cf", 0x0000, 0x4000, CRC(1d70b193) SHA1(b8ea90abe135a0031065e01697c4a3a20d51198b))
	ROM_LOAD ("disk2_33.rom", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16)) /* Disk II ROM - DOS 3.3 version */
ROM_END

ROM_START(apple2c)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2eealt.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4000,REGION_CPU1,0)
	ROM_LOAD ( "a2c.128", 0x0000, 0x4000, CRC(f0edaa1b) SHA1(1a9b8aca5e32bb702ddb7791daddd60a89655729))
ROM_END

ROM_START(apple2c0)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2eealt.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,REGION_CPU1,0)
	ROM_LOAD("a2c.256", 0x0000, 0x8000, CRC(c8b979b3) SHA1(10767e96cc17bad0970afda3a4146564e6272ba1))
ROM_END

ROM_START(apple2cp)
	ROM_REGION(0x2000,REGION_GFX1,0)
	ROM_LOAD ( "a2e.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "a2eealt.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,REGION_CPU1,0)
	ROM_LOAD("a2cplus.mon", 0x0000, 0x8000, CRC(0b996420) SHA1(1a27ae26966bbafd825d08ad1a24742d3e33557c))
ROM_END


static void apple2_floppy_getinfo(struct IODevice *dev)
{
	/* floppy */
	floppy_device_getinfo(dev, floppyoptions_apple2);
	dev->count = 2;
	dev->name = apple2_floppy_getname;
	dev->tag = APDISK_DEVTAG;
}

SYSTEM_CONFIG_START(apple2_common)
	CONFIG_DEVICE(apple2_floppy_getinfo)
	CONFIG_QUEUE_CHARS			( AY3600 )
	CONFIG_ACCEPT_CHAR			( AY3600 )
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(apple2)
	CONFIG_IMPORT_FROM( apple2_common )
//	CONFIG_RAM				(4 * 1024)	/* Still unsupported? */
//	CONFIG_RAM				(8 * 1024)	/* Still unsupported? */
//	CONFIG_RAM				(12 * 1024)	/* Still unsupported? */
//	CONFIG_RAM				(16 * 1024)
//	CONFIG_RAM				(20 * 1024)
//	CONFIG_RAM				(24 * 1024)
//	CONFIG_RAM				(32 * 1024)
//	CONFIG_RAM				(36 * 1024)
//	CONFIG_RAM				(48 * 1024)
//	CONFIG_RAM_DEFAULT			(64 * 1024)	/* At the moment the RAM bank $C000-$FFFF is available only if you choose   */
								/* default configuration: on real machine is present also in configurations */
								/* with less memory, provided that the language card is installed           */
	CONFIG_RAM_DEFAULT			(128 * 1024)	/* ONLY TEMPORARY - ACTUALLY NOT SUPPORTED!!! */
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(apple2p)
	CONFIG_IMPORT_FROM( apple2_common )
//	CONFIG_RAM				(16 * 1024)
//	CONFIG_RAM				(32 * 1024)
//	CONFIG_RAM				(48 * 1024)
//	CONFIG_RAM_DEFAULT			(64 * 1024)	/* At the moment the RAM bank $C000-$FFFF is available only if you choose   */
								/* default configuration: on real machine is present also in configurations */
								/* with less memory, provided that the language card is installed           */
	CONFIG_RAM_DEFAULT                      (128 * 1024)    /* ONLY TEMPORARY - ACTUALLY NOT SUPPORTED!!! */
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(apple2e)
	CONFIG_IMPORT_FROM( apple2_common )
	CONFIG_RAM_DEFAULT			(128 * 1024)
SYSTEM_CONFIG_END



/*     YEAR  NAME      PARENT    COMPAT		MACHINE   INPUT     INIT      CONFIG	COMPANY            FULLNAME */
COMP ( 1977, apple2,   0,        0,			apple2,   apple2,   apple2,   apple2,	"Apple Computer", "Apple ][" )
COMP ( 1979, apple2p,  apple2,   0,			apple2p,  apple2,   apple2,   apple2p,	"Apple Computer", "Apple ][+" )
COMP ( 1979, f_ace100, apple2,   0,			apple2p,  apple2,   apple2,   apple2p,	"Franklin", "Ace 100" )
COMP ( 1983, apple2e,  0,        apple2,	apple2e,  apple2,   apple2,   apple2e,	"Apple Computer", "Apple //e" )
COMP ( 1985, apple2ee, apple2e,  0,			apple2ee, apple2,   apple2,   apple2e,	"Apple Computer", "Apple //e (enhanced)" )
COMP ( 1987, apple2ep, apple2e,  0,			apple2ee, apple2,   apple2,   apple2e,	"Apple Computer", "Apple //e (Platinum)" )
COMP ( 1984, apple2c,  0,        apple2,	apple2c,  apple2,   apple2,   apple2e,	"Apple Computer", "Apple //c" )
COMPX( 1985, apple2c0, apple2c,  0,			apple2c,  apple2,   apple2,   apple2e,	"Apple Computer", "Apple //c (UniDisk 3.5)",	GAME_NOT_WORKING )
COMPX( 1988, apple2cp, apple2c,  0,			apple2c,  apple2,   apple2,   apple2e,	"Apple Computer", "Apple //c Plus",			GAME_NOT_WORKING )
