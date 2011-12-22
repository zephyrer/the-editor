#include "editorctl.h"

LRESULT editorctl_on_size (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!editorctl_update_scroll_range (hwnd)) goto error;

    return 0;
error:
    return -1;
}
