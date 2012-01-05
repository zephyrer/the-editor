#include "editorctl.h"

BOOL editorctl_update_caret_pos (EDITORCTL *editorctl)
{
    if (GetFocus () == editorctl->hwnd)
    {
        int row, col;

        row = layout_offset_to_row (&editorctl->editor.layout, editorctl->editor.caret_offset);
        col = layout_offset_to_column (&editorctl->editor.layout, editorctl->editor.caret_offset, row);

        if (!SetCaretPos (col * editorctl->cell_size.cx - editorctl->scroll_location.x, row * editorctl->cell_size.cy - editorctl->scroll_location.y)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
