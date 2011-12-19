#include <windows.h>
#include <tchar.h>
#include <Strsafe.h>
#include <CommCtrl.h>
#include <crtdbg.h>
#include <malloc.h>

#include "mainwnd/mainwnd.h"
#include "editorctl/editorctl.h"

static HWND create_main_window (HINSTANCE hInstance)
{
    return CreateWindowEx (WS_EX_CLIENTEDGE, MAINWND_CLASS_NAME, TEXT ("The Editor"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
}

static WPARAM run_main_loop ()
{
    MSG Msg;

    while (GetMessage (&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
    }

    return Msg.wParam;
}

static void error (const WCHAR *message)
{
    TCHAR *message_buffer = NULL;
	TCHAR code_buffer [12];
    TCHAR *display_buffer = NULL;
    DWORD error_code = GetLastError(); 
	size_t buffer_size;

    if (!FormatMessage (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL, 
        error_code, 
        MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (TCHAR *)&message_buffer, 0, NULL)) 
		message_buffer = NULL;

	_itot_s (error_code, code_buffer, 12, 10);

	buffer_size = _tcslen (message) + _tcslen (message_buffer) + 40;
	display_buffer = (TCHAR *)_malloca (sizeof (TCHAR) * (buffer_size));

	_tcscpy_s (display_buffer, buffer_size, message);
	_tcscat_s (display_buffer, buffer_size, TEXT (":\n("));
	_tcscat_s (display_buffer, buffer_size, code_buffer);
	_tcscat_s (display_buffer, buffer_size, TEXT (") "));
	_tcscat_s (display_buffer, buffer_size, message_buffer == NULL ? TEXT ("") : message_buffer);

    MessageBox (NULL, (LPCTSTR)display_buffer, TEXT ("Error"), MB_ICONEXCLAMATION | MB_OK);

	if (message_buffer != NULL)
	    LocalFree (message_buffer);

	_freea (display_buffer);
    return;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hwnd;
    INITCOMMONCONTROLSEX icce;
    WPARAM exit_code;

    icce.dwSize = sizeof (INITCOMMONCONTROLSEX);
    icce.dwICC = ICC_BAR_CLASSES;

    if (!InitCommonControlsEx (&icce))
    {
        error (TEXT ("Cannot load common control classes"));
        return 1;
    }

    if (!mainwnd_register_class (hInstance))
    {
        error (TEXT ("Cannot register class window class for main window"));
        return 1;
    }

    if (!editorctl_register_class (hInstance))
    {
        error (TEXT ("Cannot register class window class for main window"));
        return 1;
    }

    if (!(hwnd = create_main_window (hInstance)))
    {
        error (TEXT ("Cannot create main window"));
        return 1;
    }

    ShowWindow (hwnd, nCmdShow);
    if (!UpdateWindow (hwnd))
    {
        error (TEXT ("Cannot update main window"));
        return 1;
    }

    exit_code = run_main_loop ();

    _CrtDumpMemoryLeaks ();

    return LOWORD (exit_code);
}
