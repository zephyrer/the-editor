#include <assert.h>
#include <windows.h>

#include "../intlist/intlist.h"
#include "layout.h"

BOOL layout_destroy (LAYOUT *layout)
{
    assert (layout != NULL);

    if (!intlist_destroy (&layout->row_offsets)) goto error;
    if (!intlist_destroy (&layout->row_widths)) goto error;
    if (layout->wrap != -1 && !intlist_destroy (&layout->line_rows)) goto error;

#ifdef _DEBUG
    ZeroMemory (layout, sizeof (LAYOUT));
#endif

    return TRUE;

error:
    return FALSE;
}
