#include "editorctl.h"

BOOL editorctl_offset_to_row (HWND hwnd, int offset, int *row)
{
    EDITORCTL_EXTRA *extra;
    int a, b;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    a = 0;
    b = extra->row_count - 1;

    if (extra->row_offsets [a] == offset)
    {
        *row = a;
    }
    else if (extra->row_offsets [b] <= offset)
    {
        *row = b;
    }
    else
    {
        while (b - a > 1)
        {
            int c, c_offset;
        
            c = (a + b) / 2;
            c_offset = extra->row_offsets [c];

            if (c_offset == offset)
            {
                a = c;
                break;
            }
            else if (c_offset < offset) a = c;
            else b = c;
        }

        *row = a;
    }

    return TRUE;
error:
    return FALSE;
}
