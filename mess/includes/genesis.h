READ16_HANDLER( genesis_68000_z80_read );
READ16_HANDLER ( genesis_68000_io_r );
READ16_HANDLER( genesis_68000_z80_busreq_r );
READ16_HANDLER( genesis_68000_vdp_r );
READ_HANDLER ( genesis_banked_68k_r );
WRITE_HANDLER( genesis_z80_vdp_w );

WRITE16_HANDLER( genesis_68000_z80_write );
WRITE16_HANDLER ( genesis_68000_io_w );
WRITE16_HANDLER( genesis_68000_z80_busreq_w );
WRITE16_HANDLER ( genesis_68000_z80_reset_w );
WRITE16_HANDLER( genesis_68000_vdp_w );
WRITE_HANDLER ( z80_ym2612_w );
WRITE_HANDLER( genesis_z80_bank_sel_w );

VIDEO_START(genesis);
VIDEO_UPDATE(genesis);
MACHINE_INIT ( genesis );
INTERRUPT_GEN( genesis_interrupt );
DRIVER_INIT ( genesis );

extern data16_t *genesis_cartridge;