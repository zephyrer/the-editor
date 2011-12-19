#include "editorctl.h"

BOOL editorctl_update_scroll_range (HWND hwnd)
{
    EDITORCTL_EXTRA *extra;
    SCROLLINFO si;
    RECT r;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = extra->column_count * extra->cell_size.cx;
    si.nPage = r.right - r.left;
    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = extra->row_count * extra->cell_size.cy;
    si.nPage = r.bottom - r.top;
    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

    return TRUE;
error:
    return FALSE;
}
