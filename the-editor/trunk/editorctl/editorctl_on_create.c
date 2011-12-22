#include "editorctl.h"

LRESULT editorctl_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HANDLE heap = NULL;
    EDITORCTL_EXTRA *extra = NULL;

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((extra = (EDITORCTL_EXTRA *)HeapAlloc (heap, HEAP_ZERO_MEMORY, sizeof (EDITORCTL_EXTRA))) == NULL) goto heap_error;

    extra->heap = heap;

    extra->cell_size.cx = 8;
    extra->cell_size.cy = 16;

    extra->tab_width = 8;
    extra->new_line = "\r\n";
    extra->new_line_length = 2;
//    extra->word_wrap_column = 85;
//    extra->word_wrap_min_column = 75;

    extra->row_count = 1;
    extra->column_count = 1;
    if ((extra->row_offsets = (int *)HeapAlloc (heap, 0, 16 * sizeof (int))) == NULL) goto heap_error;
    if ((extra->row_widths = (int *)HeapAlloc (heap, 0, 16 * sizeof (int))) == NULL) goto heap_error;
    extra->row_offsets [0] = 0;
    extra->row_widths [0] = 1;

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
        if (extra->row_offsets != NULL) HeapFree (heap, 0, extra->row_offsets);
        if (extra->row_widths != NULL) HeapFree (heap, 0, extra->row_widths);
        HeapFree (heap, 0, extra);
    }

    SetWindowLongPtr (hwnd, 0, (LPARAM)NULL);

    return -1;
}
