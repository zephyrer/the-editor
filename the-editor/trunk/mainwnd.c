#include <windows.h>
#include <CommCtrl.h>

#include "xalloc.h"
#include "editorctl.h"
#include "controller.h"

extern LPTSTR MAINWND_CLASS_NAME = TEXT ("TheEditorMainWindow");

struct mainwnd_extra
{
    HWND editor_handle;
    HWND status_bar_handle;

    HFONT font;

    controller_t *controller;
};

static LRESULT mainwnd_on_create (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam; 
    struct mainwnd_extra *extra = (struct mainwnd_extra *)xalloc (sizeof (struct mainwnd_extra));
    TEXTMETRIC fm;
    HDC dc;
    SIZE cw;

    SetWindowLongPtr (hwnd, 0, (LONG)extra);

    extra->editor_handle = CreateWindow (EDITORCTL_CLASS_NAME, NULL, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, cs->hInstance, NULL);
    extra->status_bar_handle = CreateWindow (STATUSCLASSNAME, TEXT ("Status"), WS_CHILD | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, 0, cs->hInstance, NULL);
    extra->font = CreateFont (16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("Consolas"));
    extra->controller = controller_create (extra->editor_handle);

    dc = GetDC (hwnd);
    SelectObject (dc, extra->font);
    GetTextMetrics (dc, &fm);

    SendMessage (extra->editor_handle, WM_SETFONT, (WPARAM)extra->font, 0);

    cw.cx = fm.tmAveCharWidth;
    cw.cy = fm.tmHeight;
    SendMessage (extra->editor_handle, ECM_SET_CELL_SIZE, 0, (LPARAM)&cw);

    return 0;
}

static LRESULT mainwnd_on_destroy (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);

    DestroyWindow (extra->editor_handle);
    DestroyWindow (extra->status_bar_handle);
    DeleteObject (extra->font);
    controller_free (extra->controller);

    xfree (extra);
    PostQuitMessage(0);
    return 0;
}

static LRESULT mainwnd_on_close (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    DestroyWindow(hwnd);
    return 0;
}

static LRESULT mainwnd_on_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);
    RECT r1, r2;

    SendMessage (extra->status_bar_handle, WM_SIZE, 0, 0);

    GetClientRect (hwnd, &r1);
    GetWindowRect (extra->status_bar_handle, &r2);

    MoveWindow (extra->editor_handle, r1.left, r1.top, r1.right - r1.left, r1.bottom - r1.top - (r2.bottom - r2.top), TRUE);

    return 0;
}

static LRESULT mainwnd_on_setfocus (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);

    SetFocus (extra->editor_handle);

    return 0;
}

static LRESULT mainwnd_on_char (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);

    editorctl_unicode_char_t ch = (editorctl_unicode_char_t)wParam;

    switch (ch)
    {
    case 0x0A: 
    case 0x0D: 
        controller_new_line (extra->controller);
        return 0;
    case 0x08: 
    case 0x1B: 
    case 0x7F:
        return 0;
    default:
        controller_char (extra->controller, ch);
        return 0;
    }
}

static LRESULT mainwnd_on_keydown (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);

    BOOL shift = (GetKeyState (VK_SHIFT) & 0x8000) != 0;
    BOOL control = (GetKeyState (VK_CONTROL) & 0x8000) != 0;
    BOOL alt = (GetKeyState (VK_MENU) & 0x8000) != 0;

    if (!shift && !control && !alt)
    {
        switch (wParam)
        {
        case VK_LEFT:
            controller_left (extra->controller, FALSE, FALSE);
            return 0;
        case VK_RIGHT:
            controller_right (extra->controller, FALSE, FALSE);
            return 0;
        case VK_UP:
            controller_up (extra->controller, FALSE);
            return 0;
        case VK_DOWN:
            controller_down (extra->controller, FALSE);
            return 0;
		case VK_HOME:
			controller_home (extra->controller, FALSE);
			return 0;
		case VK_END:
			controller_end (extra->controller, FALSE);
			return 0;
		case VK_PRIOR:
			controller_page_up (extra->controller, FALSE);
			return 0;
		case VK_NEXT:
			controller_page_down (extra->controller, FALSE);
			return 0;
        case VK_BACK:
            controller_backspace (extra->controller, FALSE);
            return 0;
        case VK_DELETE:
            controller_delete (extra->controller, FALSE);
            return 0;
        }
    }
    else if (!shift && control && !alt)
    {
        switch (wParam)
        {
        case VK_LEFT:
            controller_left (extra->controller, TRUE, FALSE);
            return 0;
        case VK_RIGHT:
            controller_right (extra->controller, TRUE, FALSE);
            return 0;
        case VK_UP:
            controller_scroll_up (extra->controller);
            return 0;
        case VK_DOWN:
            controller_scroll_down (extra->controller);
            return 0;
		case VK_HOME:
			controller_document_home (extra->controller, FALSE);
			return 0;
		case VK_END:
			controller_document_end (extra->controller, FALSE);
			return 0;
		case VK_PRIOR:
			controller_page_home (extra->controller, FALSE);
			return 0;
		case VK_NEXT:
			controller_page_end (extra->controller, FALSE);
			return 0;
        case VK_BACK:
            controller_backspace (extra->controller, TRUE);
            return 0;
        case VK_DELETE:
            controller_delete (extra->controller, TRUE);
            return 0;
        }
    }
    else if (shift && !control && !alt)
    {
        switch (wParam)
        {
        case VK_LEFT:
            controller_left (extra->controller, FALSE, TRUE);
            return 0;
        case VK_RIGHT:
            controller_right (extra->controller, FALSE, TRUE);
            return 0;
        case VK_UP:
            controller_up (extra->controller, TRUE);
            return 0;
        case VK_DOWN:
            controller_down (extra->controller, TRUE);
            return 0;
		case VK_HOME:
			controller_home (extra->controller, TRUE);
			return 0;
		case VK_END:
			controller_end (extra->controller, TRUE);
			return 0;
		case VK_PRIOR:
			controller_page_up (extra->controller, TRUE);
			return 0;
		case VK_NEXT:
			controller_page_down (extra->controller, TRUE);
			return 0;
        }
    }
    else if (shift && control && !alt)
    {
        switch (wParam)
        {
        case VK_LEFT:
            controller_left (extra->controller, TRUE, TRUE);
            return 0;
        case VK_RIGHT:
            controller_right (extra->controller, TRUE, TRUE);
            return 0;
		case VK_HOME:
			controller_document_home (extra->controller, TRUE);
			return 0;
		case VK_END:
			controller_document_end (extra->controller, TRUE);
			return 0;
		case VK_PRIOR:
			controller_page_home (extra->controller, TRUE);
			return 0;
		case VK_NEXT:
			controller_page_end (extra->controller, TRUE);
			return 0;
        }
    }

    return 0;
}

static LRESULT mainwnd_on_request_data (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct mainwnd_extra *extra = (struct mainwnd_extra *)GetWindowLongPtr (hwnd, 0);
    struct editorctl_data_request *request = (struct editorctl_data_request *)lParam;

    controller_request_data (extra->controller, request->start_row, request->start_column, request->end_row, request->end_column, request->buffer, request->styles);

    return 1;
}

static LRESULT CALLBACK mainwnd_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            return mainwnd_on_create (hwnd, wParam, lParam);
        break;
        case WM_DESTROY:
            return mainwnd_on_destroy (hwnd, wParam, lParam);
        break;
        case WM_CLOSE:
            return mainwnd_on_close (hwnd, wParam, lParam);
        break;
        case WM_SIZE:
            return mainwnd_on_size (hwnd, wParam, lParam);
        break;
        case WM_SETFOCUS:
            return mainwnd_on_setfocus (hwnd, wParam, lParam);
        break;
        case WM_CHAR:
            return mainwnd_on_char (hwnd, wParam, lParam);
        break;
        case WM_KEYDOWN:
            return mainwnd_on_keydown (hwnd, wParam, lParam);
        break;
        case ECM_REQUEST_DATA:
            return mainwnd_on_request_data (hwnd, wParam, lParam);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

extern ATOM mainwnd_register_class (HINSTANCE hInstance)
{
    WNDCLASSEX wc;

    wc.cbSize        = sizeof (WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = mainwnd_wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof (LONG_PTR);
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = MAINWND_CLASS_NAME;
    wc.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

    return RegisterClassEx(&wc);
}
