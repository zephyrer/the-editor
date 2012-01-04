#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_replace_selection (EDITOR *editor, int length, const char data [])
{
    int from, to;

    assert (editor != NULL);
    assert (length > 0);
    assert (data != NULL);

    from = min (editor->anchor_offset, editor->caret_offset);
    to = max (editor->anchor_offset, editor->caret_offset);
    if (!editor_replace_range (editor, from, to - from, length, data, from + length)) goto error;

    return TRUE;

error:
    return FALSE;
}
