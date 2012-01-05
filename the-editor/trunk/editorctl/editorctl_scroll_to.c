#include "editorctl.h"

BOOL editorctl_scroll_to (EDITORCTL *editroctl, int x, int y)
{
    if (editroctl->scroll_location.x != x || editroctl->scroll_location.y != y)
    {
        SCROLLINFO si;

        if (!ScrollWindow (editroctl->hwnd, editroctl->scroll_location.x - x, editroctl->scroll_location.y - y, NULL, NULL)) goto error;

        if (editroctl->scroll_location.x != x)
        {
            si.cbSize = sizeof (SCROLLINFO);
            si.fMask = SIF_POS;
            si.nPos = x;
            SetScrollInfo (editroctl->hwnd, SB_HORZ, &si, TRUE);
            editroctl->scroll_location.x = x;
        }

        if (editroctl->scroll_location.y != y)
        {
            si.cbSize = sizeof (SCROLLINFO);
            si.fMask = SIF_POS;
            si.nPos = y;
            SetScrollInfo (editroctl->hwnd, SB_VERT, &si, TRUE);
            editroctl->scroll_location.y = y;
        }

        if (!editorctl_update_caret_pos (editroctl)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
