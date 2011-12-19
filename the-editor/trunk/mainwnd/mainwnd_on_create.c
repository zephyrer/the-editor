#include <windows.h>
#include <commctrl.h>

#include "mainwnd.h"
#include "../editorctl/editorctl.h"

LRESULT mainwnd_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HANDLE heap = NULL;
    MAINWND_EXTRA *extra = NULL;

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((extra = (MAINWND_EXTRA *)HeapAlloc (heap, HEAP_ZERO_MEMORY, sizeof (MAINWND_EXTRA))) == NULL)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }

    extra->heap = heap;

    SetLastError (ERROR_SUCCESS);
    if (SetWindowLongPtr (hwnd, 0, (LONG)extra) == 0 && GetLastError () != ERROR_SUCCESS) goto error;

    extra->editor = CreateWindow (EDITORCTL_CLASS_NAME, NULL, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
    extra->status_bar = CreateWindow (STATUSCLASSNAME, TEXT ("Status"), WS_CHILD | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

    return 0;
error:
    if (extra != NULL)
    {
        if (extra->editor != NULL) DestroyWindow (extra->editor);
        if (extra->status_bar != NULL) DestroyWindow (extra->status_bar);
        HeapFree (heap, 0, extra);
    }

    return -1;
}
