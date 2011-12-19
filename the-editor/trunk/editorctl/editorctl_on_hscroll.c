#include "editorctl.h"

LRESULT editorctl_on_hscroll (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    SCROLLINFO si;
    RECT r;
    int x, w, width;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;

    w = r.right - r.left;
    width = extra->column_count * extra->cell_size.cx;

    switch (LOWORD (wParam))
    {
    case SB_LEFT:
        x = 0;
        break;
    case SB_RIGHT:
        x = width - w;
        break;
    case SB_LINELEFT:
        x = extra->scroll_location.x - extra->cell_size.cx * 8;
        break;
    case SB_LINERIGHT:
        x = extra->scroll_location.x + extra->cell_size.cx * 8;
        break;
    case SB_PAGELEFT:
        x = extra->scroll_location.x - max (extra->cell_size.cx, w);
        break;
    case SB_PAGERIGHT:
        x = extra->scroll_location.x + max (extra->cell_size.cx, w);
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.cbSize = sizeof (SCROLLINFO);
        si.fMask = SIF_TRACKPOS;
        GetScrollInfo (hwnd, SB_HORZ, &si);
        x = si.nTrackPos;
        break;
    default:
        x = extra->scroll_location.x;
    }

    x = min (width - w, x);
    x = max (0, x);

    if (!editorctl_scroll_to (hwnd, x, extra->scroll_location.y)) goto error;

    return 0;
error:
    return -1;
}
