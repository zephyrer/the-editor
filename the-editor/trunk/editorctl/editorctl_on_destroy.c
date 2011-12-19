#include "editorctl.h"

LRESULT editorctl_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    HeapFree (extra->heap, 0, extra);
    if (extra->whitespace_icons)
        DeleteObject (extra->whitespace_icons);

    return 0;
error:
    return -1;
}
