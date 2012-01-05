#include <assert.h>
#include <windows.h>

#include "editorctl.h"

BOOL editorctl_update (EDITORCTL *editorctl)
{
    assert (editorctl != NULL);

    if (editorctl->editor.layout.first_dirty_row != editorctl->editor.layout.last_dirty_row)
        if (!editorctl_invalidate_rows (editorctl, editorctl->editor.layout.first_dirty_row, editorctl->editor.layout.last_dirty_row)) goto error;

    return TRUE;

error:
    return FALSE;
}
