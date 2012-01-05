#include <assert.h>
#include <windows.h>

#include "editor.h"

static void add_dirty_range (EDITOR *editor, int from, int to)
{
    assert (editor != NULL);
    assert (from >= 0);
    assert (to >= 0) ;

    if (from != to)
    {
        int fr, tr;

        fr = layout_offset_to_row (&editor->layout, from);
        tr = layout_offset_to_row (&editor->layout, to);

        layout_add_dirty_range (&editor->layout, min (fr, tr), max (fr, tr) + 1);
    }
}

void editor_set_selection (EDITOR *editor, int anchor_offset, int caret_offset)
{
    assert (editor != NULL);
    assert (anchor_offset >= 0);
    assert (anchor_offset <= editor->text.length);
    assert (caret_offset >= 0);
    assert (caret_offset <= editor->text.length);

    if (anchor_offset == caret_offset)
    {
        if (editor->anchor_offset != editor->caret_offset)
            add_dirty_range (editor, editor->anchor_offset, editor->caret_offset);
    }
    else
    {
        if (editor->anchor_offset == editor->caret_offset)
            add_dirty_range (editor, anchor_offset, caret_offset);
        else
        {
            if (max (editor->anchor_offset, editor->caret_offset) < min (anchor_offset, caret_offset))
            {
                add_dirty_range (editor, editor->anchor_offset, editor->caret_offset);
                add_dirty_range (editor, anchor_offset, caret_offset);
            }
            else
            {
                add_dirty_range (editor, min (editor->anchor_offset, editor->caret_offset), min (anchor_offset, caret_offset));
                add_dirty_range (editor, max (editor->anchor_offset, editor->caret_offset), max (anchor_offset, caret_offset));
            }
        }
    }

    editor->anchor_offset = anchor_offset;
    editor->caret_offset = caret_offset;
    editor->caret_column = -1;
}
