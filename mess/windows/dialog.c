//============================================================
//
//	dialog.c - Win32 MESS dialogs handling
//
//============================================================

#include <windows.h>

#include "dialog.h"
#include "mame.h"
#include "../windows/window.h"
#include "ui_text.h"

//============================================================
//	These defines are necessary because the MinGW headers do
//	not have the latest definitions
//============================================================

#ifdef __GNUC__
#define GetWindowLongPtr(hwnd, idx)			((LONG_PTR) GetWindowLong((hwnd), (idx)))
#define SetWindowLongPtr(hwnd, idx, val)	((LONG_PTR) SetWindowLong((hwnd), (idx), (val)))
#define GWLP_USERDATA						GWL_USERDATA
#endif

//============================================================

enum
{
	TRIGGER_INITDIALOG	= 1,
	TRIGGER_APPLY		= 2
};

typedef LRESULT (*trigger_function)(HWND dlgwnd, UINT message, WPARAM wparam, LPARAM lparam);

struct dialog_info_trigger
{
	struct dialog_info_trigger *next;
	WORD dialog_item;
	WORD trigger_flags;
	UINT message;
	WPARAM wparam;
	LPARAM lparam;
	UINT16 *result;
	trigger_function trigger_proc;
};

struct dialog_info
{
	HGLOBAL handle;
	struct dialog_info_trigger *trigger_first;
	struct dialog_info_trigger *trigger_last;
	WORD item_count;
	WORD cx, cy;
	int combo_string_count;
	int combo_default_value;
};

//============================================================
//	PARAMETERS
//============================================================

#define DIM_VERTICAL_SPACING	2
#define DIM_HORIZONTAL_SPACING	2
#define DIM_ROW_HEIGHT			12
#define DIM_LABEL_WIDTH			80
#define DIM_COMBO_WIDTH			140
#define DIM_BUTTON_WIDTH		50

#define WNDLONG_DIALOG			GWLP_USERDATA

#define DLGITEM_BUTTON			0x0080
#define DLGITEM_EDIT			0x0081
#define DLGITEM_STATIC			0x0082
#define DLGITEM_LISTBOX			0x0083
#define DLGITEM_SCROLLBAR		0x0084
#define DLGITEM_COMBOBOX		0x0085

#define DLGTEXT_OK				ui_getstring(UI_OK)
#define DLGTEXT_APPLY			"Apply"
#define DLGTEXT_CANCEL			"Cancel"

//============================================================
//	dialog_trigger
//============================================================

static void dialog_trigger(HWND dlgwnd, WORD trigger_flags)
{
	LRESULT result;
	HWND dialog_item;
	struct dialog_info *di;
	struct dialog_info_trigger *trigger;

	di = (struct dialog_info *) GetWindowLongPtr(dlgwnd, WNDLONG_DIALOG);
	assert(di);
	for (trigger = di->trigger_first; trigger; trigger = trigger->next)
	{
		if (trigger->trigger_flags & trigger_flags)
		{
			dialog_item = GetDlgItem(dlgwnd, trigger->dialog_item);
			assert(dialog_item);
			result = 0;

			if (trigger->message)
				result = SendMessage(dialog_item, trigger->message, trigger->wparam, trigger->lparam);
			if (trigger->trigger_proc)
				result = trigger->trigger_proc(dialog_item, trigger->message, trigger->wparam, trigger->lparam);

			if (trigger->result)
				*(trigger->result) = result;
		}
	}
}

//============================================================
//	dialog_proc
//============================================================

static INT_PTR CALLBACK dialog_proc(HWND dlgwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	INT_PTR handled = TRUE;
	TCHAR buf[32];
	const char *str;

	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(dlgwnd, WNDLONG_DIALOG, (LONG_PTR) lparam);
		dialog_trigger(dlgwnd, TRIGGER_INITDIALOG);
		break;

	case WM_COMMAND:
		GetWindowText((HWND) lparam, buf, sizeof(buf) / sizeof(buf[0]));

		str = buf;
		if (!strcmp(str, DLGTEXT_OK))
		{
			dialog_trigger(dlgwnd, TRIGGER_APPLY);
			EndDialog(dlgwnd, 0);
		}
		else if (!strcmp(str, DLGTEXT_CANCEL))
		{
			EndDialog(dlgwnd, 0);
		}
		else
		{
			handled = FALSE;
		}
		break;

	case WM_DESTROY:
		EndDialog(dlgwnd, 0);
		break;

	default:
		handled = FALSE;
		break;
	}
	return handled;
}


//============================================================
//	dialog_write
//============================================================

static int dialog_write(struct dialog_info *di, const void *ptr, size_t sz, int align)
{
	HGLOBAL newhandle;
	size_t base;
	UINT8 *mem;
	UINT8 *mem2;

	if (!di->handle)
	{
		newhandle = GlobalAlloc(GMEM_ZEROINIT, sz);
		base = 0;
	}
	else
	{
		base = GlobalSize(di->handle);
		base += align - 1;
		base -= base % align;
		newhandle = GlobalReAlloc(di->handle, base + sz, GMEM_ZEROINIT);
		if (!newhandle)
		{
			newhandle = GlobalAlloc(GMEM_ZEROINIT, base + sz);
			if (newhandle)
			{
				mem = GlobalLock(di->handle);
				mem2 = GlobalLock(newhandle);
				memcpy(mem2, mem, base);
				GlobalUnlock(di->handle);
				GlobalUnlock(newhandle);
				GlobalFree(di->handle);
			}
		}
	}
	if (!newhandle)
		return 1;

	mem = GlobalLock(newhandle);
	memcpy(mem + base, ptr, sz);
	GlobalUnlock(newhandle);
	di->handle = newhandle;
	return 0;
}


//============================================================
//	dialog_write_string
//============================================================

static int dialog_write_string(struct dialog_info *di, const char *str)
{
	int sz;
	WCHAR *wstr;
	
	sz = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	wstr = alloca(sz * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, sz);
	return dialog_write(di, wstr, sz * sizeof(WCHAR), 2);
}

//============================================================
//	dialog_write_item
//============================================================

static int dialog_write_item(struct dialog_info *di, DWORD style, short x, short y,
	 short cx, short cy, const char *str, WORD class_atom)
{
	DLGITEMTEMPLATE item_template;
	WORD w[2];

	memset(&item_template, 0, sizeof(item_template));
	item_template.style = style;
	item_template.x = x;
	item_template.y = y;
	item_template.cx = cx;
	item_template.cy = cy;
	item_template.id = di->item_count + 1;

	if (dialog_write(di, &item_template, sizeof(item_template), 4))
		return 1;

	w[0] = 0xffff;
	w[1] = class_atom;
	if (dialog_write(di, w, sizeof(w), 2))
		return 1;

	if (dialog_write_string(di, str))
		return 1;

	w[0] = 0;
	if (dialog_write(di, w, sizeof(w[0]), 2))
		return 1;

	di->item_count++;
	return 0;
}

//============================================================
//	dialog_add_trigger
//============================================================

static int dialog_add_trigger(struct dialog_info *di, WORD dialog_item,
	WORD trigger_flags, UINT message, trigger_function trigger_proc,
	WPARAM wparam, LPARAM lparam, UINT16 *result)
{
	struct dialog_info_trigger *trigger;

	assert(di);
	assert(trigger_flags);

	trigger = (struct dialog_info_trigger *) malloc(sizeof(struct dialog_info_trigger));
	if (!trigger)
		return 1;

	trigger->next = NULL;
	trigger->trigger_flags = trigger_flags;
	trigger->dialog_item = dialog_item;
	trigger->message = message;
	trigger->trigger_proc = trigger_proc;
	trigger->wparam = wparam;
	trigger->lparam = lparam;
	trigger->result = result;

	if (di->trigger_last)
		di->trigger_last->next = trigger;
	else
		di->trigger_first = trigger;
	di->trigger_last = trigger;
	return 0;
}

//============================================================
//	dialog_prime
//============================================================

static void dialog_prime(struct dialog_info *di)
{
	DLGTEMPLATE *dlg_template;

	dlg_template = (DLGTEMPLATE *) GlobalLock(di->handle);
	dlg_template->cdit = di->item_count;
	dlg_template->cx = di->cx;
	dlg_template->cy = di->cy;
	GlobalUnlock(di->handle);
}

//============================================================
//	dialog_get_combo_value
//============================================================

static LRESULT dialog_get_combo_value(HWND dialog_item, UINT message, WPARAM wparam, LPARAM lparam)
{
	int idx;
	idx = SendMessage(dialog_item, CB_GETCURSEL, 0, 0);
	if (idx == CB_ERR)
		return 0;
	return SendMessage(dialog_item, CB_GETITEMDATA, idx, 0);
}

//============================================================
//	win_dialog_init
//============================================================

void *win_dialog_init(const char *title)
{
	struct dialog_info *di;
	DLGTEMPLATE dlg_template;
	WORD w[2];

	// create the dialog structure
	di = malloc(sizeof(struct dialog_info));
	if (!di)
		goto error;
	memset(di, 0, sizeof(*di));

	di->cx = 0;
	di->cy = 0;

	memset(&dlg_template, 0, sizeof(dlg_template));
	dlg_template.style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION;
	dlg_template.x = 10;
	dlg_template.y = 10;
	if (dialog_write(di, &dlg_template, sizeof(dlg_template), 4))
		goto error;

	w[0] = 0;
	w[1] = 0;
	if (dialog_write(di, w, sizeof(w), 2))
		goto error;

	if (dialog_write_string(di, title))
		goto error;

	return (void *) di;

error:
	if (di)
		win_dialog_exit(di);
	return NULL;
}


//============================================================
//	win_dialog_add_combobox
//============================================================

int win_dialog_add_combobox(void *dialog, const char *label, UINT16 *value)
{
	struct dialog_info *di = (struct dialog_info *) dialog;
	short x;
	short y;

	x = DIM_HORIZONTAL_SPACING;
	y = di->cy + DIM_VERTICAL_SPACING;

	if (dialog_write_item(di, WS_CHILD | WS_VISIBLE | SS_LEFT,
			x, y, DIM_LABEL_WIDTH, DIM_ROW_HEIGHT, label, DLGITEM_STATIC))
		return 1;

	x += DIM_LABEL_WIDTH + DIM_HORIZONTAL_SPACING;
	if (dialog_write_item(di, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
			x, y, DIM_COMBO_WIDTH, DIM_ROW_HEIGHT * 8, "", DLGITEM_COMBOBOX))
		return 1;
	di->combo_string_count = 0;
	di->combo_default_value = *value;

	if (dialog_add_trigger(di, di->item_count, TRIGGER_APPLY, 0, dialog_get_combo_value, 0, 0, value))
		return 1;

	x += DIM_COMBO_WIDTH + DIM_HORIZONTAL_SPACING;

	if (x > di->cx)
		di->cx = x;
	di->cy += DIM_ROW_HEIGHT + DIM_VERTICAL_SPACING * 2;
	return 0;
}

//============================================================
//	win_dialog_add_combobox_item
//============================================================

int win_dialog_add_combobox_item(void *dialog, const char *item_label, int item_data)
{
	struct dialog_info *di = (struct dialog_info *) dialog;
	if (dialog_add_trigger(di, di->item_count, TRIGGER_INITDIALOG, CB_ADDSTRING, NULL, 0, (LPARAM) item_label, NULL))
		return 1;
	di->combo_string_count++;
	if (dialog_add_trigger(di, di->item_count, TRIGGER_INITDIALOG, CB_SETITEMDATA, NULL, di->combo_string_count-1, (LPARAM) item_data, NULL))
		return 1;
	if (item_data == di->combo_default_value)
	{
		if (dialog_add_trigger(di, di->item_count, TRIGGER_INITDIALOG, CB_SETCURSEL, NULL, di->combo_string_count-1, 0, NULL))
			return 1;
	}
	return 0;
}

//============================================================
//	win_dialog_add_standard_buttons
//============================================================

int win_dialog_add_standard_buttons(void *dialog)
{
	struct dialog_info *di = (struct dialog_info *) dialog;
	short x;
	short y;

	x = di->cx - DIM_HORIZONTAL_SPACING - DIM_BUTTON_WIDTH;
	y = di->cy + DIM_VERTICAL_SPACING;

	if (dialog_write_item(di, WS_CHILD | WS_VISIBLE | SS_LEFT,
			x, y, DIM_BUTTON_WIDTH, DIM_ROW_HEIGHT, DLGTEXT_CANCEL, DLGITEM_BUTTON))
		return 1;

	x -= DIM_HORIZONTAL_SPACING + DIM_BUTTON_WIDTH;
	if (dialog_write_item(di, WS_CHILD | WS_VISIBLE | SS_LEFT,
			x, y, DIM_BUTTON_WIDTH, DIM_ROW_HEIGHT, DLGTEXT_OK, DLGITEM_BUTTON))
		return 1;
	di->cy += DIM_ROW_HEIGHT + DIM_VERTICAL_SPACING * 2;
	return 0;
}

//============================================================
//	win_dialog_exit
//============================================================

void win_dialog_exit(void *dialog)
{
	struct dialog_info *di = (struct dialog_info *) dialog;
	struct dialog_info_trigger *trigger;
	struct dialog_info_trigger *next;

	assert(di);
	if (di->handle)
		GlobalFree(di->handle);

	trigger = di->trigger_first;
	while(trigger)
	{
		next = trigger->next;
		free(trigger);
		trigger = next;
	}
	free(di);
}

//============================================================
//	win_dialog_runmodal
//============================================================

void win_dialog_runmodal(void *dialog)
{
	struct dialog_info *di;
	
	di = (struct dialog_info *) dialog;
	assert(di);
	dialog_prime(di);
	DialogBoxIndirectParam(NULL, di->handle, win_video_window, dialog_proc, (LPARAM) di);
}

