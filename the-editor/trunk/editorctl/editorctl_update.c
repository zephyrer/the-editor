#include "editorctl.h"

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
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);

    return FALSE;
}

static const char *advance_to_next_row (const char *ptr, const char *end_ptr, int *col, int tab_width)
{
    int c;
    
    c = *col;
    while (TRUE)
    {
        char ch;

        if (ptr >= end_ptr)
        {
            ptr = NULL;
            break;
        }

        ch = *ptr++;
        if (ch == '\t')
            c += tab_width - c % tab_width;
        else
        {
            c++;
            if (ch == '\r')
            {
                if (ptr >= end_ptr || *ptr != '\n') break;
            }
            else if (ch == '\n') break;
        }
    }
    *col = c;

    return ptr;
}

static const char *advance_to_next_row_wrap (const char *ptr, const char *end_ptr, int *col, int tab_width, int wrap_col, int min_wrap_col, BOOL *wrap)
{
    int c;
    int last_wrap_col;
    const char *last_wrap_ptr;
    BOOL w = FALSE;
    
    c = *col;
    last_wrap_col = -1;
    last_wrap_ptr = NULL;
    while (TRUE)
    {
        char ch;
        int last_col;
        const char *last_ptr;

        if (ptr >= end_ptr)
        {
            ptr = NULL;
            break;
        }

        last_col = c;
        last_ptr = ptr;

        ch = *ptr++;

        if (ch == '\t')
            c += tab_width - c % tab_width;
        else
        {
            c++;
            if (ch == '\r')
            {
                if (ptr >= end_ptr || *ptr != '\n') break;
            }
            else if (ch == '\n') break;
        }

        if (c >= wrap_col)
        {
            if (last_wrap_col < 0)
            {
                last_wrap_col = last_col;
                last_wrap_ptr = last_ptr;
            }

            w = TRUE;
            break;
        }

        if (c >= min_wrap_col && (ch == ' ' || ch == '\t'))
        {
            last_wrap_col = c;
            last_wrap_ptr = ptr;
        }
    }

    if (w)
    {
        *col = last_wrap_col;
        *wrap = TRUE;
        return last_wrap_ptr;
    }
    else
    {
        *col = c;
        *wrap = FALSE;
        return ptr;
    }
}

BOOL editorctl_update (HWND hwnd, int offset, int old_length, int new_length)
{
    EDITORCTL_EXTRA *extra;
    const char *ptr, *end_ptr, *eptr;
    int row, start_row, col, col_count, reuse_row, delta, i;
    int *row_offsets = NULL, *row_widths = NULL;
    BOOL do_reuse;
    int old_width, new_width, old_row_count;
    int start_offset;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;

    start_offset = offset;
    if (extra->word_wrap_min_column > 0)
    {
        if (col < extra->word_wrap_column - extra->word_wrap_min_column && row > 0)
        {
            row = row - 1;
            if (!editorctl_rc_to_offset (hwnd, row, extra->word_wrap_min_column, &start_offset, &col)) goto error;
        }
        else if (col > extra->word_wrap_min_column)
        {
            if (!editorctl_rc_to_offset (hwnd, row, extra->word_wrap_min_column, &start_offset, &col)) goto error;
        }
    }

    start_row = row;

    if (!ensure_row_capacity (extra->heap, &row_offsets, (row - start_row + 1) * sizeof (int))) goto error;
    row_offsets [row - start_row] = extra->row_offsets [row];
    row++;

    ptr = extra->text + start_offset;
    end_ptr = extra->text + extra->text_length;
    eptr = extra->text + offset + new_length;

    reuse_row = start_row;
    delta = new_length - old_length;
    do_reuse = FALSE;

    while (TRUE)
    {
        BOOL wrap;

        if (extra->word_wrap_column > 0)
        {
            ptr = advance_to_next_row_wrap (ptr, end_ptr, &col, extra->tab_width, extra->word_wrap_column, extra->word_wrap_min_column, &wrap);
        }
        else
        {
            ptr = advance_to_next_row (ptr, end_ptr, &col, extra->tab_width);
            wrap = FALSE;
        }

        if (!ensure_row_capacity (extra->heap, &row_widths, (row - start_row) * sizeof (int))) goto error;

        row_widths [row - start_row - 1] = col + ((wrap || ptr == NULL) ? 1 : 0);

        if (ptr == NULL)
        {
            reuse_row = extra->row_count;
            break;
        }

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
    }

    if (!ensure_row_capacity (extra->heap, &extra->row_offsets, (row + extra->row_count - reuse_row) * sizeof (int))) goto error;
    if (!ensure_row_capacity (extra->heap, &extra->row_widths, (row + extra->row_count - reuse_row) * sizeof (int))) goto error;

    if (delta != 0)
    {
        for (i = reuse_row; i < extra->row_count; i++)
            extra->row_offsets [i] += delta;
    }

    old_width = 0;
    for (i = start_row; i < reuse_row; i++)
        old_width = max (old_width, extra->row_widths [i]);

    new_width = 0;
    for (i = start_row; i < row; i++)
        new_width = max (new_width, row_widths [i - start_row]);

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

    old_row_count = extra->row_count;
    extra->row_count = row + extra->row_count - reuse_row;

    if (new_width >= extra->column_count)
        extra->column_count = new_width;
    else if (old_width >= extra->column_count)
    {
        col_count = 0;
        for (i = 0; i < extra->row_count; i++)
            col_count = max (col_count, extra->row_widths [i]);
        extra->column_count = col_count;
    }

    if (row != reuse_row)
    {
        if (!editorctl_invalidate_rows (hwnd, start_row, max (extra->row_count, old_row_count))) goto error;
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