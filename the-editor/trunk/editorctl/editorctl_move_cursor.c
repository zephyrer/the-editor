#include "editorctl.h"

BOOL editorctl_move_cursor (HWND hwnd, int offset, BOOL selecting)
{
    EDITORCTL_EXTRA *extra;
    int row, col;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (selecting)
    {
        int old_row;
        int row;

        if (!editorctl_offset_to_row (hwnd, extra->caret_offset, &old_row)) goto error;
        if (!editorctl_offset_to_row (hwnd, offset, &row)) goto error;
        extra->caret_offset = offset;

        if (!editorctl_invalidate_rows (hwnd, min (old_row, row), max (old_row, row) + 1)) goto error;
    }
    else if (extra->anchor_offset != extra->caret_offset)
    {
        int anchor_row;
        int caret_row;

        if (!editorctl_offset_to_row (hwnd, extra->anchor_offset, &anchor_row)) goto error;
        if (!editorctl_offset_to_row (hwnd, extra->caret_offset, &caret_row)) goto error;
        extra->caret_offset = offset;
        extra->anchor_offset = offset;

        if (!editorctl_invalidate_rows (hwnd, min (anchor_row, caret_row), max (anchor_row, caret_row) + 1)) goto error;
    }
    else
    {
        extra->caret_offset = offset;
        extra->anchor_offset = offset;
    }

    extra->caret_column = -1;

    if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;
    if (!editorctl_ensure_cell_visible (hwnd, row, col)) goto error;

    if (!editorctl_update_caret_pos (hwnd)) goto error;

    return TRUE;
error:
    return FALSE;
}
