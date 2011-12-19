#include "editorctl.h"

LRESULT editorctl_on_vscroll (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    SCROLLINFO si;
    RECT r;
    int y, h, height;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;

    h = r.bottom - r.top;
    height = extra->row_count * extra->cell_size.cy;

    switch (LOWORD (wParam))
    {
    case SB_LEFT:
        y = 0;
        break;
    case SB_RIGHT:
        y = height - h;
        break;
    case SB_LINELEFT:
        y = extra->scroll_location.y - extra->cell_size.cy;
        break;
    case SB_LINERIGHT:
        y = extra->scroll_location.y + extra->cell_size.cy;
        break;
    case SB_PAGELEFT:
        y = extra->scroll_location.y - max (extra->cell_size.cy, h);
        break;
    case SB_PAGERIGHT:
        y = extra->scroll_location.y + max (extra->cell_size.cy, h);
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.cbSize = sizeof (SCROLLINFO);
        si.fMask = SIF_TRACKPOS;
        GetScrollInfo (hwnd, SB_VERT, &si);
        y = si.nTrackPos;
        break;
    default:
        y = extra->scroll_location.y;
    }

    y = min (height - h, y);
    y = max (0, y);

    if (!editorctl_scroll_to (hwnd, extra->scroll_location.x, y)) goto error;

    return 0;
error:
    return -1;
}
