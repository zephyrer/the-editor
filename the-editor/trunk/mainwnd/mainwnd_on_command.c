#include <windows.h>

#include "../resources.h"
#include "mainwnd.h"

BOOL CALLBACK DeleteItemProc(HWND hwndDlg, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam) 
{ 
    switch (message) 
    { 
        case WM_CLOSE: 
            EndDialog (hwndDlg, 0);
            return TRUE;
    } 
    return FALSE; 
}

LRESULT mainwnd_on_command (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND wnd;

    DialogBox ((HINSTANCE)GetWindowLong (hwnd, GWL_HINSTANCE), (LPCTSTR)ABOUT_THE_EDITOR_DIALOG_ID, hwnd, DeleteItemProc);
}
