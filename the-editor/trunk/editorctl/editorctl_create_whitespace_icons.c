#include "editorctl.h"

static BOOL draw_space (HDC hdc, int x, int y, int w, int h)
{
    if (SetPixel (hdc, x + w / 2, y + h / 2, GetSysColor (COLOR_GRAYTEXT)) == -1) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_tab (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x + 1, y + h / 2 - e, NULL)) goto error;
    if (!LineTo (hdc, x + 1, y + h / 2 + e + 1)) goto error;
    if (!MoveToEx (hdc, x + 1, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h / 2)) goto error;
    if (!LineTo (hdc, x + w - 2 - e - 1, y + h / 2 - e - 1)) goto error;
    if (!MoveToEx (hdc, x + w - 2, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2 - e - 1, y + h / 2 + e + 1)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_tab_start (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x + 1, y + h / 2 - e, NULL)) goto error;
    if (!LineTo (hdc, x + 1, y + h / 2 + e + 1)) goto error;
    if (!MoveToEx (hdc, x + 1, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w, y + h / 2)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_tab_middle (HDC hdc, int x, int y, int w, int h)
{
    if (!MoveToEx (hdc, x, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w, y + h / 2)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_tab_end (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h / 2)) goto error;
    if (!LineTo (hdc, x + w - 2 - e - 1, y + h / 2 - e - 1)) goto error;
    if (!MoveToEx (hdc, x + w - 2, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2 - e - 1, y + h / 2 + e + 1)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_cr (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x + 1 + e, y + h / 2 - e, NULL)) goto error;
    if (!LineTo (hdc, x + 1, y + h / 2)) goto error;
    if (!LineTo (hdc, x + 1 + e + 1, y + h / 2 + e + 1)) goto error;
    if (!MoveToEx (hdc, x + 1, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h / 2)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h / 2 - e - 1)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_lf (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x + w / 2 - e, y + h / 2, NULL)) goto error;
    if (!LineTo (hdc, x + w / 2, y + h / 2)) goto error;
    if (!LineTo (hdc, x + w / 2, y + h - 2)) goto error;
    if (!LineTo (hdc, x + w / 2 - e - 1, y + h - 2 - e - 1)) goto error;
    if (!MoveToEx (hdc, x + w / 2, y + h - 2, NULL)) goto error;
    if (!LineTo (hdc, x + w / 2 + e + 1, y + h - 2 - e - 1)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL draw_wrap (HDC hdc, int x, int y, int w, int h)
{
    int e;

    e = min (w, h) * 2 / 5;

    if (!MoveToEx (hdc, x + 1, y + h / 4, NULL)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h / 4)) goto error;
    if (!LineTo (hdc, x + w - 2, y + h * 3 / 4)) goto error;
    if (!LineTo (hdc, x + 1, y + h * 3 / 4)) goto error;
    if (!LineTo (hdc, x + 1 + e + 1, y + h * 3 / 4 - e - 1)) goto error;
    if (!MoveToEx (hdc, x + 1, y + h * 3 / 4, NULL)) goto error;
    if (!LineTo (hdc, x + 1 + e + 1, y + h * 3 / 4 + e + 1)) goto error;

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_create_whitespace_icons (HWND hwnd)
{
    EDITORCTL_EXTRA *extra;
    HDC hdc = NULL, mem_dc = NULL;
    RECT r;
    HPEN pen = NULL, old_pen = NULL;
    int w, h;
    HBITMAP old_bitmap;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (extra->whitespace_icons != NULL)
    {
        if (!DeleteObject (extra->whitespace_icons)) goto error;
    }

    w = extra->cell_size.cx;
    h = extra->cell_size.cy;

    if ((hdc = GetDC (hwnd)) == NULL) goto error;
    if ((mem_dc = CreateCompatibleDC (hdc)) == NULL) goto error;
    if ((extra->whitespace_icons = CreateCompatibleBitmap (hdc, EDITORCTL_WHITESPACE_LAST * w, 2 * h)) == NULL) goto error;
    if ((old_bitmap = (HBITMAP)SelectObject (mem_dc, extra->whitespace_icons)) == NULL) goto error;

    r.left = 0;
    r.top = 0;
    r.right = EDITORCTL_WHITESPACE_LAST * w;
    r.bottom = h;
    if (!FillRect (mem_dc, &r, (HBRUSH)(COLOR_WINDOW + 1))) goto error;
    r.top = h;
    r.bottom = 2 * h;
    if (!FillRect (mem_dc, &r, (HBRUSH)(COLOR_HIGHLIGHT + 1))) goto error;
    if ((pen = CreatePen (PS_SOLID, 1, GetSysColor (COLOR_GRAYTEXT))) == NULL) goto error;
    if ((old_pen = (HPEN)SelectObject (mem_dc, pen)) == NULL) goto error;

    draw_space (mem_dc, EDITORCTL_WHITESPACE_SPACE * w, 0, w, h);
    draw_space (mem_dc, EDITORCTL_WHITESPACE_SPACE * w, h, w, h);
    draw_tab (mem_dc, EDITORCTL_WHITESPACE_TAB * w, 0, w, h);
    draw_tab (mem_dc, EDITORCTL_WHITESPACE_TAB * w, h, w, h);
    draw_tab_start (mem_dc, EDITORCTL_WHITESPACE_TAB_START * w, 0, w, h);
    draw_tab_start (mem_dc, EDITORCTL_WHITESPACE_TAB_START * w, h, w, h);
    draw_tab_middle (mem_dc, EDITORCTL_WHITESPACE_TAB_MIDDLE * w, 0, w, h);
    draw_tab_middle (mem_dc, EDITORCTL_WHITESPACE_TAB_MIDDLE * w, h, w, h);
    draw_tab_end (mem_dc, EDITORCTL_WHITESPACE_TAB_END * w, 0, w, h);
    draw_tab_end (mem_dc, EDITORCTL_WHITESPACE_TAB_END * w, h, w, h);
    draw_cr (mem_dc, EDITORCTL_WHITESPACE_CR * w, 0, w, h);
    draw_cr (mem_dc, EDITORCTL_WHITESPACE_CR * w, h, w, h);
    draw_lf (mem_dc, EDITORCTL_WHITESPACE_LF * w, 0, w, h);
    draw_lf (mem_dc, EDITORCTL_WHITESPACE_LF * w, h, w, h);
    draw_wrap (mem_dc, EDITORCTL_WHITESPACE_WRAP * w, 0, w, h);
    draw_wrap (mem_dc, EDITORCTL_WHITESPACE_WRAP * w, h, w, h);

    DeleteObject (pen);
    SelectObject (mem_dc, old_bitmap);
    DeleteDC (mem_dc);
    ReleaseDC (hwnd, hdc);

    return TRUE;
error:
    if (pen != NULL)
        DeleteObject (pen);

    if (mem_dc != NULL)
        DeleteDC (mem_dc);

    if (hdc != NULL)
        ReleaseDC (hwnd, hdc);

    return FALSE;
}
