#include "editorctl.h"

static BOOL ensure_text_capacity (EDITORCTL_EXTRA *extra, int required_capacity)
{
    int current_capacity;

    if (extra->text != NULL)
    {
        if ((current_capacity = HeapSize (extra->heap, 0, extra->text)) == (SIZE_T)-1) goto error;
    }
    else current_capacity = 0;

    if (required_capacity > current_capacity)
    {
        int new_capacity;
        char *new_text;

        new_capacity = required_capacity * 3 / 2;

        if (extra->text != NULL)
        {
            if ((new_text = (char *)HeapReAlloc (extra->heap, 0, extra->text, new_capacity)) == NULL) goto error;
        }
        else
        {
            if ((new_text = (char *)HeapAlloc (extra->heap, 0, new_capacity)) == NULL) goto error;
        }

        extra->text = new_text;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL ensure_row_capacity (HANDLE heap, int **row_offsets, int required_capacity)
{
    int current_capacity;

    if (*row_offsets != NULL)
    {
        if ((current_capacity = HeapSize (heap, 0, *row_offsets)) == (SIZE_T)-1) goto error;
    }
    else current_capacity = 0;

    if (required_capacity > current_capacity)
    {
        int new_capacity;
        int *new_row_offsets;

        new_capacity = required_capacity * 3 / 2;

        if (*row_offsets != NULL)
        {
            if ((new_row_offsets = (int *)HeapReAlloc (heap, 0, *row_offsets, new_capacity)) == NULL) goto error;
        }
        else
        {
            if ((new_row_offsets = (int *)HeapAlloc (heap, 0, new_capacity)) == NULL) goto error;
        }

        *row_offsets = new_row_offsets;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL update (HWND hwnd, EDITORCTL_EXTRA *extra, int offset, int old_length, int new_length)
{
    char *ptr, *end_ptr, *eptr, *last_wrap_ptr;
    int row, start_row, col, col_count, reuse_row, delta, i, last_wrap_column;
    int *row_offsets = NULL, *row_widths = NULL;
    BOOL do_reuse;

    if (offset > 0)
    {
        if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;
        start_row = row;
        if (extra->word_wrap_min_column > 0 && col < extra->word_wrap_min_column)
        {
            if (!editorctl_rc_to_offset (hwnd, row, extra->word_wrap_min_column, &offset, &col)) goto error;
        }

        if (!ensure_row_capacity (extra->heap, &row_offsets, (row - start_row + 1) * sizeof (int))) goto error;
        row_offsets [row - start_row] = extra->row_offsets [row];
        row++;
    }
    else
    {
        row = 0;
        col = 0;
        start_row = 0;
        if (!ensure_row_capacity (extra->heap, &row_offsets, (row - start_row + 1) * sizeof (int))) goto error;
        row_offsets [row - start_row] = 0;
        row++;
    }

    ptr = extra->text + offset;
    end_ptr = extra->text + extra->text_length;
    eptr = ptr + new_length;

    reuse_row = start_row;
    delta = new_length - old_length;
    do_reuse = FALSE;
    last_wrap_ptr = NULL;

    while (ptr < end_ptr)
    {
        EDITORCTL_UNICODE_CHAR pch;
        BOOL wrap = FALSE;

        pch = *ptr++;

        if (pch == '\t')
            col = (col + extra->tab_width) / extra->tab_width * extra->tab_width;
        else col++;

        if (extra->word_wrap_min_column > 0 && col >= extra->word_wrap_min_column && (pch == ' ' || pch == '\t'))
        {
            last_wrap_ptr = ptr;
            last_wrap_column = col;
        }

        if (extra->word_wrap_column > 0 && col >= extra->word_wrap_column && pch != '\n' && pch != '\r')
        {
            if (last_wrap_ptr != NULL)
            {
                ptr = last_wrap_ptr;
                col = last_wrap_column;
            }

            wrap = TRUE;
        }

        if (pch == '\n' ||
            (pch == '\r' && (ptr == end_ptr || *ptr != '\n')) ||
            wrap)
        {
            if (!ensure_row_capacity (extra->heap, &row_widths, (row - start_row) * sizeof (int))) goto error;
            row_widths [row - start_row - 1] = col + (wrap ? 1 : 0);

            if (ptr >= eptr)
            {
                int o;

                o = ptr - extra->text - delta;

                while (reuse_row < extra->row_count && extra->row_offsets [reuse_row] < o) reuse_row++;
                if (reuse_row < extra->row_count && extra->row_offsets [reuse_row] == o)
                {
                    do_reuse = TRUE;
                    break;
                }
            }

            if (!ensure_row_capacity (extra->heap, &row_offsets, (row - start_row + 1) * sizeof (int))) goto error;
            row_offsets [row - start_row] = ptr - extra->text;
            col = 0;
            row++;
            last_wrap_ptr = NULL;
        }
    }

    if (!do_reuse)
    {
        reuse_row = extra->row_count;
        if (!ensure_row_capacity (extra->heap, &row_widths, (row - start_row) * sizeof (int))) goto error;
        row_widths [row - start_row - 1] = col + 1;
    }

    if (!ensure_row_capacity (extra->heap, &extra->row_offsets, (row + extra->row_count - reuse_row) * sizeof (int))) goto error;
    if (!ensure_row_capacity (extra->heap, &extra->row_widths, (row + extra->row_count - reuse_row) * sizeof (int))) goto error;

    if (delta != 0)
    {
        for (i = reuse_row; i < extra->row_count; i++)
            extra->row_offsets [i] += delta;
    }

    CopyMemory (
        extra->row_offsets + row,
        extra->row_offsets + reuse_row,
        (extra->row_count - reuse_row) * sizeof (int));

    CopyMemory (
        extra->row_widths + row,
        extra->row_widths + reuse_row,
        (extra->row_count - reuse_row) * sizeof (int));

    CopyMemory (
        extra->row_offsets + start_row,
        row_offsets,
        (row - start_row) * sizeof (int));

    CopyMemory (
        extra->row_widths + start_row,
        row_widths,
        (row - start_row) * sizeof (int));

    extra->row_count = row + extra->row_count - reuse_row;
    col_count = 0;
    for (i = 0; i < extra->row_count; i++)
        col_count = max (col_count, extra->row_widths [i]);
    extra->column_count = col_count;

    if (!editorctl_update_scroll_range (hwnd)) goto error;

    if (row != reuse_row)
    {
        if (!editorctl_invalidate_rows (hwnd, start_row, extra->row_count)) goto error;
    }
    else
    {
        if (!editorctl_invalidate_rows (hwnd, start_row, row)) goto error;
    }

    HeapFree (extra->heap, 0, row_offsets);

    return TRUE;
error:
    if (row_offsets != NULL)
        HeapFree (extra->heap, 0, row_offsets);

    return FALSE;
}

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length, int new_caret_offset)
{
    EDITORCTL_EXTRA *extra = NULL;
    RECT r;
    int w, h, width, height, sx, sy;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (buffer_length > length)
    {
        if (!ensure_text_capacity (extra, extra->text_length + buffer_length - length)) goto error;
    }

    if (buffer_length != length)
    {
        CopyMemory (
            extra->text + offset + buffer_length,
            extra->text + offset + length,
            extra->text_length - offset - length);
    }

    CopyMemory (
        extra->text + offset,
        buffer,
        buffer_length);

    extra->text_length = extra->text_length + buffer_length - length;

    if (!update (hwnd, extra, offset, length, buffer_length)) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;

    w = r.right - r.left;
    width = extra->column_count * extra->cell_size.cx;
    sx = extra->scroll_location.x;
    if (sx + w > width)
        sx = max (0, width - w);

    h = r.bottom - r.top;
    height = extra->row_count * extra->cell_size.cy;
    sy = extra->scroll_location.y;
    if (sy + h > height)
        sy = max (0, height - h);

    if (!editorctl_scroll_to (hwnd, sx, sy)) goto error;

    if (!editorctl_move_cursor (hwnd, new_caret_offset, FALSE)) goto error;

    return TRUE;
error:
    return FALSE;
}
