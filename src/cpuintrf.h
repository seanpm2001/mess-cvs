#ifndef CPUINTRF_H
#define CPUINTRF_H

#include "osd_cpu.h"
#include "memory.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif


struct MachineCPU
{
	int cpu_type;	/* see #defines below. */
	int cpu_clock;	/* in Hertz */
	const void *memory_read;	/* struct Memory_ReadAddress */
	const void *memory_write;	/* struct Memory_WriteAddress */
	const void *port_read;
	const void *port_write;
	int (*vblank_interrupt)(void);
	int vblank_interrupts_per_frame;	/* usually 1 */
/* use this for interrupts which are not tied to vblank 	*/
/* usually frequency in Hz, but if you need 				*/
/* greater precision you can give the period in nanoseconds */
	int (*timed_interrupt)(void);
	int timed_interrupts_per_second;
/* pointer to a parameter to pass to the CPU cores reset function */
	void *reset_param;
};

/* the following list is automatically generated by makelist.pl - don't edit manually! */
enum
{
	CPU_DUMMY,
#if (HAS_Z80)
	CPU_Z80,
#endif
#if (HAS_Z80GB)
	CPU_Z80GB,
#endif
#if (HAS_CDP1802)
	CPU_CDP1802,
#endif
#if (HAS_8080)
	CPU_8080,
#endif
#if (HAS_8085A)
	CPU_8085A,
#endif
#if (HAS_M6502)
	CPU_M6502,
#endif
#if (HAS_M65C02)
	CPU_M65C02,
#endif
#if (HAS_M65SC02)
	CPU_M65SC02,
#endif
#if (HAS_M65CE02)
	CPU_M65CE02,
#endif
#if (HAS_M6509)
	CPU_M6509,
#endif
#if (HAS_M6510)
	CPU_M6510,
#endif
#if (HAS_M6510T)
	CPU_M6510T,
#endif
#if (HAS_M7501)
	CPU_M7501,
#endif
#if (HAS_M8502)
	CPU_M8502,
#endif
#if (HAS_N2A03)
	CPU_N2A03,
#endif
#if (HAS_M4510)
	CPU_M4510,
#endif
#if (HAS_H6280)
	CPU_H6280,
#endif
#if (HAS_I86)
	CPU_I86,
#endif
#if (HAS_I88)
	CPU_I88,
#endif
#if (HAS_I186)
	CPU_I186,
#endif
#if (HAS_I188)
	CPU_I188,
#endif
#if (HAS_I286)
	CPU_I286,
#endif
#if (HAS_V20)
	CPU_V20,
#endif
#if (HAS_V30)
	CPU_V30,
#endif
#if (HAS_V33)
	CPU_V33,
#endif
#if (HAS_I8035)
	CPU_I8035,
#endif
#if (HAS_I8039)
	CPU_I8039,
#endif
#if (HAS_I8048)
	CPU_I8048,
#endif
#if (HAS_N7751)
	CPU_N7751,
#endif
#if (HAS_I8X41)
	CPU_I8X41,
#endif
#if (HAS_M6800)
	CPU_M6800,
#endif
#if (HAS_M6801)
	CPU_M6801,
#endif
#if (HAS_M6802)
	CPU_M6802,
#endif
#if (HAS_M6803)
	CPU_M6803,
#endif
#if (HAS_M6808)
	CPU_M6808,
#endif
#if (HAS_HD63701)
	CPU_HD63701,
#endif
#if (HAS_NSC8105)
	CPU_NSC8105,
#endif
#if (HAS_M6805)
	CPU_M6805,
#endif
#if (HAS_M68705)
	CPU_M68705,
#endif
#if (HAS_HD63705)
	CPU_HD63705,
#endif
#if (HAS_HD6309)
	CPU_HD6309,
#endif
#if (HAS_M6809)
	CPU_M6809,
#endif
#if (HAS_KONAMI)
	CPU_KONAMI,
#endif
#if (HAS_M68000)
	CPU_M68000,
#endif
#if (HAS_M68010)
	CPU_M68010,
#endif
#if (HAS_M68EC020)
	CPU_M68EC020,
#endif
#if (HAS_M68020)
	CPU_M68020,
#endif
#if (HAS_T11)
	CPU_T11,
#endif
#if (HAS_S2650)
	CPU_S2650,
#endif
#if (HAS_F8)
	CPU_F8,
#endif
#if (HAS_CP1600)
	CPU_CP1600,
#endif
#if (HAS_TMS34010)
	CPU_TMS34010,
#endif
#if (HAS_TMS34020)
	CPU_TMS34020,
#endif
#if (HAS_TMS9900)
	CPU_TMS9900,
#endif
#if (HAS_TMS9940)
	CPU_TMS9940,
#endif
#if (HAS_TMS9980)
	CPU_TMS9980,
#endif
#if (HAS_TMS9985)
	CPU_TMS9985,
#endif
#if (HAS_TMS9989)
	CPU_TMS9989,
#endif
#if (HAS_TMS9995)
	CPU_TMS9995,
#endif
#if (HAS_TMS99105A)
	CPU_TMS99105A,
#endif
#if (HAS_TMS99110A)
	CPU_TMS99110A,
#endif
#if (HAS_Z8000)
	CPU_Z8000,
#endif
#if (HAS_TMS320C10)
	CPU_TMS320C10,
#endif
#if (HAS_CCPU)
	CPU_CCPU,
#endif
#if (HAS_PDP1)
	CPU_PDP1,
#endif
#if (HAS_ADSP2100)
	CPU_ADSP2100,
#endif
#if (HAS_ADSP2105)
	CPU_ADSP2105,
#endif
#if (HAS_PSXCPU)
	CPU_PSXCPU,
#endif
#if (HAS_SH2)
	CPU_SH2,
#endif
#if (HAS_SPC700)
	CPU_SPC700,
#endif
#if (HAS_ASAP)
	CPU_ASAP,
#endif
#if (HAS_LH5801)
	CPU_LH5801,
#endif
#if (HAS_SATURN)
	CPU_SATURN,
#endif
#if (HAS_UPD7810)
	CPU_UPD7810,
#endif

#ifdef MESS
#if (HAS_APEXC)
	CPU_APEXC,
#endif
#if (HAS_ARM)
	CPU_ARM,
#endif
#if (HAS_G65816)
	CPU_G65816,
#endif
#if (HAS_SC61860)
	CPU_SC61860,
#endif
#endif
    CPU_COUNT
};

/* set this if the CPU is used as a slave for audio. It will not be emulated if */
/* sound is disabled, therefore speeding up a lot the emulation. */
#define CPU_AUDIO_CPU 0x8000

/* the Z80 can be wired to use 16 bit addressing for I/O ports */
#define CPU_16BIT_PORT 0x4000

#define CPU_FLAGS_MASK 0xff00



/* The old system is obsolete and no longer supported by the core */
#define NEW_INTERRUPT_SYSTEM	1

#define MAX_IRQ_LINES	8		/* maximum number of IRQ lines per CPU */

#define CLEAR_LINE		0		/* clear (a fired, held or pulsed) line */
#define ASSERT_LINE 	1		/* assert an interrupt immediately */
#define HOLD_LINE		2		/* hold interrupt line until enable is true */
#define PULSE_LINE		3		/* pulse interrupt line for one instruction */

#define MAX_REGS		128 	/* maximum number of register of any CPU */

/* Values passed to the cpu_info function of a core to retrieve information */
enum {
	CPU_INFO_REG,
	CPU_INFO_FLAGS=MAX_REGS,
	CPU_INFO_NAME,
	CPU_INFO_FAMILY,
	CPU_INFO_VERSION,
	CPU_INFO_FILE,
	CPU_INFO_CREDITS,
	CPU_INFO_REG_LAYOUT,
	CPU_INFO_WIN_LAYOUT
};

#define CPU_IS_LE		0	/* emulated CPU is little endian */
#define CPU_IS_BE		1	/* emulated CPU is big endian */

/*
 * This value is passed to cpu_get_reg to retrieve the previous
 * program counter value, ie. before a CPU emulation started
 * to fetch opcodes and arguments for the current instrution.
 */
#define REG_PREVIOUSPC	-1

/*
 * This value is passed to cpu_get_reg/cpu_set_reg, instead of one of
 * the names from the enum a CPU core defines for it's registers,
 * to get or set the contents of the memory pointed to by a stack pointer.
 * You can specify the n'th element on the stack by (REG_SP_CONTENTS-n),
 * ie. lower negative values. The actual element size (UINT16 or UINT32)
 * depends on the CPU core.
 * This is also used to replace the cpu_geturnpc() function.
 */
#define REG_SP_CONTENTS -2



/* ASG 971222 -- added this generic structure */
struct cpu_interface
{
	unsigned cpu_num;
	void (*init)(void);
	void (*reset)(void *param);
	void (*exit)(void);
	int (*execute)(int cycles);
	void (*burn)(int cycles);
	unsigned (*get_context)(void *reg);
	void (*set_context)(void *reg);
	void *(*get_cycle_table)(int which);
	void (*set_cycle_table)(int which, void *new_table);
	unsigned (*get_pc)(void);
	void (*set_pc)(unsigned val);
	unsigned (*get_sp)(void);
	void (*set_sp)(unsigned val);
	unsigned (*get_reg)(int regnum);
	void (*set_reg)(int regnum, unsigned val);
	void (*set_nmi_line)(int linestate);
	void (*set_irq_line)(int irqline, int linestate);
	void (*set_irq_callback)(int(*callback)(int irqline));
	void (*internal_interrupt)(int type);
	const char* (*cpu_info)(void *context,int regnum);
	unsigned (*cpu_dasm)(char *buffer,unsigned pc);
	unsigned num_irqs;
	int default_vector;
	int *icount;
	double overclock;
	int no_int, irq_int, nmi_int;
	int databus_width;
	mem_read_handler memory_read;
	mem_write_handler memory_write;
	mem_read_handler internal_read;
	mem_write_handler internal_write;
	offs_t pgm_memory_base;
	void (*set_op_base)(offs_t pc);
	int address_shift;
	unsigned address_bits, endianess, align_unit, max_inst_len;
};

extern struct cpu_interface cpuintf[];

void cpu_init(void);
void cpu_run(void);

/* optional watchdog */
WRITE_HANDLER( watchdog_reset_w );
READ_HANDLER( watchdog_reset_r );
WRITE16_HANDLER( watchdog_reset16_w );
READ16_HANDLER( watchdog_reset16_r );
/* Use this function to reset the machine */
void machine_reset(void);
/* Use this function to reset a single CPU */
void cpu_set_reset_line(int cpu,int state);
/* Use this function to halt a single CPU */
void cpu_set_halt_line(int cpu,int state);

/* This function returns CPUNUM current status (running or halted) */
int cpu_getstatus(int cpunum);
int cpu_gettotalcpu(void);
int cpu_getactivecpu(void);
void cpu_setactivecpu(int cpunum);

/* Returns the current program counter */
offs_t cpu_get_pc(void);
offs_t cpu_get_pc_byte(void);
/* Set the current program counter */
void cpu_set_pc(offs_t val);

/* Returns the current stack pointer */
offs_t cpu_get_sp(void);
/* Set the current stack pointer */
void cpu_set_sp(offs_t val);

/* Get the active CPUs context and return it's size */
unsigned cpu_get_context(void *context);
/* Set the active CPUs context */
void cpu_set_context(void *context);

/* Get a pointer to the active CPUs cycle count lookup table */
void *cpu_get_cycle_table(int which);
/* Override a pointer to the active CPUs cycle count lookup table */
void cpu_set_cycle_tbl(int which, void *new_table);

/* Returns a specific register value (mamedbg) */
unsigned cpu_get_reg(int regnum);
/* Sets a specific register value (mamedbg) */
void cpu_set_reg(int regnum, unsigned val);

/* Returns previous pc (start of opcode causing read/write) */
/* int cpu_getpreviouspc(void); */
#define cpu_getpreviouspc() cpu_get_reg(REG_PREVIOUSPC)

/* Returns the return address from the top of the stack (Z80 only) */
/* int cpu_getreturnpc(void); */
/* This can now be handled with a generic function */
#define cpu_geturnpc() cpu_get_reg(REG_SP_CONTENTS)

int cycles_currently_ran(void);
int cycles_left_to_run(void);
void cpu_set_op_base(unsigned val);

/* Returns the number of CPU cycles which take place in one video frame */
int cpu_gettotalcycles(void);
/* Returns the number of CPU cycles before the next interrupt handler call */
int cpu_geticount(void);
/* Returns the number of CPU cycles before the end of the current video frame */
int cpu_getfcount(void);
/* Returns the number of CPU cycles in one video frame */
int cpu_getfperiod(void);
/* Scales a given value by the ratio of fcount / fperiod */
int cpu_scalebyfcount(int value);
/* Returns the current scanline number */
int cpu_getscanline(void);
/* Returns the amount of time until a given scanline */
double cpu_getscanlinetime(int scanline);
/* Returns the duration of a single scanline */
double cpu_getscanlineperiod(void);
/* Returns the duration of a single scanline in cycles */
int cpu_getscanlinecycles(void);
/* Returns the number of cycles since the beginning of this frame */
int cpu_getcurrentcycles(void);
/* Returns the current horizontal beam position in pixels */
int cpu_gethorzbeampos(void);
/*
  Returns the number of times the interrupt handler will be called before
  the end of the current video frame. This is can be useful to interrupt
  handlers to synchronize their operation. If you call this from outside
  an interrupt handler, add 1 to the result, i.e. if it returns 0, it means
  that the interrupt handler will be called once.
*/
int cpu_getiloops(void);

/* Returns the current VBLANK state */
int cpu_getvblank(void);

/* Returns the number of the video frame we are currently playing */
int cpu_getcurrentframe(void);


/* generate a trigger after a specific period of time */
void cpu_triggertime (double duration, int trigger);
/* generate a trigger now */
void cpu_trigger (int trigger);

/* burn CPU cycles until a timer trigger */
void cpu_spinuntil_trigger (int trigger);
/* burn CPU cycles until the next interrupt */
void cpu_spinuntil_int (void);
/* burn CPU cycles until our timeslice is up */
void cpu_spin (void);
/* burn CPU cycles for a specific period of time */
void cpu_spinuntil_time (double duration);

/* yield our timeslice for a specific period of time */
void cpu_yielduntil_trigger (int trigger);
/* yield our timeslice until the next interrupt */
void cpu_yielduntil_int (void);
/* yield our current timeslice */
void cpu_yield (void);
/* yield our timeslice for a specific period of time */
void cpu_yielduntil_time (double duration);

/* set the NMI line state for a CPU, normally use PULSE_LINE */
void cpu_set_nmi_line(int cpunum, int state);
/* set the IRQ line state for a specific irq line of a CPU */
/* normally use state HOLD_LINE, irqline 0 for first IRQ type of a cpu */
void cpu_set_irq_line(int cpunum, int irqline, int state);
/* this is to be called by CPU cores only! */
void cpu_generate_internal_interrupt(int cpunum, int type);
/* set the vector to be returned during a CPU's interrupt acknowledge cycle */
void cpu_irq_line_vector_w(int cpunum, int irqline, int vector);

/* use this function to install a driver callback for IRQ acknowledge */
void cpu_set_irq_callback(int cpunum, int (*callback)(int irqline));

/* use these in your write memory/port handles to set an IRQ vector */
/* offset corresponds to the irq line number here */
WRITE_HANDLER( cpu_0_irq_line_vector_w );
WRITE_HANDLER( cpu_1_irq_line_vector_w );
WRITE_HANDLER( cpu_2_irq_line_vector_w );
WRITE_HANDLER( cpu_3_irq_line_vector_w );
WRITE_HANDLER( cpu_4_irq_line_vector_w );
WRITE_HANDLER( cpu_5_irq_line_vector_w );
WRITE_HANDLER( cpu_6_irq_line_vector_w );
WRITE_HANDLER( cpu_7_irq_line_vector_w );

/* Obsolete functions: avoid to use them in new drivers if possible. */

/* cause an interrupt on a CPU */
void cpu_cause_interrupt(int cpu,int type);
void cpu_clear_pending_interrupts(int cpu);
WRITE_HANDLER( interrupt_enable_w );
WRITE_HANDLER( interrupt_vector_w );
int interrupt(void);
int nmi_interrupt(void);
#if (HAS_M68000 || HAS_M68010 || HAS_M68020 || HAS_M68EC020)
int m68_level1_irq(void);
int m68_level2_irq(void);
int m68_level3_irq(void);
int m68_level4_irq(void);
int m68_level5_irq(void);
int m68_level6_irq(void);
int m68_level7_irq(void);
#endif
int ignore_interrupt(void);

/* CPU context access */
void* cpu_getcontext (int _activecpu);
int cpu_is_saving_context(int _activecpu);

/***************************************************************************
 * Get information for the currently active CPU
 * cputype is a value from the CPU enum in driver.h
 ***************************************************************************/
/* Return number of address bits */
unsigned cpu_address_bits(void);
/* Return address mask */
unsigned cpu_address_mask(void);
/* Return address shift factor (TMS34010 bit addressing mode) */
int cpu_address_shift(void);
/* Return endianess of the emulated CPU (CPU_IS_LE or CPU_IS_BE) */
unsigned cpu_endianess(void);
/* Return opcode align unit (1 byte, 2 word, 4 dword) */
unsigned cpu_align_unit(void);
/* Return maximum instruction length */
unsigned cpu_max_inst_len(void);

/* Return name of the active CPU */
const char *cpu_name(void);
/* Return family name of the active CPU */
const char *cpu_core_family(void);
/* Return core version of the active CPU */
const char *cpu_core_version(void);
/* Return core filename of the active CPU */
const char *cpu_core_file(void);
/* Return credits info for of the active CPU */
const char *cpu_core_credits(void);
/* Return register layout definition for the active CPU */
const char *cpu_reg_layout(void);
/* Return (debugger) window layout definition for the active CPU */
const char *cpu_win_layout(void);

/* Disassemble an instruction at PC into the given buffer */
unsigned cpu_dasm(char *buffer, unsigned pc);
/* Return a string describing the currently set flag (status) bits of the active CPU */
const char *cpu_flags(void);
/* Return a string with a register name and hex value for the active CPU */
/* regnum is a value defined in the CPU cores header files */
const char *cpu_dump_reg(int regnum);
/* Return a string describing the active CPUs current state */
const char *cpu_dump_state(void);

/***************************************************************************
 * Get information for a specific CPU type
 * cputype is a value from the CPU enum in driver.h
 ***************************************************************************/
/* Return address shift factor */
/* TMS320C10 -1: word addressing mode, TMS34010 3: bit addressing mode */
int cputype_address_shift(int cputype);
/* Return number of address bits */
unsigned cputype_address_bits(int cputype);
/* Return address mask */
unsigned cputype_address_mask(int cputype);
/* Return endianess of the emulated CPU (CPU_IS_LE or CPU_IS_BE) */
unsigned cputype_endianess(int cputype);
/* Return data bus width of the emulated CPU */
unsigned cputype_databus_width(int cputype);
/* Return opcode align unit (1 byte, 2 word, 4 dword) */
unsigned cputype_align_unit(int cputype);
/* Return maximum instruction length */
unsigned cputype_max_inst_len(int cputype);

/* Return name of the CPU */
const char *cputype_name(int cputype);
/* Return family name of the CPU */
const char *cputype_core_family(int cputype);
/* Return core version number of the CPU */
const char *cputype_core_version(int cputype);
/* Return core filename of the CPU */
const char *cputype_core_file(int cputype);
/* Return credits for the CPU core */
const char *cputype_core_credits(int cputype);
/* Return register layout definition for the CPU core */
const char *cputype_reg_layout(int cputype);
/* Return (debugger) window layout definition for the CPU core */
const char *cputype_win_layout(int cputype);

/***************************************************************************
 * Get (or set) information for a numbered CPU of the running machine
 * cpunum is a value between 0 and cpu_gettotalcpu() - 1
 ***************************************************************************/
/* Return number of address bits */
unsigned cpunum_address_bits(int cpunum);
/* Return address mask */
unsigned cpunum_address_mask(int cpunum);
/* Return endianess of the emulated CPU (CPU_LSB_FIRST or CPU_MSB_FIRST) */
unsigned cpunum_endianess(int cpunum);
/* Return data bus width of the emulated CPU */
unsigned cpunum_databus_width(int cpunum);
/* Return opcode align unit (1 byte, 2 word, 4 dword) */
unsigned cpunum_align_unit(int cpunum);
/* Return maximum instruction length */
unsigned cpunum_max_inst_len(int cpunum);

/* Get a register value for the specified CPU number of the running machine */
unsigned cpunum_get_reg(int cpunum, int regnum);
/* Set a register value for the specified CPU number of the running machine */
void cpunum_set_reg(int cpunum, int regnum, unsigned val);

/* Return (debugger) register layout definition for the CPU core */
const char *cpunum_reg_layout(int cpunum);
/* Return (debugger) window layout definition for the CPU core */
const char *cpunum_win_layout(int cpunum);

unsigned cpunum_dasm(int cpunum,char *buffer,unsigned pc);
/* Return a string describing the currently set flag (status) bits of the CPU */
const char *cpunum_flags(int cpunum);
/* Return a string with a register name and value */
/* regnum is a value defined in the CPU cores header files */
const char *cpunum_dump_reg(int cpunum, int regnum);
/* Return a string describing the CPUs current state */
const char *cpunum_dump_state(int cpunum);
/* Return a name for the specified cpu number */
const char *cpunum_name(int cpunum);
/* Return a family name for the specified cpu number */
const char *cpunum_core_family(int cpunum);
/* Return a version for the specified cpu number */
const char *cpunum_core_version(int cpunum);
/* Return a the source filename for the specified cpu number */
const char *cpunum_core_file(int cpunum);
/* Return a the credits for the specified cpu number */
const char *cpunum_core_credits(int cpunum);

/* Dump all of the running machines CPUs state to stderr */
void cpu_dump_states(void);

/* Load or save the game state */
enum {
	LOADSAVE_NONE = 0,
	LOADSAVE_SAVE = 1,
	LOADSAVE_LOAD = 2
};
void cpu_loadsave_schedule(int type, char id);
void cpu_loadsave_reset(void);

/* daisy-chain link */
typedef struct {
	void (*reset)(int); 			/* reset callback	  */
	int  (*interrupt_entry)(int);	/* entry callback	  */
	void (*interrupt_reti)(int);	/* reti callback	  */
	int irq_param;					/* callback paramater */
}	Z80_DaisyChain;

#define Z80_MAXDAISY	4		/* maximum of daisy chan device */

#define Z80_INT_REQ 	0x01	/* interrupt request mask		*/
#define Z80_INT_IEO 	0x02	/* interrupt disable mask(IEO)	*/

#define Z80_VECTOR(device,state) (((device)<<8)|(state))

#ifdef __cplusplus
}
#endif

#endif	/* CPUINTRF_H */

