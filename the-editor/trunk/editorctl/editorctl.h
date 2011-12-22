#ifndef EDITORCTL_H
#define EDITORCTL_H

#include <Windows.h>

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

typedef struct tagEDITORCTL_EXTRA
{
    HANDLE heap;

    HFONT font;
    HBITMAP whitespace_icons;

    SIZE cell_size;
    POINT scroll_location;

    BOOL overwrite;

    int row_count;
    int column_count;
    int *row_offsets;
    int *row_widths;

    int caret_offset;
    int caret_column;

    int tab_width;
    char *new_line;
    int new_line_length;
    int word_wrap_column;
    int word_wrap_min_column;

    char *text;
    int text_length;

    int anchor_offset;
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

BOOL editorctl_update (HWND hwnd, int offset, int old_length, int new_length);

BOOL editorctl_create_whitespace_icons (HWND hwnd);

BOOL editorctl_update_caret_pos (HWND hwnd);
BOOL editorctl_scroll_to (HWND hwnd, int x, int y);
BOOL editorctl_update_scroll_range (HWND hwnd);

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length, int new_caret_offset);
BOOL editorctl_move_cursor (HWND hwnd, int offset, BOOL selecting);
BOOL editorctl_invalidate_rows (HWND hwnd, int start_row, int end_row);

BOOL editorctl_offset_to_row (HWND hwnd, int offset, int *row);
BOOL editorctl_offset_to_rc (HWND hwnd, int offset, int *row, int *col);
BOOL editorctl_rc_to_offset (HWND hwnd, int row, int column, int *offset, int *real_col);

BOOL editorctl_set_iterator (HWND hwnd, int offset, EDITORCTL_TEXT_ITERATOR *iterator);
BOOL editorctl_forward (EDITORCTL_TEXT_ITERATOR *iterator);
BOOL editorctl_backward (EDITORCTL_TEXT_ITERATOR *iterator);
EDITORCTL_UNICODE_CHAR editorctl_get_prev_char (EDITORCTL_TEXT_ITERATOR *iterator);
EDITORCTL_UNICODE_CHAR editorctl_get_next_char (EDITORCTL_TEXT_ITERATOR *iterator);

void editorctl_set_next_char (EDITORCTL_UNICODE_CHAR ch, char **ptr);

#endif
