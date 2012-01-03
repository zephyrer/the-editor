#include <assert.h>
#include <windows.h>

#include "../utils/utils.h"
#include "layout.h"

int layout_offset_to_line (LAYOUT *layout, int offset)
{
    int row, line;

    assert (layout != NULL);
    assert (offset >= 0);
    assert (offset <= layout->text->length);

    row = layout_offset_to_row (layout, offset);

    if (layout->wrap == -1)
        line = row;
    else
    {
        int line;

        line = utils_bsearch_int (row, layout->line_rows.data, layout->line_rows.length);
        if (line < 0)
            line = (-line - 1) - 1;

        assert (line >= 0);
        assert (line < layout->line_rows.length);
    }

    return line;
}
