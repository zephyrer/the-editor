#include "mainwnd.h"

LRESULT CALLBACK mainwnd_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            return mainwnd_on_create (hwnd, msg, wParam, lParam);
        case WM_DESTROY:
            return mainwnd_on_destroy (hwnd, msg, wParam, lParam);
        case WM_CLOSE:
            return mainwnd_on_close (hwnd, msg, wParam, lParam);
        case WM_SIZE:
            return mainwnd_on_size (hwnd, msg, wParam, lParam);
        case WM_SETFOCUS:
            return mainwnd_on_setfocus (hwnd, msg, wParam, lParam);
        case WM_COMMAND:
            return mainwnd_on_command (hwnd, msg, wParam, lParam);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
