#include "editorctl.h"

BOOL editorctl_invalidate_rows (EDITORCTL *editorctl, int start_row, int end_row)
{
    int top, bottom;
    RECT r;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;
    top = start_row * editorctl->cell_size.cy - editorctl->scroll_location.y;
    bottom = end_row * editorctl->cell_size.cy - editorctl->scroll_location.y;

    r.top = max (r.top, top);
    r.bottom = min (r.bottom, bottom);

    if (!InvalidateRect (editorctl->hwnd, &r, FALSE)) goto error;

    return TRUE;
error:
    return FALSE;
}
