#ifndef MAINWND_H
#define MAINWND_H

#include <windows.h>

typedef struct tagMAINWND_EXTRA
{
    HANDLE heap;

    HWND editor;
    HWND status_bar;
} MAINWND_EXTRA;

extern LPTSTR MAINWND_CLASS_NAME;

ATOM mainwnd_register_class (HINSTANCE hInstance);

LRESULT CALLBACK mainwnd_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT mainwnd_on_create (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT mainwnd_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT mainwnd_on_close (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT mainwnd_on_size (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT mainwnd_on_setfocus (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
