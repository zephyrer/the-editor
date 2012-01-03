#include <windows.h>

#include "editorctl.h"

LRESULT editorctl_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HANDLE heap = NULL;
    EDITORCTL_EXTRA *extra = NULL;

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((extra = (EDITORCTL_EXTRA *)HeapAlloc (heap, HEAP_ZERO_MEMORY, sizeof (EDITORCTL_EXTRA))) == NULL) goto heap_error;

    extra->hwnd = hwnd;
    extra->heap = heap;

    extra->cell_size.cx = 8;
    extra->cell_size.cy = 16;

    extra->tab_width = 8;
    extra->new_line = "\r\n";
    extra->new_line_length = 2;
    extra->word_wrap_column = 85;
    extra->word_wrap_min_column = 75;

    extra->row_count = 1;
    extra->column_count = 1;

    if (!utils_initialize_int_list (&extra->row_offsets, 16)) goto error;
    if (!utils_initialize_int_list (&extra->row_widths, 16)) goto error;
    if (!utils_initialize_int_list (&extra->line_rows, 16)) goto error;

    extra->row_offsets.data [0] = 0;
    extra->row_widths.data [0] = 1;
    extra->line_rows.data [0] = 0;

    SetLastError (ERROR_SUCCESS);
    if (SetWindowLongPtr (hwnd, 0, (LONG)extra) == 0 && GetLastError () != ERROR_SUCCESS) goto error;

    if (!editorctl_create_whitespace_icons (hwnd)) goto error;
    if (!editorctl_replace_range (hwnd, 0, 0, NULL, 0, 0)) goto error;

    return 0;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);

error:
    if (extra != NULL)
    {
        if (extra->row_offsets.data != NULL) utils_destroy_int_list (&extra->row_offsets);
        if (extra->row_widths.data != NULL) utils_destroy_int_list (&extra->row_widths);
        if (extra->line_rows.data != NULL) utils_destroy_int_list (&extra->line_rows);
        HeapFree (heap, 0, extra);
    }

    SetWindowLongPtr (hwnd, 0, (LPARAM)NULL);

    return -1;
}
