#include <assert.h>
#include <windows.h>

#include "../intlist/intlist.h"
#include "../text/text.h"
#include "layout.h"

BOOL layout_initialize (LAYOUT *layout, HANDLE heap, TEXT *text, int tab_width, int wrap, int min_wrap)
{
    int fake;

    assert (layout != NULL);
    assert (heap != NULL);
    assert (text != NULL);
    assert (tab_width > 0);
    assert (wrap == -1 || wrap > 0);
    assert (wrap == -1 || min_wrap >= 0);
    assert (wrap == -1 || min_wrap <= wrap);
    assert (wrap != -1 || min_wrap == -1);

    ZeroMemory (layout, sizeof (LAYOUT));

    if (!intlist_initialize (&layout->row_offsets, heap, 16)) goto error;
    if (!intlist_initialize (&layout->row_widths, heap, 16)) goto error;
    if (wrap != -1 && !intlist_initialize (&layout->line_rows, heap, 16)) goto error;

    layout->max_row_width = 0;
    layout->tab_width = tab_width;
    layout->wrap = wrap;
    layout->min_wrap = min_wrap;
    layout->text = text;
    layout->heap = heap;

    if (!intlist_append (&layout->row_offsets, 0)) goto error;
    if (!intlist_append (&layout->row_widths, 0)) goto error;
    if (wrap != -1 && !intlist_append (&layout->line_rows, 0)) goto error;

    if (!layout_update (layout, 0, 0, text->length, &fake, &fake)) goto error;

    return TRUE;

error:
    if (layout->row_offsets.data != NULL)
        intlist_destroy (&layout->row_offsets);
    if (layout->row_widths.data != NULL)
        intlist_destroy (&layout->row_widths);
    if (layout->line_rows.data != NULL)
        intlist_destroy (&layout->line_rows);

    return FALSE;
}
