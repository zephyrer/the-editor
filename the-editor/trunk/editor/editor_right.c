#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

void editor_right (EDITOR *editor, BOOL selecting)
{
    int offset;

    assert (editor != NULL);

    offset = editor->caret_offset;
    if (text_next_position (&editor->text, &offset))
        editor_move_caret (editor, offset, selecting);
}
