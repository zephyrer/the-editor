#include <assert.h>
#include <windows.h>

#include "../intlist/intlist.h"
#include "../text/text.h"
#include "../utils/utils.h"
#include "layout.h"

static BOOL update_wrap (LAYOUT *layout, int offset, int old_length, int new_length)
{
    return FALSE;
}

static BOOL update_nowrap (LAYOUT *layout, int offset, int old_length, int new_length)
{
    int start_row, o, l, c, row_after;
    INTLIST row_offsets;
    INTLIST row_widths;

    assert (layout != NULL);
    assert (layout->wrap == -1);
    assert (offset >= 0);
    assert (old_length >= 0);
    assert (offset + old_length <= layout->text->length);
    assert (new_length >= 0);

    row_offsets.data = NULL;
    row_widths.data = NULL;

    if (!intlist_initialize (&row_offsets, layout->heap, 16)) goto error;
    if (!intlist_initialize (&row_widths, layout->heap, 16)) goto error;

    start_row = layout_offset_to_row (layout, offset);
    row_after = layout_offset_to_row (layout, offset + old_length);
    if (layout->row_offsets.data [row_after] < offset + old_length)
        row_after += 1;

    // Very special case when we added '\n' after '\r' which was at the end line
    if (start_row > 0 && 
        layout->row_offsets.data [start_row] == offset && 
        !text_is_line_boundary (layout->text, offset))
    {
        start_row -= 1;
        c = layout->row_widths.data [start_row];
    }
    else
    {
        c = 0;
        text_move_to_offset (
            layout->text, 
            layout->row_offsets.data [start_row], 
            &c, 
            offset, 
            layout->tab_width);
    }

    o = offset;
    l = layout->text->length;
    while (TRUE)
    {
        BOOL new_line;

        new_line = text_next_row (layout->text, &o, &c, layout->tab_width);
        intlist_append (&row_widths, new_line ? c : c + 1);

        if (new_line)
        {
            if (o >= offset + new_length)
            {
                while (
                    row_after < layout->row_offsets.length && 
                    layout->row_offsets.data [row_after] + new_length - old_length < o)
                    row_after += 1;

                if (row_after < layout->row_offsets.length && 
                    layout->row_offsets.data [row_after] + new_length - old_length == o)
                    break;
            }

            if (!intlist_append (&row_offsets, o)) goto error;
        }
        else
        {
            assert (row_after == layout->row_offsets.length);
            break;
        }
    }

    layout->max_row_width = utils_update_max_int (
        layout->row_widths.data,
        layout->row_widths.length,
        start_row,
        row_after - start_row,
        row_widths.length,
        row_widths.data,
        layout->max_row_width);

    utils_increase_int (layout->row_offsets.data + row_after, layout->row_offsets.length - row_after, new_length - old_length);
    if (!intlist_replace_range (
        &row_offsets, 
        start_row + 1, row_after - start_row - 1, 
        row_offsets.length, row_offsets.data)) goto error;

    if (!intlist_replace_range (
        &row_widths,
        start_row, row_after - start_row,
        row_widths.length, row_widths.data)) goto error;

    if (!intlist_destroy (&row_offsets)) goto error;
    row_offsets.data = NULL;

    if (!intlist_destroy (&row_widths)) goto error;
    row_widths.data = NULL;

    return TRUE;

error:
    if (row_offsets.data != NULL)
        intlist_destroy (&row_offsets);
    if (row_widths.data != NULL)
        intlist_destroy (&row_widths);
    return FALSE;
}

BOOL layout_update (LAYOUT *layout, int offset, int old_length, int new_length)
{
    assert (layout != NULL);
    assert (offset >= 0);
    assert (old_length >= 0);
    assert (offset + old_length <= layout->text->length);
    assert (new_length >= 0);

    return layout->wrap == -1 ? update_wrap (layout, offset, old_length, new_length) : update_nowrap (layout, offset, old_length, new_length);
}
