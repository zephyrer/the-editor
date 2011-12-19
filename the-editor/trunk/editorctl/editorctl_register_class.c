#include "editorctl.h"

extern LPTSTR EDITORCTL_CLASS_NAME = TEXT ("TheEditorEditorControl");

ATOM editorctl_register_class (HINSTANCE hInstance)
{
    WNDCLASSEX wc;

    wc.cbSize        = sizeof (WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = editorctl_wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof (LONG_PTR);
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = EDITORCTL_CLASS_NAME;
    wc.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

    return RegisterClassEx(&wc);
}
