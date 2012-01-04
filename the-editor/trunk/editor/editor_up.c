#include <assert.h>
#include <windows.h>

#include "../layout/layout.h"
#include "editor.h"

void editor_up (EDITOR *editor, int rows, BOOL selecting)
{
    int row;

    assert (editor != NULL);
    assert (rows > 0);

    row = layout_offset_to_row (&editor->layout, editor->caret_offset);
    if (row > 0)
    {
        int column;

        if (editor->caret_column >= 0) column = editor->caret_column;
        else column = layout_offset_to_column (&editor->layout, editor->caret_offset, row);

        editor_move_caret (
            editor, 
            layout_row_column_to_offset (
                &editor->layout, 
                max (row - rows, 0), 
                column), 
            selecting);
        editor->caret_column = column;
    }
    else
        editor_move_caret (editor, 0, selecting);
}
