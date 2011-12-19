#include <windows.h>
#include <malloc.h>

#include "xalloc.h"
#include "editorctl.h"

static LPTSTR EDITORCTL_CLASS_NAME = TEXT ("TheEditorEditorControl");

struct editorctl_extra
{
    int row_count, column_count;
    int caret_row, caret_column;
    unsigned char caret_shape;
    int cell_width, cell_height; 
    int scroll_x, scroll_y;

    HFONT font;
};

static void set_style (HDC hdc, editorctl_style_t style)
{
    switch (style)
    {
    case ECS_NORMAL:
        SetBkColor (hdc, GetSysColor (COLOR_WINDOW));
        SetTextColor (hdc, GetSysColor (COLOR_WINDOWTEXT));
        break;
    case ECS_SELECTED:
		SetBkColor (hdc, GetSysColor (COLOR_HIGHLIGHT));
        SetTextColor (hdc, GetSysColor (COLOR_HIGHLIGHTTEXT));
        break;
    case ECS_WHITESPACE:
        SetBkColor (hdc, GetSysColor (COLOR_WINDOW));
        SetTextColor (hdc, GetSysColor (COLOR_GRAYTEXT));
        break;
    case ECS_SELECTED_WHITESPACE:
        SetBkColor (hdc, GetSysColor (COLOR_HIGHLIGHT));
        SetTextColor (hdc, GetSysColor (COLOR_GRAYTEXT));
        break;
    }
}

static void editorctl_update_caret (HWND hwnd)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    DestroyCaret ();

    if (GetFocus () == hwnd && extra->caret_shape != ECCS_NONE)
    {
        switch (extra->caret_shape)
        {
        case ECCS_INSERT:
            CreateCaret (hwnd, (HBITMAP) NULL, 1, extra->cell_height);
            break;
        case ECCS_OVERWRITE:
            CreateCaret (hwnd, (HBITMAP) NULL, extra->cell_width, extra->cell_height);
            break;
        }

        SetCaretPos (extra->caret_column * extra->cell_width - extra->scroll_x, extra->caret_row * extra->cell_height - extra->scroll_y);
        ShowCaret (hwnd);
    }
}

static void editorctl_scroll_to (HWND hwnd, int new_scroll_x, int new_scroll_y)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    SCROLLINFO si;

	if (new_scroll_x == extra->scroll_x && new_scroll_y == extra->scroll_y) return;

    ScrollWindow (hwnd, extra->scroll_x - new_scroll_x, extra->scroll_y - new_scroll_y, NULL, NULL);

    extra->scroll_x = new_scroll_x;
    extra->scroll_y = new_scroll_y;

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_POS;
    si.nPos = new_scroll_x;
    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_POS;
    si.nPos = new_scroll_y;
    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
}

static void editorctl_update_scroll (HWND hwnd)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    RECT r;
    int width, height;
    int content_width, content_height;

    SCROLLINFO si;

    int new_scroll_x = extra->scroll_x, new_scroll_y = extra->scroll_y;

    GetClientRect (hwnd, &r);

    width = r.right - r.left;
    height = r.bottom - r.top;
    content_width = extra->column_count * extra->cell_width;
    content_height = extra->row_count * extra->cell_height;

    if (new_scroll_x > (content_width - width))
        new_scroll_x = content_width - width;

    if (new_scroll_x < 0)
        new_scroll_x = 0;

    if (new_scroll_y > (content_height - height))
        new_scroll_y = content_height - height;

    if (new_scroll_y < 0)
        new_scroll_y = 0;

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = content_width;
    si.nPage = width;

    SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = content_height;
    si.nPage = height;

    SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

	editorctl_scroll_to (hwnd, new_scroll_x, new_scroll_y);
}

#pragma region Event Handlers

static LRESULT editorctl_on_create (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam; 
    struct editorctl_extra *extra = (struct editorctl_extra *)xalloc (sizeof (struct editorctl_extra));
    SetWindowLongPtr (hwnd, 0, (LONG)extra);

    extra->row_count = 0;
    extra->column_count = 0;
    extra->caret_row = 0;
    extra->caret_column = 0;
    extra->caret_shape = ECCS_NONE;
    extra->cell_width = 8;
    extra->cell_height = 16;
    extra->scroll_x = 0;
    extra->scroll_y = 0;
    extra->font = NULL;

    return 0;
}

static LRESULT editorctl_on_destroy (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    xfree (extra);
    return 0;
}

static LRESULT editorctl_on_paint (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    PAINTSTRUCT ps; 
    HDC hdc = BeginPaint(hwnd, &ps); 
    int start_row = (ps.rcPaint.top + extra->scroll_y) / extra->cell_height;
    int end_row = (ps.rcPaint.bottom + extra->scroll_y + extra->cell_height - 1) / extra->cell_height + 1;
    int start_column = (ps.rcPaint.left + extra->scroll_x) / extra->cell_width;
    int end_column = (ps.rcPaint.right + extra->scroll_x + extra->cell_width - 1) / extra->cell_width + 1;
    int row, column;
    RECT r;
    struct editorctl_data_request request;
    int width;
    int height;

    end_row = min (end_row, extra->row_count);
    end_column = min (end_column, extra->column_count);
    width = end_column - start_column;
    height = end_row - start_row;

    if (width > 0 && height > 0)
    {
        int count = width * height;

        request.start_row = start_row;
        request.end_row = end_row;
        request.start_column = start_column;
        request.end_column = end_column;
        request.buffer = (editorctl_unicode_char_t *)_malloca (count * sizeof (editorctl_unicode_char_t));
        request.styles = (editorctl_style_t *)_malloca (count * sizeof (editorctl_style_t));

        if (SendMessage (GetParent (hwnd), ECM_REQUEST_DATA, 0, (LPARAM)&request))
        {
            TCHAR *str = (TCHAR *)_malloca (width * 2 * sizeof (TCHAR));
            INT *w = (INT *)_malloca (width * sizeof (INT));
            int i;

            for (i = 0; i < width; i++)
                w [i] = extra->cell_width;

            SelectObject (hdc, extra->font ? extra->font : GetStockObject (SYSTEM_FONT));

            for (row = start_row; row < end_row; row++)
            {
                int from_column = start_column;
                int n = 0;
                editorctl_style_t last_style;

                r.top = row * extra->cell_height - extra->scroll_y;
                r.bottom = row * extra->cell_height - extra->scroll_y + extra->cell_height;

                for (column = start_column; column < end_column; column++)
                {
                    int offset = (row - start_row) * width + (column - start_column);
                    editorctl_style_t style = request.styles [offset];

                    if (n > 0 && style != last_style)
                    {
                        r.left = from_column * extra->cell_width - extra->scroll_x;
                        r.right = column * extra->cell_width - extra->scroll_x;

                        if (last_style != ECS_NONE)
                        {
                            set_style (hdc, last_style);
                            ExtTextOut (hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, str, n, w);
                        }
                        else FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1));

                        n = 0;
                        from_column = column;
                    }

                    str [n++] = (TCHAR)(request.buffer [offset]);
                    last_style = style;
                }

                if (n > 0)
                {
                    r.left = from_column * extra->cell_width - extra->scroll_x;
                    r.right = end_column * extra->cell_width - extra->scroll_x;

                    if (last_style != ECS_NONE)
                    {
                        set_style (hdc, last_style);
                        ExtTextOut (hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, str, n, w);
                    }
                    else FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1));
                }
            }

            _freea (str);
            _freea (w);
        }
        else
        {
            r.top = start_row * extra->cell_height - extra->scroll_y;
            r.bottom = end_row * extra->cell_height - extra->scroll_y;
            r.left = start_column * extra->cell_width - extra->scroll_x;
            r.right = end_column * extra->cell_width - extra->scroll_x;

            FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1));
        }

        _freea (request.buffer);
        _freea (request.styles);
    }

    r.top = ps.rcPaint.top;
    r.bottom = end_row * extra->cell_height - extra->scroll_y;
    r.left = end_column * extra->cell_width - extra->scroll_x;
    r.right = ps.rcPaint.right;

    FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1));

    r.top = end_row * extra->cell_height - extra->scroll_y;
    r.bottom = ps.rcPaint.bottom;
    r.left = ps.rcPaint.left;
    r.right = ps.rcPaint.right;

    FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1));

    EndPaint(hwnd, &ps); 
    return 0;
}

static LRESULT editorctl_on_setfocus (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    editorctl_update_caret (hwnd);

	InvalidateRect (hwnd, NULL, FALSE);

    return 0;
}

static LRESULT editorctl_on_killfocus (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    editorctl_update_caret (hwnd);

	InvalidateRect (hwnd, NULL, FALSE);

	return 0;
}

static LRESULT editorctl_on_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    editorctl_update_scroll (hwnd);

    return 0;
}

static LRESULT editorctl_on_hscroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    RECT r;
    int new_scroll_x = extra->scroll_x;
    int width;
    int content_width;
    SCROLLINFO si;

    GetClientRect (hwnd, &r);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_TRACKPOS;

    GetScrollInfo (hwnd, SB_HORZ, &si);

    width = r.right - r.left;
    content_width = extra->column_count * extra->cell_width;

    switch (LOWORD (wParam))
    {
    case SB_ENDSCROLL:
        break;
    case SB_LEFT:
        new_scroll_x = 0;
        break;
    case SB_RIGHT:
        new_scroll_x = content_width - width;
        break;
    case SB_LINELEFT:
        new_scroll_x -= extra->cell_width * 8;
        break;
    case SB_LINERIGHT:
        new_scroll_x += extra->cell_width * 8;
        break;
    case SB_PAGELEFT:
        new_scroll_x -= width;
        break;
    case SB_PAGERIGHT:
        new_scroll_x += width;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        new_scroll_x = si.nTrackPos;
        break;
    }

    if (new_scroll_x > (content_width - width))
        new_scroll_x = content_width - width;

    if (new_scroll_x < 0)
        new_scroll_x = 0;

	editorctl_scroll_to (hwnd, new_scroll_x, extra->scroll_y);

    return 0;
}

static LRESULT editorctl_on_vscroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    RECT r;
    int new_scroll_y = extra->scroll_y;
    int height;
    int content_height;
    SCROLLINFO si;

    GetClientRect (hwnd, &r);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_TRACKPOS;

    GetScrollInfo (hwnd, SB_VERT, &si);

    height = r.bottom - r.top;
    content_height = extra->row_count * extra->cell_height;

    switch (LOWORD (wParam))
    {
    case SB_ENDSCROLL:
        break;
    case SB_LEFT:
        new_scroll_y = 0;
        break;
    case SB_RIGHT:
        new_scroll_y = content_height - height;
        break;
    case SB_LINELEFT:
        new_scroll_y -= extra->cell_height;
        break;
    case SB_LINERIGHT:
        new_scroll_y += extra->cell_height;
        break;
    case SB_PAGELEFT:
        new_scroll_y -= height;
        break;
    case SB_PAGERIGHT:
        new_scroll_y += height;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        new_scroll_y = si.nTrackPos;
        break;
    }

    if (new_scroll_y > (content_height - height))
        new_scroll_y = content_height - height;

    if (new_scroll_y < 0)
        new_scroll_y = 0;

	editorctl_scroll_to (hwnd, extra->scroll_x, new_scroll_y);

    return 0;
}

static LRESULT editorctl_on_setfont (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    HFONT new_font = (HFONT)wParam;

    if (extra->font == new_font) return 0;

    extra->font = new_font;

    InvalidateRect (hwnd, NULL, FALSE);

    return 0;
}

static LRESULT editorctl_on_mousewheel (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    RECT r;
    int new_scroll_y = extra->scroll_y;
    int height;
    int content_height;
    int delta;

    GetClientRect (hwnd, &r);

    height = r.bottom - r.top;
    content_height = extra->row_count * extra->cell_height;

    delta = GET_WHEEL_DELTA_WPARAM (wParam);
    new_scroll_y -= delta * extra->cell_height * 3 / WHEEL_DELTA;

    if (new_scroll_y > (content_height - height))
        new_scroll_y = content_height - height;

    if (new_scroll_y < 0)
        new_scroll_y = 0;

	editorctl_scroll_to (hwnd, extra->scroll_x, new_scroll_y);

    return 0;
}

static LRESULT editorctl_on_char (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    return SendMessage (GetParent (hwnd), WM_CHAR, wParam, lParam);
}

static LRESULT editorctl_on_keydown (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    return SendMessage (GetParent (hwnd), WM_KEYDOWN, wParam, lParam);
}

static LRESULT editorctl_on_set_model_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    SIZE *size = (SIZE *)lParam;

    int start_column, end_column, start_row, end_row;
    RECT r, r1;

    if (extra->column_count == size->cx && extra->row_count == size->cy) return 0;

    start_column = min (extra->column_count, size->cx);
    end_column = max (extra->column_count, size->cx);
    start_row = min (extra->row_count, size->cy);
    end_row = max (extra->row_count, size->cy);

    extra->column_count = size->cx;
    extra->row_count = size->cy;

    editorctl_update_scroll (hwnd);

    GetClientRect (hwnd, &r);

    r1 = r;
    r1.left = start_column * extra->cell_width - extra->scroll_x;
    r1.right = end_column * extra->cell_width - extra->scroll_x;
    InvalidateRect (hwnd, &r1, FALSE);

    r1 = r;
    r1.top = start_row * extra->cell_height - extra->scroll_y;
    r1.bottom = end_row * extra->cell_height - extra->scroll_y;
    InvalidateRect (hwnd, &r1, FALSE);

    return 0;
}

static LRESULT editorctl_on_get_model_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    SIZE *result = (SIZE *)lParam;

    result->cx = extra->column_count;
    result->cy = extra->row_count;

    return 0;
}

static LRESULT editorctl_on_set_caret_position (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    POINT *position = (POINT *)lParam;

    if (extra->caret_column == position->x && extra->caret_row == position->y) return 0;

    extra->caret_column = position->x;
    extra->caret_row = position->y;

    editorctl_update_caret (hwnd);

    return 0;
}

static LRESULT editorctl_on_get_caret_position (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    POINT *result = (POINT *)lParam;

    result->x = extra->caret_column;
    result->y = extra->caret_row;

    return 0;
}

static LRESULT editorctl_on_set_caret_shape (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);

    unsigned char new_caret_shape = (unsigned char)wParam;

    if (extra->caret_shape == new_caret_shape) return 0;

    extra->caret_shape = new_caret_shape;

    editorctl_update_caret (hwnd);

    return 0;
}

static LRESULT editorctl_on_set_cell_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    SIZE *size = (SIZE *)lParam;

    if (extra->cell_width == size->cx && extra->cell_height == size->cy) return 0;

    extra->cell_width = size->cx;
    extra->cell_height = size->cy;

    editorctl_update_caret (hwnd);
    editorctl_update_scroll (hwnd);

    InvalidateRect (hwnd, NULL, FALSE);

    return 0;
}

static LRESULT editorctl_on_get_cell_size (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    SIZE *result = (SIZE *)lParam;

    result->cx = extra->cell_width;
    result->cy = extra->cell_height;

    return 0;
}

static LRESULT editorctl_on_get_scroll_position (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    POINT *result = (POINT *)lParam;

	result->x = extra->scroll_x;
	result->y = extra->scroll_y;

    return 0;
}

static LRESULT editorctl_on_set_scroll_position (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    struct editorctl_extra *extra = (struct editorctl_extra *)GetWindowLongPtr (hwnd, 0);
    POINT *point = (POINT *)lParam;

	editorctl_scroll_to (hwnd, point->x, point->y);

    return 0;
}

#pragma endregion

static LRESULT CALLBACK editorctl_wnd_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            return editorctl_on_create (hwnd, wParam, lParam);
        case WM_DESTROY:
            return editorctl_on_destroy (hwnd, wParam, lParam);
        case WM_PAINT:
            return editorctl_on_paint (hwnd, wParam, lParam);
        case WM_SETFOCUS:
            return editorctl_on_setfocus (hwnd, wParam, lParam);
        case WM_KILLFOCUS:
            return editorctl_on_killfocus (hwnd, wParam, lParam);
        case WM_SIZE:
            return editorctl_on_size (hwnd, wParam, lParam);
        case WM_HSCROLL:
            return editorctl_on_hscroll (hwnd, wParam, lParam);
        case WM_VSCROLL:
            return editorctl_on_vscroll (hwnd, wParam, lParam);
        case WM_SETFONT:
            return editorctl_on_setfont (hwnd, wParam, lParam);
        case WM_MOUSEWHEEL:
            return editorctl_on_mousewheel (hwnd, wParam, lParam);
        case WM_CHAR:
            return editorctl_on_char (hwnd, wParam, lParam);
        case WM_KEYDOWN:
            return editorctl_on_keydown (hwnd, wParam, lParam);
        case ECM_SET_MODEL_SIZE:
            return editorctl_on_set_model_size (hwnd, wParam, lParam);
        case ECM_GET_MODEL_SIZE:
            return editorctl_on_get_model_size (hwnd, wParam, lParam);
        case ECM_SET_CARET_POSITION:
            return editorctl_on_set_caret_position (hwnd, wParam, lParam);
        case ECM_GET_CARET_POSITION:
            return editorctl_on_get_caret_position (hwnd, wParam, lParam);
        case ECM_SET_CARET_SHAPE:
            return editorctl_on_set_caret_shape (hwnd, wParam, lParam);
        case ECM_SET_CELL_SIZE:
            return editorctl_on_set_cell_size (hwnd, wParam, lParam);
        case ECM_GET_CELL_SIZE:
            return editorctl_on_get_cell_size (hwnd, wParam, lParam);
        case ECM_GET_SCROLL_POSITION:
            return editorctl_on_get_scroll_position (hwnd, wParam, lParam);
        case ECM_SET_SCROLL_POSITION:
            return editorctl_on_set_scroll_position (hwnd, wParam, lParam);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static ATOM editorctl_register_class (HINSTANCE hInstance)
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
