#include <assert.h>
#include <windows.h>

#include "../resources.h"
#include "mainwnd.h"

extern LPTSTR MAINWND_CLASS_NAME = TEXT ("TheEditorMainWindow");

ATOM mainwnd_register_class (HINSTANCE hInstance)
{
    WNDCLASSEX wc;
    HICON icon;

    assert (hInstance != NULL);

    icon = LoadIcon (hInstance, (LPCWSTR)THE_EDITOR_ICON_ID);

    wc.cbSize        = sizeof (WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = mainwnd_wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof (LONG_PTR);
    wc.hInstance     = hInstance;
    wc.hIcon         = icon;
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = (LPCWSTR)MAIN_MENU_ID;
    wc.lpszClassName = MAINWND_CLASS_NAME;
    wc.hIconSm       = NULL;

    return RegisterClassEx(&wc);
}
