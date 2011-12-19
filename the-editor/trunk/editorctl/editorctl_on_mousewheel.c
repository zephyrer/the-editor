#include "editorctl.h"

LRESULT editorctl_on_mousewheel (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra = NULL;
    RECT r;
    int y, h, height;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    GetClientRect (hwnd, &r);

    h = r.bottom - r.top;
    height = extra->row_count * extra->cell_size.cy;

    y = extra->scroll_location.y - GET_WHEEL_DELTA_WPARAM (wParam) * extra->cell_size.cy * 3 / WHEEL_DELTA;

    y = min (height - h, y);
    y = max (0, y);

    if (!editorctl_scroll_to (hwnd, extra->scroll_location.x, y)) goto error;

    return 0;
error:
    return -1;
}
