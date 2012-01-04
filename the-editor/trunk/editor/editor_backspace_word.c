#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_backspace_word (EDITOR *editor)
{
    int offset;

    assert (editor != NULL);
    assert (editor->anchor_offset == editor->caret_offset);

    offset = editor->caret_offset;
    while (text_prev_position (&editor->text, &offset))
    {
        if (text_is_word_boundary (&editor->text, offset)) break;
    }

    return editor_replace_range (editor, offset, editor->caret_offset - offset, 0, NULL, offset);
}
