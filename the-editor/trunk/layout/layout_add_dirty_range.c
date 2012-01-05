#include <assert.h>
#include <windows.h>

#include "layout.h"

void layout_add_dirty_range (LAYOUT *layout, int first_dirty_row, int last_dirty_row)
{
    assert (layout != NULL);
    assert (first_dirty_row >= 0);
    assert (last_dirty_row >= first_dirty_row);

    if (first_dirty_row != last_dirty_row)
    {
        if (layout->first_dirty_row == layout->last_dirty_row)
        {
            layout->first_dirty_row = first_dirty_row;
            layout->last_dirty_row = last_dirty_row;
        }
        else
        {
            layout->first_dirty_row = min (layout->first_dirty_row, first_dirty_row);
            layout->last_dirty_row = max (layout->last_dirty_row, last_dirty_row);
        }
    }
}
