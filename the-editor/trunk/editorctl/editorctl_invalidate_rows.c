#include "editorctl.h"

BOOL editorctl_invalidate_rows (EDITORCTL *editorctl, int start_row, int end_row)
{
    RECT r;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;
    r.top = start_row * editorctl->cell_size.cy - editorctl->scroll_location.y;
    r.bottom = end_row * editorctl->cell_size.cy - editorctl->scroll_location.y;

    if (!InvalidateRect (editorctl->hwnd, &r, FALSE)) goto error;

    return TRUE;
error:
    return FALSE;
}
