#include <assert.h>
#include <windows.h>

#include "layout.h"

void layout_reset_dirty_rows (LAYOUT *layout)
{
    assert (layout != NULL);

    layout->first_dirty_row = 0;
    layout->last_dirty_row = 0;
}
