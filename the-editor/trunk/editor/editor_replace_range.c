#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_replace_range (EDITOR *editor, int offset, int length, int replacement_length, const char replacement [], int caret_offset)
{
    assert (editor != NULL);
    assert (offset >= 0);
    assert (length >= 0);
    assert (offset + length <= editor->text.length);
    assert (replacement_length >= 0);
    assert (replacement_length == 0 || replacement != NULL);

    if (!text_replace_range (&editor->text, offset, length, replacement_length, replacement)) goto error;
    if (!layout_update (&editor->layout, offset, length, replacement_length)) goto error;
    editor_move_caret (editor, caret_offset, FALSE);

    return TRUE;
error:
    return FALSE;
}
