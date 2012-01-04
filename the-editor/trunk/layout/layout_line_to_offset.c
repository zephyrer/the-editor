#include <assert.h>
#include <windows.h>

#include "../intlist/intlist.h"
#include "layout.h"

int layout_line_to_offset (LAYOUT *layout, int line)
{
    assert (layout != NULL);
    assert (line >= 0);
    assert (
        (layout->wrap == -1 && line < layout->row_offsets.length) || 
        (layout->wrap != -1 && line < layout->line_rows.length));

    if (layout->wrap == -1)
        return layout->row_offsets.data [line];
    else
        return layout->row_offsets.data [layout->line_rows.data [line]];
}
