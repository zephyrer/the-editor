#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_lbuttonup (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    extra->last_click_time = GetTickCount64 ();
    extra->last_click_x = GET_X_LPARAM (lParam);
    extra->last_click_y = GET_Y_LPARAM (lParam);

    if (ReleaseCapture ()) goto error;

    return 0;
error:
    return -1;
}
