#include "editorctl.h"

BOOL editorctl_offset_to_rc (HWND hwnd, int offset, int *row, int *col)
{
    EDITORCTL_EXTRA *extra;
    int r;
    int c;
    char *ptr, *end_ptr;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!editorctl_offset_to_row (hwnd, offset, &r)) goto error;

    ptr = extra->text + extra->row_offsets [r];
    end_ptr = extra->text + offset;

    c = 0;
    while (ptr < end_ptr)
    {
        if (*ptr++ == '\t')
            c = (c + extra->tab_width) / extra->tab_width * extra->tab_width;
        else c++;
    }

    *row = r;
    *col = c;

    return TRUE;
error:
    return FALSE;
}

/*
BOOL editorctl_offset_to_rc (HWND hwnd, int offset, int *row, int *col)
{
    EDITORCTL_EXTRA *extra;
    int r;
    int c;
    EDITORCTL_TEXT_ITERATOR it;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!editorctl_offset_to_row (hwnd, offset, &r)) goto error;

    if (!editorctl_set_iterator (hwnd, extra->row_offsets [r], &it)) goto error;

    c = 0;
    while (it.offset < offset)
    {
        editorctl_forward (&it);
        if (editorctl_get_prev_char (&it) == '\t')
            c = (c + extra->tab_width) / extra->tab_width * extra->tab_width;
        else c++;
    }

    *row = r;
    *col = c;

    return TRUE;
error:
    return FALSE;
}
*/