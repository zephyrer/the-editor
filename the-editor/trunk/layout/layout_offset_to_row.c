#include <assert.h>
#include <windows.h>

#include "../utils/utils.h"
#include "layout.h"

int layout_offset_to_row (LAYOUT *layout, int offset)
{
    int row;

    assert (layout != NULL);
    assert (offset >= 0);

    row = utils_bsearch_int (offset, layout->row_offsets.data, layout->row_offsets.length);
    if (row < 0)
        row = (-row - 1) - 1;

    assert (row >= 0);
    assert (row < layout->row_offsets.length);

    return row;
}
