#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_left_word (EDITOR *editor, BOOL selecting)
{
    int offset;

    assert (editor != NULL);

    offset = editor->caret_offset;
    while (text_prev_position (&editor->text, &offset))
    {
        if (text_is_word_boundary (&editor->text, offset)) break;
    }

    return editor_move_caret (editor, offset, selecting);
}
