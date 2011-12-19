#include "editorctl.h"

LRESULT editorctl_on_setfont (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra = NULL;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (extra->font != (HFONT)wParam)
    {
        extra->font = (HFONT)wParam;

        if (lParam)
        {
            if (!InvalidateRect (hwnd, NULL, TRUE)) goto error;
        }
    }

    return 0;
error:
    return 1;
}
