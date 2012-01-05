#include "editorctl.h"

LRESULT CALLBACK editorctl_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL *editorctl;

    SetLastError (ERROR_SUCCESS);
    editorctl = (EDITORCTL *)GetWindowLongPtr (hwnd, 0);
    if (editorctl == NULL && GetLastError () != ERROR_SUCCESS) goto error;

    switch(msg)
    {
        case WM_CREATE:
            return editorctl_on_create (editorctl, hwnd, msg, wParam, lParam);
        case WM_DESTROY:
            return editorctl_on_destroy (editorctl, hwnd, msg, wParam, lParam);
        case WM_PAINT:
            return editorctl_on_paint (editorctl, hwnd, msg, wParam, lParam);
        case WM_SETFOCUS:
            return editorctl_on_setfocus (editorctl, hwnd, msg, wParam, lParam);
        case WM_KILLFOCUS:
            return editorctl_on_killfocus (editorctl, hwnd, msg, wParam, lParam);
        case WM_SIZE:
            return editorctl_on_size (editorctl, hwnd, msg, wParam, lParam);
        case WM_HSCROLL:
            return editorctl_on_hscroll (editorctl, hwnd, msg, wParam, lParam);
        case WM_VSCROLL:
            return editorctl_on_vscroll (editorctl, hwnd, msg, wParam, lParam);
        case WM_MOUSEWHEEL:
            return editorctl_on_mousewheel (editorctl, hwnd, msg, wParam, lParam);
        case WM_SETFONT:
            return editorctl_on_setfont (editorctl, hwnd, msg, wParam, lParam);
/*        case WM_CHAR:
            return editorctl_on_char (editorctl, hwnd, msg, wParam, lParam);
        case WM_KEYDOWN:
            return editorctl_on_keydown (editorctl, hwnd, msg, wParam, lParam); */
        case WM_LBUTTONDOWN:
            return editorctl_on_lbuttondown (editorctl, hwnd, msg, wParam, lParam);
        case WM_MOUSEMOVE:
            return editorctl_on_mousemove (editorctl, hwnd, msg, wParam, lParam);
        case WM_LBUTTONUP:
            return editorctl_on_lbuttonup (editorctl, hwnd, msg, wParam, lParam);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

error:
    return -1;
}
