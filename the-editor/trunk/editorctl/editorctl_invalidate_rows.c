#include "editorctl.h"

BOOL editorctl_invalidate_rows (HWND hwnd, int start_row, int end_row)
{
    EDITORCTL_EXTRA *extra;
    RECT r;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;
    r.top = start_row * extra->cell_size.cy - extra->scroll_location.y;
    r.bottom = end_row * extra->cell_size.cy - extra->scroll_location.y;

    if (!InvalidateRect (hwnd, &r, FALSE)) goto error;

    return TRUE;
error:
    return FALSE;
}
