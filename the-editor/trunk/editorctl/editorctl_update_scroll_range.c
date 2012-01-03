#include <assert.h>
#include <windows.h>

#include "editorctl.h"

static BOOL do_update_range (EDITORCTL_EXTRA *extra)
{
    RECT r;
    SCROLLINFO si;
    int w, h;
    int width, height;
    int sx, sy;

    assert (extra != NULL);

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

    if (!editorctl_scroll_to (extra, sx, sy)) goto error;

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_update_scroll_range (EDITORCTL_EXTRA *extra)
{
    RECT r1, r2;

    assert (extra != NULL);

    if (!GetClientRect (extra->hwnd, &r1)) goto error;
    if (!do_update_range (extra)) goto error;
    if (!GetClientRect (extra->hwnd, &r2)) goto error;

    if (r1.left != r2.left || r1.top != r2.top || r1.right != r2.right || r1.bottom != r2.bottom)
    {
        if (!do_update_range (extra)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
