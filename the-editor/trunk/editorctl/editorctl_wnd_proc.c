#include "editorctl.h"

LRESULT CALLBACK editorctl_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            return editorctl_on_create (hwnd, msg, wParam, lParam);
        case WM_DESTROY:
            return editorctl_on_destroy (hwnd, msg, wParam, lParam);
        case WM_PAINT:
            return editorctl_on_paint (hwnd, msg, wParam, lParam);
        case WM_SETFOCUS:
            return editorctl_on_setfocus (hwnd, msg, wParam, lParam);
        case WM_KILLFOCUS:
            return editorctl_on_killfocus (hwnd, msg, wParam, lParam);
        case WM_SIZE:
            return editorctl_on_size (hwnd, msg, wParam, lParam);
        case WM_HSCROLL:
            return editorctl_on_hscroll (hwnd, msg, wParam, lParam);
        case WM_VSCROLL:
            return editorctl_on_vscroll (hwnd, msg, wParam, lParam);
        case WM_MOUSEWHEEL:
            return editorctl_on_mousewheel (hwnd, msg, wParam, lParam);
        case WM_SETFONT:
            return editorctl_on_setfont (hwnd, msg, wParam, lParam);
        case WM_CHAR:
            return editorctl_on_char (hwnd, msg, wParam, lParam);
        case WM_KEYDOWN:
            return editorctl_on_keydown (hwnd, msg, wParam, lParam);
        case WM_LBUTTONDOWN:
            return editorctl_on_lbuttondown (hwnd, msg, wParam, lParam);
        case WM_MOUSEMOVE:
            return editorctl_on_mousemove (hwnd, msg, wParam, lParam);
        case WM_LBUTTONUP:
            return editorctl_on_lbuttonup (hwnd, msg, wParam, lParam);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
