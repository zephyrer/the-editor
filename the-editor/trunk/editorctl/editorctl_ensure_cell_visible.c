#include "editorctl.h"

BOOL editorctl_ensure_cell_visible (EDITORCTL *editorctl, int row, int col)
{
    int x, y, sx, sy;
    RECT r;

    x = col * editorctl->cell_size.cx;
    y = row * editorctl->cell_size.cy;
    sx = editorctl->scroll_location.x;
    sy = editorctl->scroll_location.y;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;

    if (x + editorctl->cell_size.cx > r.right + sx)
        sx = x + editorctl->cell_size.cx - r.right;

    if (x < r.left + sx)
        sx = x - r.left;

    if (y + editorctl->cell_size.cy > r.bottom + sy)
        sy = y + editorctl->cell_size.cy - r.bottom;

    if (y < r.top + sy)
        sy = y - r.top;

    if (!editorctl_scroll_to (editorctl, sx, sy)) goto error;

    return TRUE;
error:
    return FALSE;
}
