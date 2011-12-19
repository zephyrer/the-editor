#include "editorctl.h"

BOOL editorctl_scroll_to (HWND hwnd, int x, int y)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (extra->scroll_location.x != x || extra->scroll_location.y != y)
    {
        SCROLLINFO si;

        if (!ScrollWindow (hwnd, extra->scroll_location.x - x, extra->scroll_location.y - y, NULL, NULL)) goto error;

        if (extra->scroll_location.x != x)
        {
            si.cbSize = sizeof (SCROLLINFO);
            si.fMask = SIF_POS;
            si.nPos = x;
            SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);
            extra->scroll_location.x = x;
        }

        if (extra->scroll_location.y != y)
        {
            si.cbSize = sizeof (SCROLLINFO);
            si.fMask = SIF_POS;
            si.nPos = y;
            SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
            extra->scroll_location.y = y;
        }

        if (!editorctl_update_caret_pos (hwnd)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
