#include <windows.h>

#include "../layout/layout.h"

#include "editorctl.h"

LRESULT editorctl_on_size (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (editorctl->editor.layout.wrap > 0)
    {
        int new_wrap;
        RECT r;

        if (!GetClientRect (editorctl->hwnd, &r)) goto error;
        new_wrap = (r.right - r.left - editorctl->cell_size.cx - 1) / editorctl->cell_size.cx;
        if (new_wrap < 20) new_wrap = 20;
        if (editorctl->editor.layout.wrap != new_wrap)
        {
            if (!layout_destroy (&editorctl->editor.layout)) goto error;
            if (!layout_initialize (&editorctl->editor.layout, editorctl->heap, &editorctl->editor.text, 8, new_wrap, new_wrap - 10)) goto error;
            if (!editorctl_update_caret (editorctl, TRUE)) goto error;
            if (!InvalidateRect (editorctl->hwnd, &r, FALSE)) goto error;
        }

    }

    if (!editorctl_update_scroll_range (editorctl)) goto error;

    return 0;
error:
    return -1;
}
