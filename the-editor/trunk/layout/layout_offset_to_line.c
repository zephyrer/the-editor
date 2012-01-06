#include <assert.h>
#include <windows.h>

#include "layout.h"

int layout_offset_to_line (LAYOUT *layout, int offset)
{
    assert (layout != NULL);
    assert (offset >= 0);
    assert (offset <= layout->text->length);

    return layout_row_to_line (layout, layout_offset_to_row (layout, offset));
}
