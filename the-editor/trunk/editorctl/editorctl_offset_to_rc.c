#include "editorctl.h"

BOOL editorctl_offset_to_rc (HWND hwnd, int offset, int *row, int *col)
{
    EDITORCTL_EXTRA *extra = NULL;
    int line;
    int c;
    char *ptr, *p;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!editorctl_offset_to_line (hwnd, offset, &line)) goto error;

    ptr = extra->text + extra->line_offsets [line];
    p = extra->text + offset;

    c = 0;
    while (ptr < p)
    {
        EDITORCTL_UNICODE_CHAR ch;

        ch = editorctl_get_next_char (&ptr);
        if (ch == '\t')
            c = (c + extra->tab_width) / extra->tab_width * extra->tab_width;
        else c++;
    }

    *row = line;
    *col = c;

    return TRUE;
error:
    return FALSE;
}
