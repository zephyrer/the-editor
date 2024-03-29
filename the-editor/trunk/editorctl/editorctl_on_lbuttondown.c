#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_lbuttondown (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ULONGLONG t;
    BOOL selecting;
    int x, y;

    x = GET_X_LPARAM (lParam);
    y = GET_Y_LPARAM (lParam);

    t = GetTickCount64 ();
    if (t - editorctl->last_click_time <= GetDoubleClickTime () && abs (x - editorctl->last_click_x) < 3 && abs (y - editorctl->last_click_y) < 3)
    {
        switch (editorctl->mouse_select_mode)
        {
        case EDITORCTL_MSM_CHAR:
            editorctl->mouse_select_mode = EDITORCTL_MSM_WORD;
            break;
        case EDITORCTL_MSM_WORD:
            editorctl->mouse_select_mode = EDITORCTL_MSM_ROW;
            break;
        case EDITORCTL_MSM_ROW:
            editorctl->mouse_select_mode = EDITORCTL_MSM_CHAR;
            break;
        }
        selecting = TRUE;
    }
    else if (wParam & MK_SHIFT)
    {
        editorctl->mouse_anchor_offset = editorctl->editor.anchor_offset;
        editorctl->mouse_select_mode = (wParam & MK_CONTROL) ? EDITORCTL_MSM_WORD : EDITORCTL_MSM_CHAR;
        selecting = TRUE;
    }
    else
    {
        editorctl->mouse_select_mode = (wParam & MK_CONTROL) ? EDITORCTL_MSM_WORD : EDITORCTL_MSM_CHAR;
        selecting = FALSE;
    }

    if (!editorctl_move_mouse_cursor (editorctl, x, y, selecting)) goto error;

    SetCapture (hwnd);

    return 0;
error:
    return -1;
}
