#include "editorctl.h"

LRESULT editorctl_on_size (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = extra->column_count * extra->cell_size.cx;
    si.nPage = w;
    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = extra->row_count * extra->cell_size.cy;
    si.nPage = h;
    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

    if (!editorctl_scroll_to (hwnd, sx, sy)) goto error;

    return 0;
error:
    return -1;
}
