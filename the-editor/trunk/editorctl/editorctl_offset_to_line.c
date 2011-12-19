#include "editorctl.h"

BOOL editorctl_offset_to_line (HWND hwnd, int offset, int *line)
{
    EDITORCTL_EXTRA *extra = NULL;
    int a, b;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    a = 0;
    b = extra->line_count - 1;

    if (extra->line_offsets [a] == offset) return a;
    if (extra->line_offsets [b] == offset) return b;
    while (b - a > 1)
    {
        int c, c_offset;
        
        c = (a + b) / 2;
        c_offset = extra->line_offsets [c];

        if (c_offset == offset)
        {
            a = c;
            break;
        }
        else if (c_offset < offset) a = c;
        else b = c;
    }

    *line = a;

    return TRUE;
error:
    return FALSE;
}
