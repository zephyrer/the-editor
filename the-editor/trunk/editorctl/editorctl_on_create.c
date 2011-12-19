#include "editorctl.h"

LRESULT editorctl_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HANDLE heap = NULL;
    EDITORCTL_EXTRA *extra = NULL;

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((extra = (EDITORCTL_EXTRA *)HeapAlloc (heap, HEAP_ZERO_MEMORY, sizeof (EDITORCTL_EXTRA))) == NULL)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }

    extra->heap = heap;

    extra->cell_size.cx = 8;
    extra->cell_size.cy = 16;

    extra->row_count = 3000;
    extra->column_count = 20000;

    extra->tab_width = 8;
    extra->new_line = "\r\n";
    extra->new_line_length = 2;

    extra->line_count = 1;
    if ((extra->line_offsets = (int *)HeapAlloc (heap, HEAP_ZERO_MEMORY, 16 * sizeof (int))) == NULL)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }
    if ((extra->line_lengths = (int *)HeapAlloc (heap, HEAP_ZERO_MEMORY, 16 * sizeof (int))) == NULL)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }

    SetLastError (ERROR_SUCCESS);
    if (SetWindowLongPtr (hwnd, 0, (LONG)extra) == 0 && GetLastError () != ERROR_SUCCESS) goto error;

    if (!editorctl_update_scroll_range (hwnd)) goto error;
    if (!editorctl_scroll_to (hwnd, 100, 100)) goto error;

    return 0;
error:
    if (extra != NULL)
    {
        if (extra->line_offsets != NULL) HeapFree (heap, 0, extra->line_offsets);
        if (extra->line_lengths != NULL) HeapFree (heap, 0, extra->line_lengths);
        HeapFree (heap, 0, extra);
    }

    return -1;
}
