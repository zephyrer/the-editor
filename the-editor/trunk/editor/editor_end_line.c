#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"
#include "editor.h"

BOOL editor_end_line (EDITOR *editor, BOOL selecting)
{
    int line;

    assert (editor != NULL);

    line = layout_offset_to_line (&editor->layout, editor->caret_offset);
    if (line < layout_line_count (&editor->layout) - 1)
    {
        int offset;

        offset = layout_line_to_offset (&editor->layout, line + 1);
        assert (text_prev_position (&editor->text, &offset));
        return editor_move_caret (editor, offset, selecting);
    }
    else
        return editor_move_caret (editor, editor->text.length, selecting);
}
