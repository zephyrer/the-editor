#include "editorctl.h"

BOOL editorctl_move_cursor (HWND hwnd, int offset)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    extra->caret_offset = offset;
    extra->anchor_offset = offset;
    if (!editorctl_update_caret_pos (hwnd)) goto error;

    return TRUE;
error:
    return FALSE;
}
