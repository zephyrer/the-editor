#include "editorctl.h"

BOOL editorctl_rc_to_offset (HWND hwnd, int row, int column, int *offset, int *real_col)
{
    EDITORCTL_EXTRA *extra;
    char *ptr, *end_ptr, *p;
    int c, rc;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    ptr = extra->text + extra->row_offsets [row];
    end_ptr = row < extra->row_count - 1 ? extra->text + extra->row_offsets [row + 1] : extra->text + extra->text_length;
    c = 0;
    p = ptr;
    rc = 0;
    while (c <= column)
    {
        char ch;

        p = ptr;
        rc = c;

        if (ptr >= end_ptr) break;

        ch = *ptr++;
        if (ch == '\r' || ch == '\n') break;

        if (ch == '\t')
            c = (c + extra->tab_width) / extra->tab_width * extra->tab_width;
        else c = c + 1;
    }
    *offset = p - extra->text;
    *real_col = rc;

    return TRUE;
error:
    return FALSE;
}

/*
BOOL editorctl_rc_to_offset (HWND hwnd, int row, int column, int *offset, int *real_col)
{
    EDITORCTL_EXTRA *extra;
    EDITORCTL_TEXT_ITERATOR it;
    int end_offset;
    int c, o, rc;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    if (!editorctl_set_iterator (hwnd, extra->row_offsets [row], &it)) goto error;
    end_offset = row < extra->row_count - 1 ? extra->row_offsets [row + 1] : extra->text_length;
    c = 0;
    o = it.offset;
    rc = 0;
    while (c < column && it.offset < end_offset)
    {
        o = it.offset;
        rc = c;

        editorctl_forward (&it);
        if (editorctl_get_prev_char (&it) == '\t')
            c = (c + extra->tab_width) / extra->tab_width * extra->tab_width;
        else c = c + 1;
    }
    *offset = o;
    *real_col = rc;

    return TRUE;
error:
    return FALSE;
}
*/