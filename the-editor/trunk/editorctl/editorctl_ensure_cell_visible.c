#include "editorctl.h"

BOOL editorctl_ensure_cell_visible (HWND hwnd, int row, int col)
{
    EDITORCTL_EXTRA *extra;
    int x, y, sx, sy;
    RECT r;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    x = col * extra->cell_size.cx;
    y = row * extra->cell_size.cy;
    sx = extra->scroll_location.x;
    sy = extra->scroll_location.y;

    if (!GetClientRect (hwnd, &r)) goto error;

    if (x + extra->cell_size.cx > r.right + sx)
        sx = x + extra->cell_size.cx - r.right;

    if (x < r.left + sx)
        sx = x - r.left;

    if (y + extra->cell_size.cy > r.bottom + sy)
        sy = y + extra->cell_size.cy - r.bottom;

    if (y < r.top + sy)
        sy = y - r.top;

    if (!editorctl_scroll_to (hwnd, sx, sy)) goto error;

    return TRUE;
error:
    return FALSE;
}
