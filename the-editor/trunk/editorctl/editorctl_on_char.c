#include "editorctl.h"

LRESULT editorctl_on_char (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ((GetKeyState (VK_CONTROL) & 0x8000) == 0 && (GetKeyState (VK_MENU) & 0x8000) == 0 && (wParam >= 0x20 || wParam == '\t') && wParam != 0x7F)
    {
        char ch;

        ch = (char)wParam;

        if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
        {
            editor_replace_selection (&editorctl->editor, 1, &ch);
        }
        else
        {
            if (editorctl->overwrite && editor_can_overwrite (&editorctl->editor))
                editor_overwrite (&editorctl->editor, 1, &ch);
            else
                editor_insert (&editorctl->editor, 1, &ch);
        }

        if (!editorctl_update_scroll_range (editorctl)) goto error;
        if (!editorctl_update (editorctl)) goto error;
        if (!editorctl_update_caret (editorctl, TRUE)) goto error;
    }

    return 0;

error:
    return -1;
}
