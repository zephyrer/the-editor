#include "editorctl.h"

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length)
{
    EDITORCTL_EXTRA *extra = NULL;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;



    return TRUE;
error:
    return FALSE;
}
