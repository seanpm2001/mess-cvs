#include "keyboard.h"

/* translation table from X11 codes to raw keyboard scan codes */
/* the idea is make a re-definable lookup table, instead a long case switch */

/* following code is taken from keysmdef.h */

/******************************************************************/
/* $XConsortium: keysymdef.h /main/24 1996/02/02 14:28:10 kaleb $ */
/******************************************************************/

/* XK_VoidSymbol	0xFFFFFF	*/ /* void symbol */

static int extended_code_table[512] = {
/*                      		0xFE00  */ KEY_NONE,
/* XK_ISO_Lock				0xFE01  */ KEY_NONE,
/* XK_ISO_Level2_Latch			0xFE02  */ KEY_NONE,
/* XK_ISO_Level3_Shift			0xFE03  */ KEY_NONE,
/* XK_ISO_Level3_Latch			0xFE04  */ KEY_NONE,
/* XK_ISO_Level3_Lock			0xFE05  */ KEY_NONE,
/* XK_ISO_Group_Latch			0xFE06  */ KEY_NONE,
/* XK_ISO_Group_Lock			0xFE07  */ KEY_NONE,
/* XK_ISO_Next_Group			0xFE08  */ KEY_NONE,
/* XK_ISO_Next_Group_Lock		0xFE09  */ KEY_NONE,
/* XK_ISO_Prev_Group			0xFE0A  */ KEY_NONE,
/* XK_ISO_Prev_Group_Lock		0xFE0B  */ KEY_NONE,
/* XK_ISO_First_Group			0xFE0C  */ KEY_NONE,
/* XK_ISO_First_Group_Lock		0xFE0D  */ KEY_NONE,
/* XK_ISO_Last_Group			0xFE0E  */ KEY_NONE,
/* XK_ISO_Last_Group_Lock		0xFE0F  */ KEY_NONE,
/*					0xFE10  */ KEY_NONE,
/*					0xFE11  */ KEY_NONE,
/*					0xFE12  */ KEY_NONE,
/*					0xFE13  */ KEY_NONE,
/*					0xFE14  */ KEY_NONE,
/*					0xFE15  */ KEY_NONE,
/*					0xFE16  */ KEY_NONE,
/*					0xFE17  */ KEY_NONE,
/*					0xFE18  */ KEY_NONE,
/*					0xFE19  */ KEY_NONE,
/*					0xFE1A  */ KEY_NONE,
/*					0xFE1B  */ KEY_NONE,
/*					0xFE1C  */ KEY_NONE,
/*					0xFE1D  */ KEY_NONE,
/*					0xFE1E  */ KEY_NONE,
/*					0xFE1F  */ KEY_NONE,
/* XK_ISO_Left_Tab			0xFE20  */ KEY_TAB,
/* XK_ISO_Move_Line_Up			0xFE21  */ KEY_UP,
/* XK_ISO_Move_Line_Down		0xFE22  */ KEY_DOWN,
/* XK_ISO_Partial_Line_Up		0xFE23  */ KEY_NONE,
/* XK_ISO_Partial_Line_Down		0xFE24  */ KEY_NONE,
/* XK_ISO_Partial_Space_Left		0xFE25  */ KEY_NONE,
/* XK_ISO_Partial_Space_Right		0xFE26  */ KEY_NONE,
/* XK_ISO_Set_Margin_Left		0xFE27  */ KEY_NONE,
/* XK_ISO_Set_Margin_Right		0xFE28  */ KEY_NONE,
/* XK_ISO_Release_Margin_Left		0xFE29  */ KEY_NONE,
/* XK_ISO_Release_Margin_Right		0xFE2A  */ KEY_NONE,
/* XK_ISO_Release_Both_Margins		0xFE2B  */ KEY_NONE,
/* XK_ISO_Fast_Cursor_Left		0xFE2C  */ KEY_LEFT,
/* XK_ISO_Fast_Cursor_Right		0xFE2D  */ KEY_RIGHT,
/* XK_ISO_Fast_Cursor_Up		0xFE2E  */ KEY_UP,
/* XK_ISO_Fast_Cursor_Down		0xFE2F  */ KEY_DOWN,
/* XK_ISO_Continuous_Underline		0xFE30  */ KEY_MINUS,
/* XK_ISO_Discontinuous_Underline	0xFE31  */ KEY_MINUS,
/* XK_ISO_Emphasize			0xFE32  */ KEY_NONE,
/* XK_ISO_Center_Object			0xFE33  */ KEY_NONE,
/* XK_ISO_Enter				0xFE34  */ KEY_ENTER,
/*					0xFE35  */ KEY_NONE,
/*					0xFE36  */ KEY_NONE,
/*					0xFE37  */ KEY_NONE,
/*					0xFE38  */ KEY_NONE,
/*					0xFE39  */ KEY_NONE,
/*					0xFE3A  */ KEY_NONE,
/*					0xFE3B  */ KEY_NONE,
/*					0xFE3C  */ KEY_NONE,
/*					0xFE3D  */ KEY_NONE,
/*					0xFE3E  */ KEY_NONE,
/*					0xFE3F  */ KEY_NONE,
/*					0xFE40  */ KEY_NONE,
/*					0xFE41  */ KEY_NONE,
/*					0xFE42  */ KEY_NONE,
/*					0xFE43  */ KEY_NONE,
/*					0xFE44  */ KEY_NONE,
/*					0xFE45  */ KEY_NONE,
/*					0xFE46  */ KEY_NONE,
/*					0xFE47  */ KEY_NONE,
/*					0xFE48  */ KEY_NONE,
/*					0xFE49  */ KEY_NONE,
/*					0xFE4A  */ KEY_NONE,
/*					0xFE4B  */ KEY_NONE,
/*					0xFE4C  */ KEY_NONE,
/*					0xFE4D  */ KEY_NONE,
/*					0xFE4E  */ KEY_NONE,
/*					0xFE4F  */ KEY_NONE,
/* XK_dead_grave			0xFE50  */ KEY_TILDE,
/* XK_dead_acute			0xFE51  */ KEY_NONE,
/* XK_dead_circumflex			0xFE52  */ KEY_TILDE,
/* XK_dead_tilde			0xFE53  */ KEY_TILDE,
/* XK_dead_macron			0xFE54  */ KEY_NONE,
/* XK_dead_breve			0xFE55  */ KEY_NONE,
/* XK_dead_abovedot			0xFE56  */ KEY_NONE,
/* XK_dead_diaeresis			0xFE57  */ KEY_NONE,
/* XK_dead_abovering			0xFE58  */ KEY_NONE,
/* XK_dead_doubleacute			0xFE59  */ KEY_NONE,
/* XK_dead_caron			0xFE5A  */ KEY_NONE,
/* XK_dead_cedilla			0xFE5B  */ KEY_NONE,
/* XK_dead_ogonek			0xFE5C  */ KEY_NONE,
/* XK_dead_iota				0xFE5D  */ KEY_NONE,
/* XK_dead_voiced_sound			0xFE5E  */ KEY_NONE,
/* XK_dead_semivoiced_sound		0xFE5F  */ KEY_NONE,
/* XK_dead_belowdot			0xFE60  */ KEY_NONE,
/*					0xFE61  */ KEY_NONE,
/*					0xFE62  */ KEY_NONE,
/*					0xFE63  */ KEY_NONE,
/*					0xFE64  */ KEY_NONE,
/*					0xFE65  */ KEY_NONE,
/*					0xFE66  */ KEY_NONE,
/*					0xFE67  */ KEY_NONE,
/*					0xFE68  */ KEY_NONE,
/*					0xFE69  */ KEY_NONE,
/*					0xFE6A  */ KEY_NONE,
/*					0xFE6B  */ KEY_NONE,
/*					0xFE6C  */ KEY_NONE,
/*					0xFE6D  */ KEY_NONE,
/*					0xFE6E  */ KEY_NONE,
/*					0xFE6F  */ KEY_NONE,
/* XK_AccessX_Enable			0xFE70  */ KEY_NONE,
/* XK_AccessX_Feedback_Enable		0xFE71  */ KEY_NONE,
/* XK_RepeatKeys_Enable			0xFE72  */ KEY_NONE,
/* XK_SlowKeys_Enable			0xFE73  */ KEY_NONE,
/* XK_BounceKeys_Enable			0xFE74  */ KEY_NONE,
/* XK_StickyKeys_Enable			0xFE75  */ KEY_NONE,
/* XK_MouseKeys_Enable			0xFE76  */ KEY_NONE,
/* XK_MouseKeys_Accel_Enable		0xFE77  */ KEY_NONE,
/* XK_Overlay1_Enable			0xFE78  */ KEY_NONE,
/* XK_Overlay2_Enable			0xFE79  */ KEY_NONE,
/* XK_AudibleBell_Enable		0xFE7A  */ KEY_NONE,
/*					0xFE7B  */ KEY_NONE,
/*					0xFE7C  */ KEY_NONE,
/*					0xFE7D  */ KEY_NONE,
/*					0xFE7E  */ KEY_NONE,
/*					0xFE7F  */ KEY_NONE,
/*					0xFE80  */ KEY_NONE,
/*					0xFE81  */ KEY_NONE,
/*					0xFE82  */ KEY_NONE,
/*					0xFE83  */ KEY_NONE,
/*					0xFE84  */ KEY_NONE,
/*					0xFE85  */ KEY_NONE,
/*					0xFE86  */ KEY_NONE,
/*					0xFE87  */ KEY_NONE,
/*					0xFE88  */ KEY_NONE,
/*					0xFE89  */ KEY_NONE,
/*					0xFE8A  */ KEY_NONE,
/*					0xFE8B  */ KEY_NONE,
/*					0xFE8C  */ KEY_NONE,
/*					0xFE8D  */ KEY_NONE,
/*					0xFE8E  */ KEY_NONE,
/*					0xFE8F  */ KEY_NONE,
/*					0xFE90  */ KEY_NONE,
/*					0xFE91  */ KEY_NONE,
/*					0xFE92  */ KEY_NONE,
/*					0xFE93  */ KEY_NONE,
/*					0xFE94  */ KEY_NONE,
/*					0xFE95  */ KEY_NONE,
/*					0xFE96  */ KEY_NONE,
/*					0xFE97  */ KEY_NONE,
/*					0xFE98  */ KEY_NONE,
/*					0xFE99  */ KEY_NONE,
/*					0xFE9A  */ KEY_NONE,
/*					0xFE9B  */ KEY_NONE,
/*					0xFE9C  */ KEY_NONE,
/*					0xFE9D  */ KEY_NONE,
/*					0xFE9E  */ KEY_NONE,
/*					0xFE9F  */ KEY_NONE,
/*					0xFEA0  */ KEY_NONE,
/*					0xFEA1  */ KEY_NONE,
/*					0xFEA2  */ KEY_NONE,
/*					0xFEA3  */ KEY_NONE,
/*					0xFEA4  */ KEY_NONE,
/*					0xFEA5  */ KEY_NONE,
/*					0xFEA6  */ KEY_NONE,
/*					0xFEA7  */ KEY_NONE,
/*					0xFEA8  */ KEY_NONE,
/*					0xFEA9  */ KEY_NONE,
/*					0xFEAA  */ KEY_NONE,
/*					0xFEAB  */ KEY_NONE,
/*					0xFEAC  */ KEY_NONE,
/*					0xFEAD  */ KEY_NONE,
/*					0xFEAE  */ KEY_NONE,
/*					0xFEAF  */ KEY_NONE,
/*					0xFEB0  */ KEY_NONE,
/*					0xFEB1  */ KEY_NONE,
/*					0xFEB2  */ KEY_NONE,
/*					0xFEB3  */ KEY_NONE,
/*					0xFEB4  */ KEY_NONE,
/*					0xFEB5  */ KEY_NONE,
/*					0xFEB6  */ KEY_NONE,
/*					0xFEB7  */ KEY_NONE,
/*					0xFEB8  */ KEY_NONE,
/*					0xFEB9  */ KEY_NONE,
/*					0xFEBA  */ KEY_NONE,
/*					0xFEBB  */ KEY_NONE,
/*					0xFEBC  */ KEY_NONE,
/*					0xFEBD  */ KEY_NONE,
/*					0xFEBE  */ KEY_NONE,
/*					0xFEBF  */ KEY_NONE,
/*					0xFEC0  */ KEY_NONE,
/*					0xFEC1  */ KEY_NONE,
/*					0xFEC2  */ KEY_NONE,
/*					0xFEC3  */ KEY_NONE,
/*					0xFEC4  */ KEY_NONE,
/*					0xFEC5  */ KEY_NONE,
/*					0xFEC6  */ KEY_NONE,
/*					0xFEC7  */ KEY_NONE,
/*					0xFEC8  */ KEY_NONE,
/*					0xFEC9  */ KEY_NONE,
/*					0xFECA  */ KEY_NONE,
/*					0xFECB  */ KEY_NONE,
/*					0xFECC  */ KEY_NONE,
/*					0xFECD  */ KEY_NONE,
/*					0xFECE  */ KEY_NONE,
/*					0xFECF  */ KEY_NONE,
/* XK_First_Virtual_Screen		0xFED0  */ KEY_NONE,
/* XK_Prev_Virtual_Screen		0xFED1  */ KEY_NONE,
/* XK_Next_Virtual_Screen		0xFED2  */ KEY_NONE,
/*					0xFED3  */ KEY_NONE,
/* XK_Last_Virtual_Screen		0xFED4  */ KEY_NONE,
/* XK_Terminate_Server			0xFED5  */ KEY_NONE,
/*					0xFED6  */ KEY_NONE,
/*					0xFED7  */ KEY_NONE,
/*					0xFED8  */ KEY_NONE,
/*					0xFED9  */ KEY_NONE,
/*					0xFEDA  */ KEY_NONE,
/*					0xFEDB  */ KEY_NONE,
/*					0xFEDC  */ KEY_NONE,
/*					0xFEDD  */ KEY_NONE,
/*					0xFEDE  */ KEY_NONE,
/*					0xFEDF  */ KEY_NONE,
/* XK_Pointer_Left			0xFEE0  */ KEY_NONE,
/* XK_Pointer_Right			0xFEE1  */ KEY_NONE,
/* XK_Pointer_Up			0xFEE2  */ KEY_NONE,
/* XK_Pointer_Down			0xFEE3  */ KEY_NONE,
/* XK_Pointer_UpLeft			0xFEE4  */ KEY_NONE,
/* XK_Pointer_UpRight			0xFEE5  */ KEY_NONE,
/* XK_Pointer_DownLeft			0xFEE6  */ KEY_NONE,
/* XK_Pointer_DownRight			0xFEE7  */ KEY_NONE,
/* XK_Pointer_Button_Dflt		0xFEE8  */ KEY_NONE,
/* XK_Pointer_Button1			0xFEE9  */ KEY_NONE,
/* XK_Pointer_Button2			0xFEEA  */ KEY_NONE,
/* XK_Pointer_Button3			0xFEEB  */ KEY_NONE,
/* XK_Pointer_Button4			0xFEEC  */ KEY_NONE,
/* XK_Pointer_Button5			0xFEED  */ KEY_NONE,
/* XK_Pointer_DblClick_Dflt		0xFEEE  */ KEY_NONE,
/* XK_Pointer_DblClick1			0xFEEF  */ KEY_NONE,
/* XK_Pointer_DblClick2			0xFEF0  */ KEY_NONE,
/* XK_Pointer_DblClick3			0xFEF1  */ KEY_NONE,
/* XK_Pointer_DblClick4			0xFEF2  */ KEY_NONE,
/* XK_Pointer_DblClick5			0xFEF3  */ KEY_NONE,
/* XK_Pointer_Drag_Dflt			0xFEF4  */ KEY_NONE,
/* XK_Pointer_Drag1			0xFEF5  */ KEY_NONE,
/* XK_Pointer_Drag2			0xFEF6  */ KEY_NONE,
/* XK_Pointer_Drag3			0xFEF7  */ KEY_NONE,
/* XK_Pointer_Drag4			0xFEF8  */ KEY_NONE,
/* XK_Pointer_EnableKeys		0xFEF9  */ KEY_NONE,
/* XK_Pointer_Accelerate		0xFEFA  */ KEY_NONE,
/* XK_Pointer_DfltBtnNext		0xFEFB  */ KEY_NONE,
/* XK_Pointer_DfltBtnPrev		0xFEFC  */ KEY_NONE,
/* XK_Pointer_Drag5			0xFEFD  */ KEY_NONE,
/*					0xFEFE  */ KEY_NONE,
/*					0xFEFF  */ KEY_NONE,
/*					0xFF00  */ KEY_NONE,
/*					0xFF01  */ KEY_NONE,
/*					0xFF02  */ KEY_NONE,
/*					0xFF03  */ KEY_NONE,
/*					0xFF04  */ KEY_NONE,
/*					0xFF05  */ KEY_NONE,
/*					0xFF06  */ KEY_NONE,
/*					0xFF07  */ KEY_NONE,
/* XK_BackSpace				0xFF08	*/ KEY_BACKSPACE,
/* XK_Tab				0xFF09	*/ KEY_TAB,
/* XK_Linefeed				0xFF0A	*/ KEY_ENTER,
/* XK_Clear				0xFF0B  */ KEY_DEL,
/*					0xFF0C  */ KEY_NONE,
/* XK_Return				0xFF0D	*/ KEY_ENTER,
/*					0xFF0E  */ KEY_NONE,
/*					0xFF0F  */ KEY_NONE,
/*					0xFF10  */ KEY_F11,
/*					0xFF11  */ KEY_F12,
/*					0xFF12  */ KEY_NONE,
/* XK_Pause				0xFF13	*/ KEY_PAUSE,
/* XK_Scroll_Lock			0xFF14  */ KEY_SCRLOCK,
/* XK_Sys_Req				0xFF15  */ KEY_PRTSCR,
/*					0xFF16  */ KEY_NONE,
/*					0xFF17  */ KEY_NONE,
/*					0xFF18  */ KEY_NONE,
/*					0xFF19  */ KEY_NONE,
/*					0xFF1A  */ KEY_NONE,
/* XK_Escape				0xFF1B  */ KEY_ESC,
/*					0xFF1C  */ KEY_NONE,
/*					0xFF1D  */ KEY_NONE,
/*					0xFF1E  */ KEY_NONE,
/*					0xFF1F  */ KEY_NONE,
/* XK_Multi_key				0xFF20  */ KEY_NONE,
/* XK_Kanji				0xFF21	*/ KEY_NONE,
/* XK_Muhenkan				0xFF22  */ KEY_NONE,
/* XK_Henkan_Mode			0xFF23  */ KEY_NONE,
/* XK_Henkan				0xFF23  duplicated */
/* XK_Romaji				0xFF24  */ KEY_NONE,
/* XK_Hiragana				0xFF25  */ KEY_NONE,
/* XK_Katakana				0xFF26  */ KEY_NONE,
/* XK_Hiragana_Katakana			0xFF27  */ KEY_NONE,
/* XK_Zenkaku				0xFF28  */ KEY_NONE,
/* XK_Hankaku				0xFF29  */ KEY_NONE,
/* XK_Zenkaku_Hankaku			0xFF2A  */ KEY_NONE,
/* XK_Touroku				0xFF2B  */ KEY_NONE,
/* XK_Massyo				0xFF2C  */ KEY_NONE,
/* XK_Kana_Lock				0xFF2D  */ KEY_NONE,
/* XK_Kana_Shift			0xFF2E  */ KEY_NONE,
/* XK_Eisu_Shift			0xFF2F  */ KEY_NONE,
/* XK_Eisu_toggle			0xFF30  */ KEY_NONE,
/*					0xFF31  */ KEY_NONE,
/*					0xFF32  */ KEY_NONE,
/*					0xFF33  */ KEY_NONE,
/*					0xFF34  */ KEY_NONE,
/*					0xFF35  */ KEY_NONE,
/*					0xFF36  */ KEY_NONE,
/*					0xFF37  */ KEY_NONE,
/*					0xFF38  */ KEY_NONE,
/*					0xFF39  */ KEY_NONE,
/*					0xFF3A  */ KEY_NONE,
/*					0xFF3B  */ KEY_NONE,
/*					0xFF3C  */ KEY_NONE,
/*					0xFF3D  */ KEY_NONE,
/*					0xFF3E  */ KEY_NONE,
/*					0xFF3F  */ KEY_NONE,
/*					0xFF40  */ KEY_NONE,
/*					0xFF41  */ KEY_NONE,
/*					0xFF42  */ KEY_NONE,
/*					0xFF43  */ KEY_NONE,
/*					0xFF44  */ KEY_NONE,
/*					0xFF45  */ KEY_NONE,
/*					0xFF46  */ KEY_NONE,
/*					0xFF47  */ KEY_NONE,
/*					0xFF48  */ KEY_NONE,
/*					0xFF49  */ KEY_NONE,
/*					0xFF4A  */ KEY_NONE,
/*					0xFF4B  */ KEY_NONE,
/*					0xFF4C  */ KEY_NONE,
/*					0xFF4D  */ KEY_NONE,
/*					0xFF4E  */ KEY_NONE,
/*					0xFF4F  */ KEY_NONE,
/* XK_Home				0xFF50  */ KEY_HOME,
/* XK_Left				0xFF51	*/ KEY_LEFT,
/* XK_Up				0xFF52	*/ KEY_UP,
/* XK_Right				0xFF53	*/ KEY_RIGHT,
/* XK_Down				0xFF54	*/ KEY_DOWN,
/* XK_Prior				0xFF55	*/ KEY_PGUP,
/* XK_Page_Up				0xFF55  duplicated */
/* XK_Next				0xFF56	*/ KEY_PGDN,
/* XK_Page_Down				0xFF56  duplicated */
/* XK_End				0xFF57	*/ KEY_END,
/* XK_Begin				0xFF58	*/ KEY_HOME,
/*					0xFF59  */ KEY_NONE,
/*					0xFF5A  */ KEY_NONE,
/*					0xFF5B  */ KEY_NONE,
/*					0xFF5C  */ KEY_NONE,
/*					0xFF5D  */ KEY_NONE,
/*					0xFF5E  */ KEY_NONE,
/*					0xFF5F  */ KEY_NONE,
/* XK_Select				0xFF60	*/ KEY_ENTER_PAD,
/* XK_Print				0xFF61  */ KEY_PRTSCR,
/* XK_Execute				0xFF62	*/ KEY_ENTER,
/* XK_Insert				0xFF63	*/ KEY_INSERT,
/*					0xFF64	*/ KEY_NONE,
/* XK_Undo				0xFF65	*/ KEY_NONE,
/* XK_Redo				0xFF66	*/ KEY_NONE,
/* XK_Menu				0xFF67  */ KEY_MENU,
/* XK_Find				0xFF68	*/ KEY_NONE,
/* XK_Cancel				0xFF69	*/ KEY_ESC,
/* XK_Help				0xFF6A	*/ KEY_NONE,
/* XK_Break				0xFF6B  */ KEY_PAUSE,
/* 					0xFF6C  */ KEY_NONE,
/* 					0xFF6D  */ KEY_NONE,
/* 					0xFF6E  */ KEY_NONE,
/* 					0xFF6F  */ KEY_NONE,
/* 					0xFF70  */ KEY_NONE,
/* 					0xFF71  */ KEY_NONE,
/* 					0xFF72  */ KEY_NONE,
/* 					0xFF73  */ KEY_NONE,
/* 					0xFF74  */ KEY_NONE,
/* 					0xFF75  */ KEY_NONE,
/* 					0xFF76  */ KEY_NONE,
/* 					0xFF77  */ KEY_NONE,
/* 					0xFF78  */ KEY_NONE,
/* 					0xFF79  */ KEY_NONE,
/* 					0xFF7A  */ KEY_NONE,
/* 					0xFF7B  */ KEY_NONE,
/* 					0xFF7C  */ KEY_NONE,
/* 					0xFF7D  */ KEY_NONE,
/* XK_Mode_switch			0xFF7E	*/ KEY_ALTGR,
/* XK_script_switch			0xFF7E  duplicated */
/* XK_Num_Lock				0xFF7F  */ KEY_NUMLOCK,
/* XK_KP_Space				0xFF80	*/ KEY_SPACE,
/* 					0xFF81  */ KEY_NONE,
/* 					0xFF82  */ KEY_NONE,
/* 					0xFF83  */ KEY_NONE,
/* 					0xFF84  */ KEY_NONE,
/* 					0xFF85  */ KEY_NONE,
/* 					0xFF86  */ KEY_NONE,
/* 					0xFF87  */ KEY_NONE,
/* 					0xFF88  */ KEY_NONE,
/* XK_KP_Tab				0xFF89  */ KEY_TAB,
/* 					0xFF8A  */ KEY_NONE,
/* 					0xFF8B  */ KEY_NONE,
/* 					0xFF8C  */ KEY_NONE,
/* XK_KP_Enter				0xFF8D	*/ KEY_ENTER_PAD,
/* 					0xFF8E  */ KEY_NONE,
/* 					0xFF8F  */ KEY_NONE,
/* 					0xFF90  */ KEY_NONE,
/* XK_KP_F1				0xFF91	*/ KEY_F1,
/* XK_KP_F2				0xFF92  */ KEY_F2,
/* XK_KP_F3				0xFF93  */ KEY_F3,
/* XK_KP_F4				0xFF94  */ KEY_F4,
/* XK_KP_Home				0xFF95  */ KEY_7_PAD,
/* XK_KP_Left				0xFF96  */ KEY_4_PAD,
/* XK_KP_Up				0xFF97  */ KEY_8_PAD,
/* XK_KP_Right				0xFF98  */ KEY_6_PAD,
/* XK_KP_Down				0xFF99  */ KEY_2_PAD,
/* XK_KP_Prior				0xFF9A  */ KEY_9_PAD,
/* XK_KP_Page_Up			0xFF9A  duplicated */
/* XK_KP_Next				0xFF9B  */ KEY_3_PAD,
/* XK_KP_Page_Down			0xFF9B  duplicated */
/* XK_KP_End				0xFF9C  */ KEY_1_PAD,
/* XK_KP_Begin				0xFF9D  */ KEY_5_PAD,
/* XK_KP_Insert				0xFF9E  */ KEY_0_PAD,
/* XK_KP_Delete				0xFF9F  */ KEY_DEL_PAD,
/* 					0xFFA0  */ KEY_NONE,
/* 					0xFFA1  */ KEY_NONE,
/* 					0xFFA2  */ KEY_NONE,
/* 					0xFFA3  */ KEY_NONE,
/* 					0xFFA4  */ KEY_NONE,
/* 					0xFFA5  */ KEY_NONE,
/* 					0xFFA6  */ KEY_NONE,
/* 					0xFFA7  */ KEY_NONE,
/* 					0xFFA8  */ KEY_NONE,
/* 					0xFFA9  */ KEY_NONE,
/* XK_KP_Multiply			0xFFAA  */ KEY_ASTERISK,
/* XK_KP_Add				0xFFAB  */ KEY_PLUS_PAD,
/* XK_KP_Separator			0xFFAC	*/ KEY_DEL_PAD,
/* XK_KP_Subtract			0xFFAD  */ KEY_MINUS_PAD,
/* XK_KP_Decimal			0xFFAE  */ KEY_DEL_PAD,
/* XK_KP_Divide				0xFFAF  */ KEY_SLASH_PAD,
/* XK_KP_0				0xFFB0  */ KEY_0_PAD,
/* XK_KP_1				0xFFB1  */ KEY_1_PAD,
/* XK_KP_2				0xFFB2  */ KEY_2_PAD,
/* XK_KP_3				0xFFB3  */ KEY_3_PAD,
/* XK_KP_4				0xFFB4  */ KEY_4_PAD,
/* XK_KP_5				0xFFB5  */ KEY_5_PAD,
/* XK_KP_6				0xFFB6  */ KEY_6_PAD,
/* XK_KP_7				0xFFB7  */ KEY_7_PAD,
/* XK_KP_8				0xFFB8  */ KEY_8_PAD,
/* XK_KP_9				0xFFB9  */ KEY_9_PAD,
/* 					0xFFBA  */ KEY_NONE,
/* 					0xFFBB  */ KEY_NONE,
/* 					0xFFBC  */ KEY_NONE,
/* XK_KP_Equal				0xFFBD	*/ KEY_ENTER_PAD,
/* XK_F1				0xFFBE  */ KEY_F1,
/* XK_F2				0xFFBF  */ KEY_F2,
/* XK_F3				0xFFC0  */ KEY_F3,
/* XK_F4				0xFFC1  */ KEY_F4,
/* XK_F5				0xFFC2  */ KEY_F5,
/* XK_F6				0xFFC3  */ KEY_F6,
/* XK_F7				0xFFC4  */ KEY_F7,
/* XK_F8				0xFFC5  */ KEY_F8,
/* XK_F9				0xFFC6  */ KEY_F9,
/* XK_F10				0xFFC7  */ KEY_F10,
/* XK_F11				0xFFC8  */ KEY_F11,
/* XK_L1				0xFFC8  duplicated */
/* XK_F12				0xFFC9  */ KEY_F12,
/* XK_L2				0xFFC9  duplicated */
/* XK_F13				0xFFCA  */ KEY_NONE,
/* XK_L3				0xFFCA  duplicated */
/* XK_F14				0xFFCB  */ KEY_NONE,
/* XK_L4				0xFFCB  duplicated */
/* XK_F15				0xFFCC  */ KEY_NONE,
/* XK_L5				0xFFCC  duplicated */
/* XK_F16				0xFFCD  */ KEY_NONE,
/* XK_L6				0xFFCD  duplicated */
/* XK_F17				0xFFCE  */ KEY_NONE,
/* XK_L7				0xFFCE  duplicated */
/* XK_F18				0xFFCF  */ KEY_NONE,
/* XK_L8				0xFFCF  duplicated */
/* XK_F19				0xFFD0  */ KEY_NONE,
/* XK_L9				0xFFD0  duplicated */
/* XK_F20				0xFFD1  */ KEY_NONE,
/* XK_L10				0xFFD1  duplicated */
/* XK_F21				0xFFD2  */ KEY_NONE,
/* XK_R1				0xFFD2  duplicated */
/* XK_F22				0xFFD3  */ KEY_NONE,
/* XK_R2				0xFFD3  duplicated */
/* XK_F23				0xFFD4  */ KEY_NONE,
/* XK_R3				0xFFD4  duplicated */
/* XK_F24				0xFFD5  */ KEY_NONE,
/* XK_R4				0xFFD5  duplicated */
/* XK_F25				0xFFD6  */ KEY_NONE,
/* XK_R5				0xFFD6  duplicated */
/* XK_F26				0xFFD7  */ KEY_NONE,
/* XK_R6				0xFFD7  duplicated */
/* XK_F27				0xFFD8  */ KEY_NONE,
/* XK_R7				0xFFD8  duplicated */
/* XK_F28				0xFFD9  */ KEY_NONE,
/* XK_R8				0xFFD9  duplicated */
/* XK_F29				0xFFDA  */ KEY_NONE,
/* XK_R9				0xFFDA  duplicated */
/* XK_F30				0xFFDB  */ KEY_NONE,
/* XK_R10				0xFFDB  duplicated */
/* XK_F31				0xFFDC  */ KEY_NONE,
/* XK_R11				0xFFDC  duplicated */
/* XK_F32				0xFFDD  */ KEY_NONE,
/* XK_R12				0xFFDD  duplicated */
/* XK_F33				0xFFDE  */ KEY_NONE,
/* XK_R13				0xFFDE  duplicated */
/* XK_F34				0xFFDF  */ KEY_NONE,
/* XK_R14				0xFFDF  duplicated */
/* XK_F35				0xFFE0  */ KEY_NONE,
/* XK_R15				0xFFE0  duplicated */
/* XK_Shift_L				0xFFE1	*/ KEY_LSHIFT,
/* XK_Shift_R				0xFFE2	*/ KEY_RSHIFT,
/* XK_Control_L				0xFFE3	*/ KEY_LCONTROL,
/* XK_Control_R				0xFFE4	*/ KEY_RCONTROL,
/* XK_Caps_Lock				0xFFE5	*/ KEY_CAPSLOCK,
/* XK_Shift_Lock			0xFFE6	*/ KEY_CAPSLOCK,
/* XK_Meta_L				0xFFE7	*/ KEY_ALT,
/* XK_Meta_R				0xFFE8	*/ KEY_ALT, 
/* XK_Alt_L				0xFFE9	*/ KEY_ALT,
/* XK_Alt_R				0xFFEA	*/ KEY_ALTGR,
/* XK_Super_L				0xFFEB	*/ KEY_NONE,
/* XK_Super_R				0xFFEC	*/ KEY_NONE,
/* XK_Hyper_L				0xFFED	*/ KEY_NONE,
/* XK_Hyper_R				0xFFEE	*/ KEY_NONE,
/* 					0xFFEF  */ KEY_NONE,
/* 					0xFFF0  */ KEY_NONE,
/* 					0xFFF1  */ KEY_NONE,
/* 					0xFFF2  */ KEY_NONE,
/* 					0xFFF3  */ KEY_NONE,
/* 					0xFFF4  */ KEY_NONE,
/* 					0xFFF5  */ KEY_NONE,
/* 					0xFFF6  */ KEY_NONE,
/* 					0xFFF7  */ KEY_NONE,
/* 					0xFFF8  */ KEY_NONE,
/* 					0xFFF9  */ KEY_NONE,
/* 					0xFFFA  */ KEY_NONE,
/* 					0xFFFB  */ KEY_NONE,
/* 					0xFFFC  */ KEY_NONE,
/* 					0xFFFD  */ KEY_NONE,
/* 					0xFFFE  */ KEY_NONE,
/* XK_Delete				0xFFFF	*/ KEY_DEL
}; 	/* extended_code_table */

static int code_table[] = {

/* 			  0x000  */ KEY_NONE,
/* 			  0x001  */ KEY_NONE,
/* 			  0x002  */ KEY_NONE,
/* 			  0x003  */ KEY_NONE,
/* 			  0x004  */ KEY_NONE,
/* 			  0x005  */ KEY_NONE,
/* 			  0x006  */ KEY_NONE,
/* 			  0x007  */ KEY_NONE,
/* 			  0x008  */ KEY_NONE,
/* 			  0x009  */ KEY_NONE,
/* 			  0x00a  */ KEY_NONE,
/* 			  0x00b  */ KEY_NONE,
/* 			  0x00c  */ KEY_NONE,
/* 			  0x00d  */ KEY_NONE,
/* 			  0x00e  */ KEY_NONE,
/* 			  0x00f  */ KEY_NONE,
/* 			  0x010  */ KEY_NONE,
/* 			  0x011  */ KEY_NONE,
/* 			  0x012  */ KEY_NONE,
/* 			  0x013  */ KEY_NONE,
/* 			  0x014  */ KEY_NONE,
/* 			  0x015  */ KEY_NONE,
/* 			  0x016  */ KEY_NONE,
/* 			  0x017  */ KEY_NONE,
/* 			  0x018  */ KEY_NONE,
/* 			  0x019  */ KEY_NONE,
/* 			  0x01a  */ KEY_NONE,
/* 			  0x01b  */ KEY_NONE,
/* 			  0x01c  */ KEY_NONE,
/* 			  0x01d  */ KEY_NONE,
/* 			  0x01e  */ KEY_NONE,
/* 			  0x01f  */ KEY_NONE,
/* XK_space               0x020  */ KEY_SPACE,
/* XK_exclam              0x021  */ KEY_1,
/* XK_quotedbl            0x022  */ KEY_QUOTE,
/* XK_numbersign          0x023  */ KEY_3,
/* XK_dollar              0x024  */ KEY_4,
/* XK_percent             0x025  */ KEY_5,
/* XK_ampersand           0x026  */ KEY_7,
/* XK_apostrophe          0x027  */ KEY_QUOTE, /* keyboard dependent */
/* XK_quoteright          0x027	 duplicated */ 
/* XK_parenleft           0x028  */ KEY_9,
/* XK_parenright          0x029  */ KEY_0,
/* XK_asterisk            0x02a  */ KEY_ASTERISK,
/* XK_plus                0x02b  */ KEY_EQUALS,/* keyboard dependent */
/* XK_comma               0x02c  */ KEY_COMMA,
/* XK_minus               0x02d  */ KEY_MINUS,
/* XK_period              0x02e  */ KEY_STOP,
/* XK_slash               0x02f  */ KEY_SLASH,
/* XK_0                   0x030  */ KEY_0,
/* XK_1                   0x031  */ KEY_1,
/* XK_2                   0x032  */ KEY_2,
/* XK_3                   0x033  */ KEY_3,
/* XK_4                   0x034  */ KEY_4,
/* XK_5                   0x035  */ KEY_5,
/* XK_6                   0x036  */ KEY_6,
/* XK_7                   0x037  */ KEY_7,
/* XK_8                   0x038  */ KEY_8,
/* XK_9                   0x039  */ KEY_9,
/* XK_colon               0x03a  */ KEY_COLON,
/* XK_semicolon           0x03b  */ KEY_COLON,
/* XK_less                0x03c  */ KEY_COMMA,
/* XK_equal               0x03d  */ KEY_EQUALS,
/* XK_greater             0x03e  */ KEY_STOP,
/* XK_question            0x03f  */ KEY_SLASH,
/* XK_at                  0x040  */ KEY_2,
/* XK_A                   0x041  */ KEY_A,
/* XK_B                   0x042  */ KEY_B,
/* XK_C                   0x043  */ KEY_C,
/* XK_D                   0x044  */ KEY_D,
/* XK_E                   0x045  */ KEY_E,
/* XK_F                   0x046  */ KEY_F,
/* XK_G                   0x047  */ KEY_G,
/* XK_H                   0x048  */ KEY_H,
/* XK_I                   0x049  */ KEY_I,
/* XK_J                   0x04a  */ KEY_J,
/* XK_K                   0x04b  */ KEY_K,
/* XK_L                   0x04c  */ KEY_L,
/* XK_M                   0x04d  */ KEY_M,
/* XK_N                   0x04e  */ KEY_N,
/* XK_O                   0x04f  */ KEY_O,
/* XK_P                   0x050  */ KEY_P,
/* XK_Q                   0x051  */ KEY_Q,
/* XK_R                   0x052  */ KEY_R,
/* XK_S                   0x053  */ KEY_S,
/* XK_T                   0x054  */ KEY_T,
/* XK_U                   0x055  */ KEY_U,
/* XK_V                   0x056  */ KEY_V,
/* XK_W                   0x057  */ KEY_W,
/* XK_X                   0x058  */ KEY_X,
/* XK_Y                   0x059  */ KEY_Y,
/* XK_Z                   0x05a  */ KEY_Z,
/* XK_bracketleft         0x05b  */ KEY_OPENBRACE,
/* XK_backslash           0x05c  */ KEY_BACKSLASH,
/* XK_bracketright        0x05d  */ KEY_CLOSEBRACE,
/* XK_asciicircum         0x05e  */ KEY_TILDE,
/* XK_underscore          0x05f  */ KEY_MINUS,
/* XK_grave               0x060  */ KEY_TILDE, /* keyboard dependent */
/* XK_quoteleft           0x060  duplicated */
/* XK_a                   0x061  */ KEY_A,
/* XK_b                   0x062  */ KEY_B,
/* XK_c                   0x063  */ KEY_C,
/* XK_d                   0x064  */ KEY_D,
/* XK_e                   0x065  */ KEY_E,
/* XK_f                   0x066  */ KEY_F,
/* XK_g                   0x067  */ KEY_G,
/* XK_h                   0x068  */ KEY_H,
/* XK_i                   0x069  */ KEY_I,
/* XK_j                   0x06a  */ KEY_J,
/* XK_k                   0x06b  */ KEY_K,
/* XK_l                   0x06c  */ KEY_L,
/* XK_m                   0x06d  */ KEY_M,
/* XK_n                   0x06e  */ KEY_N,
/* XK_o                   0x06f  */ KEY_O,
/* XK_p                   0x070  */ KEY_P,
/* XK_q                   0x071  */ KEY_Q,
/* XK_r                   0x072  */ KEY_R,
/* XK_s                   0x073  */ KEY_S,
/* XK_t                   0x074  */ KEY_T,
/* XK_u                   0x075  */ KEY_U,
/* XK_v                   0x076  */ KEY_V,
/* XK_w                   0x077  */ KEY_W,
/* XK_x                   0x078  */ KEY_X,
/* XK_y                   0x079  */ KEY_Y,
/* XK_z                   0x07a  */ KEY_Z,
/* XK_braceleft           0x07b  */ KEY_OPENBRACE,
/* XK_bar                 0x07c  */ KEY_NONE,
/* XK_braceright          0x07d  */ KEY_CLOSEBRACE,
/* XK_asciitilde          0x07e  */ KEY_TILDE,
/*                        0x07f  */ KEY_NONE,
/* 			  0x080  */ KEY_NONE,
/* 			  0x081  */ KEY_NONE,
/* 			  0x082  */ KEY_NONE,
/* 			  0x083  */ KEY_NONE,
/* 			  0x084  */ KEY_NONE,
/* 			  0x085  */ KEY_NONE,
/* 			  0x086  */ KEY_NONE,
/* 			  0x087  */ KEY_NONE,
/* 			  0x088  */ KEY_NONE,
/* 			  0x089  */ KEY_NONE,
/* 			  0x08a  */ KEY_NONE,
/* 			  0x08b  */ KEY_NONE,
/* 			  0x08c  */ KEY_NONE,
/* 			  0x08d  */ KEY_NONE,
/* 			  0x08e  */ KEY_NONE,
/* 			  0x08f  */ KEY_NONE,
/* 			  0x090  */ KEY_NONE,
/* 			  0x091  */ KEY_NONE,
/* 			  0x092  */ KEY_NONE,
/* 			  0x093  */ KEY_NONE,
/* 			  0x094  */ KEY_NONE,
/* 			  0x095  */ KEY_NONE,
/* 			  0x096  */ KEY_NONE,
/* 			  0x097  */ KEY_NONE,
/* 			  0x098  */ KEY_NONE,
/* 			  0x099  */ KEY_NONE,
/* 			  0x09a  */ KEY_NONE,
/* 			  0x09b  */ KEY_NONE,
/* 			  0x09c  */ KEY_NONE,
/* 			  0x09d  */ KEY_NONE,
/* 			  0x09e  */ KEY_NONE,
/* 			  0x09f  */ KEY_NONE,
/* XK_nobreakspace        0x0a0  */ KEY_NONE,
/* XK_exclamdown          0x0a1  */ KEY_EQUALS, /* keyboard dependent */
/* XK_cent                0x0a2  */ KEY_NONE,
/* XK_sterling            0x0a3  */ KEY_NONE,
/* XK_currency            0x0a4  */ KEY_NONE,
/* XK_yen                 0x0a5  */ KEY_NONE,
/* XK_brokenbar           0x0a6  */ KEY_NONE,
/* XK_section             0x0a7  */ KEY_TILDE,
/* XK_diaeresis           0x0a8  */ KEY_NONE,
/* XK_copyright           0x0a9  */ KEY_NONE,
/* XK_ordfeminine         0x0aa  */ KEY_NONE,
/* XK_guillemotleft       0x0ab  */ KEY_NONE,
/* XK_notsign             0x0ac  */ KEY_NONE,
/* XK_hyphen              0x0ad  */ KEY_NONE,
/* XK_registered          0x0ae  */ KEY_NONE,
/* XK_macron              0x0af  */ KEY_NONE,
/* XK_degree              0x0b0  */ KEY_NONE,
/* XK_plusminus           0x0b1  */ KEY_NONE,
/* XK_twosuperior         0x0b2  */ KEY_NONE,
/* XK_treesuperior        0x0b3  */ KEY_NONE,
/* XK_acute               0x0b4  */ KEY_QUOTE, /* keyboard dependent */
/* XK_mu                  0x0b5  */ KEY_NONE,
/* XK_paragraph           0x0b6  */ KEY_NONE,
/* XK_periodcentered      0x0b7  */ KEY_NONE,
/* XK_cedilla             0x0b8  */ KEY_NONE,
/* XK_onesuperior         0x0b9  */ KEY_NONE,
/* XK_masculine           0x0ba  */ KEY_NONE,
/* XK_guillemotright      0x0bb  */ KEY_NONE,
/* XK_onequarter          0x0bc  */ KEY_NONE,
/* XK_onehalf             0x0bd  */ KEY_NONE,
/* XK_threequarters       0x0be  */ KEY_NONE,
/* XK_questiondown        0x0bf  */ KEY_NONE,
/* XK_Agrave              0x0c0  */ KEY_NONE,
/* XK_Aacute              0x0c1  */ KEY_NONE,
/* XK_Acircumflex         0x0c2  */ KEY_NONE,
/* XK_Atilde              0x0c3  */ KEY_NONE,
/* XK_Adiaeresis          0x0c4  */ KEY_NONE,
/* XK_Aring               0x0c5  */ KEY_NONE,
/* XK_AE                  0x0c6  */ KEY_NONE,
/* XK_Ccedilla            0x0c7  */ KEY_NONE,
/* XK_Egrave              0x0c8  */ KEY_NONE,
/* XK_Eacute              0x0c9  */ KEY_NONE,
/* XK_Ecircumflex         0x0ca  */ KEY_NONE,
/* XK_Ediaeresis          0x0cb  */ KEY_NONE,
/* XK_Igrave              0x0cc  */ KEY_NONE,
/* XK_Iacute              0x0cd  */ KEY_NONE,
/* XK_Icircumflex         0x0ce  */ KEY_NONE,
/* XK_Idiaeresis          0x0cf  */ KEY_NONE,
/* XK_ETH                 0x0d0  */ KEY_NONE,
/* XK_Eth                 0x0d0  duplicated */
/* XK_Ntilde              0x0d1  */ KEY_NONE,
/* XK_Ograve              0x0d2  */ KEY_NONE,
/* XK_Oacute              0x0d3  */ KEY_NONE,
/* XK_Ocircumflex         0x0d4  */ KEY_NONE,
/* XK_Otilde              0x0d5  */ KEY_NONE,
/* XK_Odiaeresis          0x0d6  */ KEY_NONE,
/* XK_multiply            0x0d7  */ KEY_NONE,
/* XK_Ooblique            0x0d8  */ KEY_NONE,
/* XK_Ugrave              0x0d9  */ KEY_NONE,
/* XK_Uacute              0x0da  */ KEY_NONE,
/* XK_Ucircumflex         0x0db  */ KEY_NONE,
/* XK_Udiaeresis          0x0dc  */ KEY_NONE,
/* XK_Yacute              0x0dd  */ KEY_NONE,
/* XK_THORN               0x0de  */ KEY_NONE,
/* XK_Thorn               0x0de  duplicated */
/* XK_ssharp              0x0df  */ KEY_NONE,
/* XK_agrave              0x0e0  */ KEY_NONE,
/* XK_aacute              0x0e1  */ KEY_NONE,
/* XK_acircumflex         0x0e2  */ KEY_NONE,
/* XK_atilde              0x0e3  */ KEY_NONE,
/* XK_adiaeresis          0x0e4  */ KEY_NONE,
/* XK_aring               0x0e5  */ KEY_NONE,
/* XK_ae                  0x0e6  */ KEY_NONE,
/* XK_ccedilla            0x0e7  */ KEY_SLASH, /* keyboard dependent */
/* XK_egrave              0x0e8  */ KEY_NONE,
/* XK_eacute              0x0e9  */ KEY_NONE,
/* XK_ecircumflex         0x0ea  */ KEY_NONE,
/* XK_ediaeresis          0x0eb  */ KEY_NONE,
/* XK_igrave              0x0ec  */ KEY_NONE,
/* XK_iacute              0x0ed  */ KEY_NONE,
/* XK_icircumflex         0x0ee  */ KEY_NONE,
/* XK_idiaeresis          0x0ef  */ KEY_NONE,
/* XK_eth                 0x0f0  */ KEY_NONE,
/* XK_ntilde              0x0f1  */ KEY_COLON, /* keyboard dependent */
/* XK_ograve              0x0f2  */ KEY_NONE,
/* XK_oacute              0x0f3  */ KEY_NONE,
/* XK_ocircumflex         0x0f4  */ KEY_NONE,
/* XK_otilde              0x0f5  */ KEY_NONE,
/* XK_odiaeresis          0x0f6  */ KEY_NONE,
/* XK_division            0x0f7  */ KEY_NONE,
/* XK_oslash              0x0f8  */ KEY_NONE,
/* XK_ugrave              0x0f9  */ KEY_NONE,
/* XK_uacute              0x0fa  */ KEY_NONE,
/* XK_ucircumflex         0x0fb  */ KEY_NONE,
/* XK_udiaeresis          0x0fc  */ KEY_NONE,
/* XK_yacute              0x0fd  */ KEY_NONE,
/* XK_thorn               0x0fe  */ KEY_NONE,
/* XK_ydiaeresis          0x0ff  */ KEY_NONE

}; /* code_table */
