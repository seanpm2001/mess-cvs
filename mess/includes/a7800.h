/* vidhrdw/a7800.c */
extern VIDEO_START( a7800 );
extern VIDEO_UPDATE( a7800 );
extern void a7800_interrupt(void);
extern READ_HANDLER  ( a7800_MARIA_r);
extern WRITE_HANDLER ( a7800_MARIA_w );


/* machine/a7800.c */
extern unsigned char *a7800_ram;
extern unsigned char *a7800_cartridge_rom;
extern MACHINE_INIT( a7800 );
extern UINT32 a7800_partialcrc(const unsigned char *,unsigned int);
extern int a7800_init_cart (int id);
extern void a7800_exit_rom (int id);
extern READ_HANDLER  ( a7800_TIA_r );
extern WRITE_HANDLER ( a7800_TIA_w );
extern READ_HANDLER  ( a7800_RIOT_r );
extern WRITE_HANDLER ( a7800_RIOT_w );
extern READ_HANDLER  ( a7800_MAINRAM_r );
extern WRITE_HANDLER ( a7800_MAINRAM_w );
extern READ_HANDLER  ( a7800_RAM0_r );
extern WRITE_HANDLER ( a7800_RAM0_w );
extern READ_HANDLER  ( a7800_RAM1_r );
extern WRITE_HANDLER ( a7800_RAM1_w );
extern WRITE_HANDLER ( a7800_cart_w );

#ifdef RUNTIME_LOADER
# ifdef __cplusplus
	extern "C" void a7800_runtime_loader_init(void);
# else
	extern void a7800_runtime_loader_init(void);
# endif
#endif
