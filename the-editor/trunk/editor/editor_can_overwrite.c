#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "editor.h"

BOOL editor_can_overwrite (EDITOR *editor)
{
    assert (editor != NULL);

    if (editor->anchor_offset != editor->caret_offset) return FALSE;
    else
    {
        int offset = editor->caret_offset;
        if (!text_next_position (&editor->text, &offset)) return FALSE;
        else if (text_is_line_boundary (&editor->text, offset)) return FALSE;
        else return TRUE;
    }
}
