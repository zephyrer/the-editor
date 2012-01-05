#include "editorctl.h"

LRESULT editorctl_on_killfocus (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!DestroyCaret ()) goto error;

    return 0;
error:
    return -1;
}
