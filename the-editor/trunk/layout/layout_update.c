#include <assert.h>
#include <windows.h>

#include "../intlist/intlist.h"
#include "../text/text.h"
#include "layout.h"

BOOL layout_update (LAYOUT *layout, int offset, int old_length, int new_length)
{
    int start_row, start_offset;

    assert (layout != NULL);
    assert (offset >= 0);
    assert (old_length >= 0);
    assert (offset + old_length <= layout->text.length);
    assert (new_length >= 0);

    start_row = utils_brearch_int (offset, layout->row_offsets.data, layout->row_offsets.length);
    if (start_row < 0)
        start_row = (-start_row - 1) - 1;

    assert (start_row >= 0);
    assert (start_row < layout->row_offsets.length);

}
