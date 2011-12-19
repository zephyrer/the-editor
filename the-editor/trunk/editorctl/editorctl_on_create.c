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

    extra->cell_size.cx = 80;
    extra->cell_size.cy = 160;

    extra->tab_width = 8;
    extra->new_line = "\r\n";
    extra->new_line_length = 2;

    SetLastError (ERROR_SUCCESS);
    if (SetWindowLongPtr (hwnd, 0, (LONG)extra) == 0 && GetLastError () != ERROR_SUCCESS) goto error;

    if (!editorctl_create_whitespace_icons (hwnd)) goto error;
    if (!editorctl_replace_range (hwnd, 0, 0, NULL, 0)) goto error;

    return 0;
error:
    if (extra != NULL)
    {
        if (extra->row_offsets != NULL) HeapFree (heap, 0, extra->row_offsets);
        HeapFree (heap, 0, extra);
    }

    return -1;
}
