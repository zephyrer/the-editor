#include "editorctl.h"

LRESULT editorctl_on_vscroll (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SCROLLINFO si;
    RECT r;
    int y, h, height;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;

    h = r.bottom - r.top;
    height = editorctl->editor.layout.row_offsets.length * editorctl->cell_size.cy;

    switch (LOWORD (wParam))
    {
    case SB_LEFT:
        y = 0;
        break;
    case SB_RIGHT:
        y = height - h;
        break;
    case SB_LINELEFT:
        y = editorctl->scroll_location.y - editorctl->cell_size.cy;
        break;
    case SB_LINERIGHT:
        y = editorctl->scroll_location.y + editorctl->cell_size.cy;
        break;
    case SB_PAGELEFT:
        y = editorctl->scroll_location.y - max (editorctl->cell_size.cy, h);
        break;
    case SB_PAGERIGHT:
        y = editorctl->scroll_location.y + max (editorctl->cell_size.cy, h);
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.cbSize = sizeof (SCROLLINFO);
        si.fMask = SIF_TRACKPOS;
        GetScrollInfo (hwnd, SB_VERT, &si);
        y = si.nTrackPos;
        break;
    default:
        y = editorctl->scroll_location.y;
    }

    y = min (height - h, y);
    y = max (0, y);

    if (!editorctl_scroll_to (editorctl, editorctl->scroll_location.x, y)) goto error;

    return 0;
error:
    return -1;
}
