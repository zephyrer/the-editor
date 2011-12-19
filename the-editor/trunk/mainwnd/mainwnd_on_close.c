#include "mainwnd.h"

LRESULT mainwnd_on_close (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!DestroyWindow (hwnd)) goto error;

    return 0;
error:
    return -1;
}
