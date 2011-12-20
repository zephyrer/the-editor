#include "editorctl.h"

BOOL editorctl_update_caret_pos (HWND hwnd)
{
    if (GetFocus () == hwnd)
    {
        EDITORCTL_EXTRA *extra;
        int row, col;

        if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

        if (!editorctl_offset_to_rc (hwnd, extra->caret_offset, &row, &col)) goto error;

        SetCaretPos (col * extra->cell_size.cx - extra->scroll_location.x, row * extra->cell_size.cy - extra->scroll_location.y);
    }

    return TRUE;
error:
    return FALSE;
}
