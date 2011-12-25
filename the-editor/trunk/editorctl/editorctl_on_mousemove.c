#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_mousemove (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        EDITORCTL_EXTRA *extra;

        if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

        if (!editorctl_move_mouse_cursor (hwnd, GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam), TRUE)) goto error;
    }

    return 0;
error:
    return -1;
}
