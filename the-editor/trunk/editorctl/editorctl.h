#ifndef EDITORCTL_H
#define EDITORCTL_H

#include <Windows.h>

#define EDITORCTL_MAX_CHAR_WIDTH 5

typedef struct tagEDITORCTL_EXTRA
{
    HANDLE heap;

    HFONT font;

    SIZE cell_size;
    POINT scroll_location;

    BOOL overwrite;

    int row_count;
    int column_count;

    int caret_offset;

    int tab_width;
    char *new_line;
    int new_line_length;

    char *text;
    int text_length;
    int line_count;
    int *line_offsets;
    int *line_lengths;

    int anchor_offset;
} EDITORCTL_EXTRA;

typedef UINT32 EDITORCTL_UNICODE_CHAR;

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

BOOL editorctl_update_caret_pos (HWND hwnd);
BOOL editorctl_scroll_to (HWND hwnd, int x, int y);
BOOL editorctl_update_scroll_range (HWND hwnd);

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length);

BOOL editorctl_offset_to_line (HWND hwnd, int offset, int *line);
BOOL editorctl_offset_to_rc (HWND hwnd, int offset, int *row, int *col);

EDITORCTL_UNICODE_CHAR editorctl_get_next_char (const char **ptr);
void editorctl_set_next_char (EDITORCTL_UNICODE_CHAR ch, char **ptr);


#endif
