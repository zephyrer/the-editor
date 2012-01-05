#include "editorctl.h"

LRESULT editorctl_on_setfocus (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!CreateCaret (hwnd, (HBITMAP) NULL, editorctl->overwrite ? editorctl->cell_size.cx : 1, editorctl->cell_size.cy)) goto error;
    if (!editorctl_update_caret_pos (editorctl)) goto error;
    if (!ShowCaret (editorctl->hwnd)) goto error;

    return 0;
error:
    return -1;
}
