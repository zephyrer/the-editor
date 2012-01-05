#include "editorctl.h"

LRESULT editorctl_on_setfont (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (editorctl->font != (HFONT)wParam)
    {
        editorctl->font = (HFONT)wParam;

        if (lParam)
        {
            if (!InvalidateRect (editorctl->hwnd, NULL, TRUE)) goto error;
        }
    }

    return 0;
error:
    return 1;
}
