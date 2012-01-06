#include <assert.h>
#include <windows.h>

#include "../utils/utils.h"
#include "layout.h"

int layout_row_to_line (LAYOUT *layout, int row)
{
    assert (layout != NULL);
    assert (row >= 0);
    assert (row < layout->row_offsets.length);

    if (layout->wrap == -1)
        return row;
    else
    {
        int line;

        line = utils_bsearch_int (row, layout->line_rows.data, layout->line_rows.length);
        if (line < 0)
            line = (-line - 1) - 1;

        assert (line >= 0);
        assert (line < layout->line_rows.length);

        return line;
    }
}
