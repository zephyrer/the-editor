#include "editorctl.h"

LRESULT editorctl_on_setfocus (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!editorctl_update_caret (editorctl, FALSE)) goto error;

    return 0;
error:
    return -1;
}
