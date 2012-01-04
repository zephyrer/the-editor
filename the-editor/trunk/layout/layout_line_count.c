#include <assert.h>
#include <windows.h>

#include "layout.h"

int layout_line_count (LAYOUT *layout)
{
    assert (layout != NULL);

    if (layout->wrap == -1)
        return layout->row_offsets.length;
    else
        return layout->line_rows.length;
}
