#include "editorctl.h"

static BOOL do_update_range (HWND hwnd)
{
    EDITORCTL_EXTRA *extra;
    RECT r;
    SCROLLINFO si;
    int w, h;
    int width, height;
    int sx, sy;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    if (!GetClientRect (hwnd, &r)) goto error;

    w = r.right - r.left;
    width = extra->column_count * extra->cell_size.cx;
    sx = extra->scroll_location.x;
    if (sx + w > width)
        sx = max (0, width - w);

    h = r.bottom - r.top;
    height = extra->row_count * extra->cell_size.cy;
    sy = extra->scroll_location.y;
    if (sy + h > height)
        sy = max (0, height - h);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = extra->column_count * extra->cell_size.cx;
    si.nPage = w;
    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = extra->row_count * extra->cell_size.cy;
    si.nPage = h;
    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

    if (!editorctl_scroll_to (hwnd, sx, sy)) goto error;

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_update_scroll_range (HWND hwnd)
{
    RECT r, r1;

    if (!GetClientRect (hwnd, &r)) goto error;

    if (!do_update_range (hwnd)) goto error;

    if (!GetClientRect (hwnd, &r1)) goto error;

    if (r.left != r1.left || r.top != r1.top || r.right != r1.right || r.bottom != r1.bottom)
    {
        if (!do_update_range (hwnd)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
