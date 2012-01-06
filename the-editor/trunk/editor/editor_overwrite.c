#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_overwrite (EDITOR *editor, int length, const char data [])
{
    int offset;

    assert (editor != NULL);
    assert (length > 0);
    assert (data != NULL);
    assert (editor_can_overwrite (editor));

    offset = editor->caret_offset;
    assert (text_next_position (&editor->text, &offset));

    return editor_replace_range (editor, editor->caret_offset, offset - editor->caret_offset, length, data, editor->caret_offset + length);
}
