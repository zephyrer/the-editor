#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_mousemove (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        if (!editorctl_move_mouse_cursor (editorctl, GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam), TRUE)) goto error;
    }

    return 0;
error:
    return -1;
}
