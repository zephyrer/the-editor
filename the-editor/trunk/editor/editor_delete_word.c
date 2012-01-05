#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_delete_word (EDITOR *editor)
{
    int offset;

    assert (editor != NULL);
    assert (editor->anchor_offset == editor->caret_offset);

    offset = editor->caret_offset;
    if (text_next_word_boundary (&editor->text, &offset))
        return editor_replace_range (editor, editor->caret_offset, offset - editor->caret_offset, 0, NULL, editor->caret_offset);
    else return TRUE;
}
