#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_delete_selection (EDITOR *editor)
{
    int from, to;

    assert (editor != NULL);

    from = min (editor->anchor_offset, editor->caret_offset);
    to = max (editor->anchor_offset, editor->caret_offset);
    if (!editor_replace_range (editor, from, to - from, 0, NULL, from)) goto error;

    return TRUE;

error:
    return FALSE;
}
