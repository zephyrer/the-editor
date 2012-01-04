#include <assert.h>
#include <windows.h>

#include "editor.h"

void editor_move_caret (EDITOR *editor, int offset, BOOL selecting)
{
    assert (editor != NULL);
    assert (offset >= 0);
    assert (offset <= editor->text.length);

    if (!selecting) editor->anchor_offset = offset;
    editor->caret_offset = offset;
    editor->caret_column = -1;
}
