#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "layout.h"

int layout_offset_to_column (LAYOUT *layout, int offset, int row)
{
    int column;

    assert (layout != NULL);
    assert (offset >= 0);
    assert (offset <= layout->text->length);
    assert (row >= 0);
    assert (row < layout->row_offsets.length);
    assert (offset >= layout->row_offsets.data [row]);
    assert (row == layout->row_offsets.length - 1 || offset <= layout->row_offsets.data [row + 1]);

    column = 0;
    text_move_to_offset (layout->text, layout->row_offsets.data [row], &column, offset, layout->tab_width);
    return column;
}
