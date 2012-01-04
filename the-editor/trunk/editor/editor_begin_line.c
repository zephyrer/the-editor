#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"
#include "editor.h"

void editor_begin_line (EDITOR *editor, BOOL selecting)
{
    int line, line_offset, nonspace_offset;

    assert (editor != NULL);

    line = layout_offset_to_line (&editor->layout, editor->caret_offset);
    line_offset = layout_line_to_offset (&editor->layout, line);
    nonspace_offset = line_offset;
    if (text_next_nonspace (&editor->text, &nonspace_offset) && nonspace_offset != editor->caret_offset)
        editor_move_caret (editor, nonspace_offset, selecting);
    else 
        editor_move_caret (editor, line_offset, selecting);
}
