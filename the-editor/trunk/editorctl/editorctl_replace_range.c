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

static BOOL ensure_row_capacity (EDITORCTL_EXTRA *extra, int required_capacity)
{
    int current_capacity;

    if (extra->row_offsets != NULL)
    {
        if ((current_capacity = HeapSize (extra->heap, 0, extra->row_offsets)) == (SIZE_T)-1) goto error;
    }
    else current_capacity = 0;

    if (required_capacity > current_capacity)
    {
        int new_capacity;
        int *new_row_offsets;

        new_capacity = required_capacity * 3 / 2;

        if (extra->row_offsets != NULL)
        {
            if ((new_row_offsets = (int *)HeapReAlloc (extra->heap, 0, extra->row_offsets, new_capacity)) == NULL) goto error;
        }
        else
        {
            if ((new_row_offsets = (int *)HeapAlloc (extra->heap, 0, new_capacity)) == NULL) goto error;
        }

        extra->row_offsets = new_row_offsets;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL update (HWND hwnd, EDITORCTL_EXTRA *extra, int offset, int old_length, int new_length)
{
    char *ptr, *end_ptr;
    int row, row_offset;
    int state;
    int col;
    int col_count;

    ptr = extra->text;
    end_ptr = ptr + extra->text_length;
    row = 0;
    row_offset = 0;
    state = 0;
    col = 0;
    col_count = 0;
    while (ptr < end_ptr)
    {
        EDITORCTL_UNICODE_CHAR ch;
        BOOL new_row;
        int offset;

        new_row = FALSE;
        offset = ptr - extra->text;
        ch = editorctl_get_next_char (&ptr);
        switch (state)
        {
        case 0: // In the middle of the line
            if (ch == '\r') state = 1;
            else if (ch == '\n') state = 2;
            break;
        case 1: // After '\r'
            if (ch == '\r') new_row = TRUE;
            if (ch == '\n') state = 2;
            else
            {
                new_row = TRUE;
                state = 0;
            }
            break;
        case 2: // After '\n'
            if (ch == '\r')
            {
                new_row = TRUE;
                state = 1;
            }
            else if (ch == 'n') new_row = TRUE;
            else
            {
                new_row = TRUE;
                state = 0;
            }
            break;
        }

        if (new_row)
        {
            if (!ensure_row_capacity (extra, (row + 1) * sizeof (int))) goto error;
            extra->row_offsets [row] = row_offset;
            row++;
            col_count = max (col_count, col);
            col = 0;
            row_offset = offset;
        }

        if (ch == '\t')
            col = (col + extra->tab_width) / extra->tab_width * extra->tab_width;
        else col++;
    }

    ensure_row_capacity (extra, (row + 1) * sizeof (int));
    extra->row_offsets [row] = row_offset;
    row++;
    col_count = max (col_count, col);
    extra->column_count = col_count;
    extra->row_count = row;

    if (!editorctl_update_scroll_range (hwnd)) goto error;

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length)
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
    if (!InvalidateRect (hwnd, NULL, TRUE)) goto error;

    return TRUE;
error:
    return FALSE;
}
