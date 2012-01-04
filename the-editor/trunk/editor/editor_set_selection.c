#include <assert.h>
#include <windows.h>

#include "editor.h"

void editor_set_selection (EDITOR *editor, int anchor_offset, int caret_offset)
{
    assert (editor != NULL);
    assert (anchor_offset >= 0);
    assert (anchor_offset <= editor->text.length);
    assert (caret_offset >= 0);
    assert (caret_offset <= editor->text.length);

    editor->anchor_offset = anchor_offset;
    editor->caret_offset = caret_offset;
    editor->caret_column = -1;
}
