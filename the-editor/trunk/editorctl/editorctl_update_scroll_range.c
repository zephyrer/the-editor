#include "editorctl.h"

BOOL editorctl_update_scroll_range (HWND hwnd)
{
    EDITORCTL_EXTRA *extra = NULL;
    SCROLLINFO si;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_RANGE;
    si.nMin = 0;
    si.nMax = extra->column_count * extra->cell_size.cx;
    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_RANGE;
    si.nMin = 0;
    si.nMax = extra->row_count * extra->cell_size.cy;
    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

    return TRUE;
error:
    return FALSE;
}
