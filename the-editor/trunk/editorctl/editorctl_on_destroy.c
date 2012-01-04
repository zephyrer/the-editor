#include <windows.h>

#include "editorctl.h"

LRESULT editorctl_on_destroy (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (editorctl != NULL)
    {
        if (editorctl->editor.heap != NULL) editor_destroy (&editorctl->editor);
        HeapFree (editorctl->heap, 0, editorctl);
    }

    SetWindowLongPtr (hwnd, 0, (LPARAM)NULL);

    return 0;
}
