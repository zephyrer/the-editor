#include "editorctl.h"

LRESULT editorctl_on_mousewheel (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    RECT r;
    int y, h, height;

    GetClientRect (editorctl->hwnd, &r);

    h = r.bottom - r.top;
    height = editorctl->editor.layout.row_offsets.length * editorctl->cell_size.cy;

    y = editorctl->scroll_location.y - GET_WHEEL_DELTA_WPARAM (wParam) * editorctl->cell_size.cy * 3 / WHEEL_DELTA;

    y = min (height - h, y);
    y = max (0, y);

    if (!editorctl_scroll_to (editorctl, editorctl->scroll_location.x, y)) goto error;

    return 0;
error:
    return -1;
}
