/* machine/ti85.c */

extern UINT8 LCD_memory;
extern int LCD_status;
extern void ti85_init_machine (void);
extern void ti85_stop_machine (void);
extern READ_HANDLER( ti85_port_0000_r);
extern READ_HANDLER( ti85_port_0001_r);
extern READ_HANDLER( ti85_port_0002_r);
extern READ_HANDLER( ti85_port_0003_r);
extern READ_HANDLER( ti85_port_0004_r);
extern READ_HANDLER( ti85_port_0005_r);
extern READ_HANDLER( ti85_port_0006_r);
extern READ_HANDLER( ti85_port_0007_r);
extern WRITE_HANDLER( ti85_port_0000_w);
extern WRITE_HANDLER( ti85_port_0001_w);
extern WRITE_HANDLER( ti85_port_0002_w);
extern WRITE_HANDLER( ti85_port_0003_w);
extern WRITE_HANDLER( ti85_port_0004_w);
extern WRITE_HANDLER( ti85_port_0005_w);
extern WRITE_HANDLER( ti85_port_0006_w);
extern WRITE_HANDLER( ti85_port_0007_w);

/* vidhrdw/ti85.c */

extern int ti85_vh_start (void);
extern void ti85_vh_stop (void);
extern void ti85_vh_screenrefresh (struct osd_bitmap *bitmap, int full_refresh);
extern WRITE_HANDLER( ti85_vh_charram_w );
extern unsigned char *ti85_charram;
extern size_t ti85_charram_size;

												
