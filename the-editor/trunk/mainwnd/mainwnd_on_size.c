#include "mainwnd.h"

LRESULT mainwnd_on_size (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MAINWND_EXTRA *extra;
    RECT r1, r2;

    if ((extra = (MAINWND_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (SendMessage (extra->status_bar, WM_SIZE, 0, 0) != S_OK) goto error;

    GetClientRect (hwnd, &r1);
    GetWindowRect (extra->status_bar, &r2);

    MoveWindow (extra->editor, r1.left, r1.top, r1.right - r1.left, r1.bottom - r1.top - (r2.bottom - r2.top), TRUE);

    return 0;
error:
    return -1;
}
