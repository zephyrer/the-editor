#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

void editor_end_text (EDITOR *editor, BOOL selecting)
{
    assert (editor != NULL);

    editor_move_caret (editor, editor->text.length, selecting);
}
