#ifndef EDITORCTL_H
#define EDITORCTL_H

#include <windows.h>

#include "../editor/editor.h"

typedef enum tagEDITORCTL_WHITESPACE
{
    EDITORCTL_WHITESPACE_SPACE,
    EDITORCTL_WHITESPACE_TAB_START,
    EDITORCTL_WHITESPACE_TAB_MIDDLE,
    EDITORCTL_WHITESPACE_TAB_END,
    EDITORCTL_WHITESPACE_TAB,
    EDITORCTL_WHITESPACE_CR,
    EDITORCTL_WHITESPACE_LF,
    EDITORCTL_WHITESPACE_WRAP,

    EDITORCTL_WHITESPACE_LAST
} EDITORCTL_WHITESPACE;

typedef enum tagEDITORCTL_MOUSE_SELECT_MODE
{
    EDITORCTL_MSM_CHAR = 0,
    EDITORCTL_MSM_WORD = 1,
    EDITORCTL_MSM_ROW = 2
} EDITORCTL_MOUSE_SELECT_MODE;

typedef struct tagEDITORCTL
{
    HWND hwnd;
    HANDLE heap;

    HFONT font;
    HBITMAP whitespace_icons;

    SIZE cell_size;
    POINT scroll_location;

    BOOL overwrite;

    char *new_line;
    int new_line_length;

    int mouse_anchor_offset;
    EDITORCTL_MOUSE_SELECT_MODE mouse_select_mode;
    ULONGLONG last_click_time;
    int last_click_x;
    int last_click_y;

    EDITOR editor;
} EDITORCTL;

extern LPTSTR EDITORCTL_CLASS_NAME;

ATOM editorctl_register_class (HINSTANCE hInstance);
LRESULT CALLBACK editorctl_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_create (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_destroy (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_paint (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_setfocus (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_killfocus (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_size (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_hscroll (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_vscroll (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_mousewheel (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_setfont (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_char (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_keydown (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_lbuttondown (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_mousemove (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_lbuttonup (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL editorctl_create_whitespace_icons (EDITORCTL *editorctl);

BOOL editorctl_update_caret (EDITORCTL *editorctl, BOOL ensure_visible);
BOOL editorctl_scroll_to (EDITORCTL *editorctl, int x, int y);
BOOL editorctl_update_scroll_range (EDITORCTL *editorctl);
BOOL editorctl_ensure_cell_visible (EDITORCTL *editorctl, int row, int col);

BOOL editorctl_move_mouse_cursor (EDITORCTL *editorctl, int x, int y, BOOL selecting);
BOOL editorctl_invalidate_rows (EDITORCTL *editorctl, int start_row, int end_row);

BOOL editorctl_update (EDITORCTL *editorctl);

#endif
