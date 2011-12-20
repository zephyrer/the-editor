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
    EDITORCTL_TEXT_ITERATOR it;
    int row, start_row, col, col_count, reuse_row, delta, i;
    int *row_offsets = NULL, *row_widths = NULL;
    BOOL do_reuse;

    if (offset > 0)
    {
        if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;
        start_row = row;
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
    if (!editorctl_set_iterator (hwnd, offset, &it)) goto error;

    reuse_row = start_row;
    delta = new_length - old_length;
    do_reuse = FALSE;

    while (editorctl_forward (&it))
    {
        EDITORCTL_UNICODE_CHAR pch;

        pch = editorctl_get_prev_char (&it);

        if (pch == '\t')
            col = (col + extra->tab_width) / extra->tab_width * extra->tab_width;
        else col++;

        if (pch == '\n' ||
            (pch == '\r' && editorctl_get_next_char (&it) != '\n'))
        {
            if (!ensure_row_capacity (extra->heap, &row_widths, (row - start_row) * sizeof (int))) goto error;
            row_widths [row - start_row - 1] = col + 1;

            if (it.offset >= offset + new_length)
            {
                while (reuse_row < extra->row_count && extra->row_offsets [reuse_row] < it.offset - delta) reuse_row++;
                if (reuse_row < extra->row_count && extra->row_offsets [reuse_row] == it.offset - delta)
                {
                    do_reuse = TRUE;
                    break;
                }
            }

            if (!ensure_row_capacity (extra->heap, &row_offsets, (row - start_row + 1) * sizeof (int))) goto error;
            row_offsets [row - start_row] = it.offset;
            col = 0;
            row++;
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
        extra->row_offsets + start_row,
        row_offsets,
        (row - start_row) * sizeof (int));

    CopyMemory (
        extra->row_widths + row,
        extra->row_widths + reuse_row,
        (extra->row_count - reuse_row) * sizeof (int));

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
    if (!editorctl_move_cursor (hwnd, new_caret_offset)) goto error;

    if (!InvalidateRect (hwnd, NULL, TRUE)) goto error;

    return TRUE;
error:
    return FALSE;
}
