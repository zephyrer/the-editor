#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_lbuttonup (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    editorctl->last_click_time = GetTickCount64 ();
    editorctl->last_click_x = GET_X_LPARAM (lParam);
    editorctl->last_click_y = GET_Y_LPARAM (lParam);

    if (ReleaseCapture ()) goto error;

    return 0;
error:
    return -1;
}
