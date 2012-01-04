#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_insert (EDITOR *editor, int length, const char data [])
{
    assert (editor != NULL);
    assert (editor->anchor_offset == editor->caret_offset);
    assert (length > 0);
    assert (data != NULL);

    if (!editor_replace_range (editor, editor->caret_offset, 0, length, data, editor->caret_offset + length)) goto error;

error:
    return FALSE;
}
