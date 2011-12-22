#include "editorctl.h"

LRESULT editorctl_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    if (extra != NULL)
    {
        if (extra->row_offsets != NULL)
            HeapFree (extra->heap, 0, extra->row_offsets);
        if (extra->row_widths != NULL)
            HeapFree (extra->heap, 0, extra->row_widths);
        if (extra->whitespace_icons != NULL)
            DeleteObject (extra->whitespace_icons);
        HeapFree (extra->heap, 0, extra);
    }

    return 0;
error:
    return -1;
}
