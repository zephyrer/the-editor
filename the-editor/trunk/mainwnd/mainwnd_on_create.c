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
    if ((extra->editor = CreateWindow (EDITORCTL_CLASS_NAME, NULL, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL)) == NULL) goto error;
    if ((extra->status_bar = CreateWindow (STATUSCLASSNAME, TEXT ("Status"), WS_CHILD | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, NULL)) == NULL) goto error;

    if ((extra->font = CreateFont (16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("Consolas"))) == 0) goto error;
    if (SendMessage (extra->editor, WM_SETFONT, (WPARAM)extra->font, 0) != S_OK) goto error;

    return 0;
error:
    if (extra != NULL)
    {
        if (extra->editor != NULL) DestroyWindow (extra->editor);
        if (extra->status_bar != NULL) DestroyWindow (extra->status_bar);
        if (extra->font != NULL) DeleteObject (extra->font);
        HeapFree (heap, 0, extra);
    }

    return -1;
}
