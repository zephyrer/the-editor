#include "mainwnd.h"

LRESULT mainwnd_on_setfocus (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MAINWND_EXTRA *extra;

    if ((extra = (MAINWND_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (SetFocus (extra->editor) == NULL) goto error;

    return 0;
error:
    return -1;
}
