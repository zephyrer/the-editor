#include "editorctl.h"

LRESULT editorctl_on_size (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!editorctl_update_scroll_range (editorctl)) goto error;

    return 0;
error:
    return -1;
}
