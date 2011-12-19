#include "mainwnd.h"

LRESULT mainwnd_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MAINWND_EXTRA *extra;

    if ((extra = (MAINWND_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;
    DestroyWindow (extra->editor);
    DestroyWindow (extra->status_bar);
    HeapFree (extra->heap, 0, extra);

    return 0;
error:
    return -1;
}
