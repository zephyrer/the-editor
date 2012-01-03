#ifndef EDITORCTL_H
#define EDITORCTL_H

#include <windows.h>

#include "../utils/utils.h"
#include "../intlist/intlist.h"
#include "../text/text.h"

#define EDITORCTL_MAX_CHAR_WIDTH 5

typedef UINT32 EDITORCTL_UNICODE_CHAR;

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

typedef struct tagEDITORCTL_EXTRA
{
    HWND hwnd;
    HANDLE heap;

    HFONT font;
    HBITMAP whitespace_icons;

    SIZE cell_size;
    POINT scroll_location;

    BOOL overwrite;

    int row_count;
    int column_count;
    int line_count;

    int caret_offset;
    int caret_column;

    int tab_width;
    char *new_line;
    int new_line_length;
    int word_wrap_column;
    int word_wrap_min_column;

    int anchor_offset;

    int mouse_anchor_offset;
    EDITORCTL_MOUSE_SELECT_MODE mouse_select_mode;
    ULONGLONG last_click_time;
    int last_click_x;
    int last_click_y;

    INTLIST row_offsets;
    INTLIST row_widths;
    INTLIST line_rows;

    TEXT text;
} EDITORCTL_EXTRA;

typedef struct tagEDITORCTL_TEXT_ITERATOR
{
    int offset;

    char *text;
    int text_length;
} EDITORCTL_TEXT_ITERATOR;

extern LPTSTR EDITORCTL_CLASS_NAME;

ATOM editorctl_register_class (HINSTANCE hInstance);
LRESULT CALLBACK editorctl_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_paint (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_setfocus (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_killfocus (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_size (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_hscroll (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_vscroll (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_mousewheel (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_setfont (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_char (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_keydown (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_lbuttondown (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_mousemove (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT editorctl_on_lbuttonup (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL editorctl_update (EDITORCTL_EXTRA *extra, int offset, int old_length, int new_length);

BOOL editorctl_create_whitespace_icons (EDITORCTL_EXTRA *extra);

BOOL editorctl_update_caret_pos (EDITORCTL_EXTRA *extra);
BOOL editorctl_scroll_to (EDITORCTL_EXTRA *extra, int x, int y);
BOOL editorctl_update_scroll_range (EDITORCTL_EXTRA *extra);
BOOL editorctl_ensure_cell_visible (EDITORCTL_EXTRA *extra, int row, int col);

BOOL editorctl_replace_range (EDITORCTL_EXTRA *extra, int offset, int length, const char *buffer, int buffer_length, int new_caret_offset);
BOOL editorctl_move_cursor (EDITORCTL_EXTRA *extra, int offset, BOOL selecting);
BOOL editorctl_move_mouse_cursor (EDITORCTL_EXTRA *extra, int x, int y, BOOL selecting);
BOOL editorctl_invalidate_rows (EDITORCTL_EXTRA *extra, int start_row, int end_row);

BOOL editorctl_offset_to_row (EDITORCTL_EXTRA *extra, int offset, int *row);
BOOL editorctl_offset_to_rc (EDITORCTL_EXTRA *extra, int offset, int *row, int *col);
BOOL editorctl_rc_to_offset (EDITORCTL_EXTRA *extra, int row, int column, int *offset, int *real_col);

BOOL editorctl_offset_to_line (EDITORCTL_EXTRA *extra, int offset, int *line);
BOOL editorctl_row_to_line (EDITORCTL_EXTRA *extra, int row, int *line);
BOOL editorctl_offset_to_lp (EDITORCTL_EXTRA *extra, int offset, int *line, int *pos);
BOOL editorctl_lp_to_offset (EDITORCTL_EXTRA *extra, int line, int pos, int *offset);

#endif
