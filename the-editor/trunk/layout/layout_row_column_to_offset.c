#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "layout.h"

int layout_row_column_to_offset (LAYOUT *layout, int row, int column)
{
    int offset, c;

    assert (layout != NULL);
    assert (row >= 0);
    assert (row < layout->row_offsets.length);
    assert (column >= 0);

    offset = layout->row_offsets.data [row];
    column = 0;
    text_move_to_column (layout->text, &offset, &c, column, layout->tab_width);
    return c;
}
