#include "editorctl.h"

LRESULT editorctl_on_hscroll (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SCROLLINFO si;
    RECT r;
    int x, w, width;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;

    w = r.right - r.left;
    width = editorctl->editor.layout.max_row_width * editorctl->cell_size.cx;

    switch (LOWORD (wParam))
    {
    case SB_LEFT:
        x = 0;
        break;
    case SB_RIGHT:
        x = width - w;
        break;
    case SB_LINELEFT:
        x = editorctl->scroll_location.x - editorctl->cell_size.cx * 8;
        break;
    case SB_LINERIGHT:
        x = editorctl->scroll_location.x + editorctl->cell_size.cx * 8;
        break;
    case SB_PAGELEFT:
        x = editorctl->scroll_location.x - max (editorctl->cell_size.cx, w);
        break;
    case SB_PAGERIGHT:
        x = editorctl->scroll_location.x + max (editorctl->cell_size.cx, w);
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.cbSize = sizeof (SCROLLINFO);
        si.fMask = SIF_TRACKPOS;
        GetScrollInfo (hwnd, SB_HORZ, &si);
        x = si.nTrackPos;
        break;
    default:
        x = editorctl->scroll_location.x;
    }

    x = min (width - w, x);
    x = max (0, x);

    if (!editorctl_scroll_to (editorctl, x, editorctl->scroll_location.y)) goto error;

    return 0;
error:
    return -1;
}
