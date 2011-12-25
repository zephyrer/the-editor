#include <WindowsX.h>

#include "editorctl.h"

LRESULT editorctl_on_lbuttondown (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    ULONGLONG t;
    BOOL selecting;
    int x, y;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    x = GET_X_LPARAM (lParam);
    y = GET_Y_LPARAM (lParam);

    t = GetTickCount64 ();
    if (t - extra->last_click_time <= GetDoubleClickTime () && abs (x - extra->last_click_x) < 3 && abs (y - extra->last_click_y) < 3)
    {
        switch (extra->mouse_select_mode)
        {
        case EDITORCTL_MSM_CHAR:
            extra->mouse_select_mode = EDITORCTL_MSM_WORD;
            break;
        case EDITORCTL_MSM_WORD:
            extra->mouse_select_mode = EDITORCTL_MSM_ROW;
            break;
        case EDITORCTL_MSM_ROW:
            extra->mouse_select_mode = EDITORCTL_MSM_CHAR;
            break;
        }
        selecting = TRUE;
    }
    else
    {
        extra->mouse_select_mode = (wParam & MK_CONTROL) ? EDITORCTL_MSM_WORD : EDITORCTL_MSM_CHAR;
        selecting = FALSE;
    }

    if (!editorctl_move_mouse_cursor (hwnd, x, y, selecting)) goto error;

    SetCapture (hwnd);

    return 0;
error:
    return -1;
}
