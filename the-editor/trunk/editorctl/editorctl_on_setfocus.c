#include "editorctl.h"

LRESULT editorctl_on_setfocus (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra = NULL;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    if (!CreateCaret (hwnd, (HBITMAP) NULL, extra->overwrite ? extra->cell_size.cx : 1, extra->cell_size.cy)) goto error;
    if (!editorctl_update_caret_pos (hwnd)) goto error;
    if (!ShowCaret (hwnd)) goto error;

    return 0;
error:
    return -1;
}
