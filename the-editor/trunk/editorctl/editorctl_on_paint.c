#include <malloc.h>
#include <assert.h>

#include "editorctl.h"

typedef enum tagCELL_STYLE
{
    STYLE_NONE,
    STYLE_NORMAL,
    STYLE_SELECTED,
    STYLE_WHITESPACE,
    STYLE_WHITESPACE_SELECTED
} CELL_STYLE;

static BOOL render_row (EDITORCTL_EXTRA *extra, HWND hwnd, int row, int start_col, int width, EDITORCTL_UNICODE_CHAR chars [], CELL_STYLE styles [])
{
    int col, end_col, state;
    EDITORCTL_TEXT_ITERATOR it;
    int end_offset;

    end_col = start_col + width;

    if (row < extra->row_count && start_col < extra->row_widths [row])
    {
        int offset;

        if (!editorctl_rc_to_offset (hwnd, row, start_col, &offset, &col)) goto error;
        if (!editorctl_set_iterator (hwnd, offset, &it)) goto error;
        end_offset = row < extra->row_count - 1 ? extra->row_offsets [row + 1] : extra->text_length;
        state = 0;
    }
    else
    {
        end_offset = -1;
        state = 1;
        col = start_col;
    }

    while (col < end_col)
    {
        EDITORCTL_UNICODE_CHAR ch;
        CELL_STYLE style;

        switch (state)
        {
        case 0: // Before char
            if (it.offset >= end_offset)
            {
                EDITORCTL_UNICODE_CHAR pch;

                pch = editorctl_get_prev_char (&it);
                if (pch != '\n' && pch != '\r' && row < extra->row_count - 1)
                {
                    style = STYLE_WHITESPACE;
                    ch = EDITORCTL_WHITESPACE_WRAP;
                }
                else
                {
                    style = STYLE_NONE;
                    ch = 0;
                }

                state = 1;
                if (col < start_col) col = start_col;
            }
            else
            {
                editorctl_forward (&it);
                ch = editorctl_get_prev_char (&it);

                if (ch == ' ')
                {
                    style = STYLE_WHITESPACE;
                    ch = EDITORCTL_WHITESPACE_SPACE;
                }
                else if (ch == '\r')
                {
                    style = STYLE_WHITESPACE;
                    ch = EDITORCTL_WHITESPACE_CR;
                }
                else if (ch == '\n')
                {
                    style = STYLE_WHITESPACE;
                    ch = EDITORCTL_WHITESPACE_LF;
                }
                else if (ch == '\t')
                {
                    style = STYLE_WHITESPACE;
                    if ((col + 1) % extra->tab_width == 0)
                    {
                        ch = EDITORCTL_WHITESPACE_TAB;
                    }
                    else
                    {
                        ch = EDITORCTL_WHITESPACE_TAB_START;
                        state = 2;
                    }
                }
                else
                {
                    style = STYLE_NORMAL;
                }
            }
            break;
        case 1: // After end of line
            style = STYLE_NONE;
            ch = 0;
            break;
        case 2: // Inside the tab
            style = STYLE_WHITESPACE;
            if ((col + 1) % extra->tab_width == 0)
            {
                ch = EDITORCTL_WHITESPACE_TAB_END;
                state = 0;
            }
            else
            {
                ch = EDITORCTL_WHITESPACE_TAB_MIDDLE;
            }
            break;
        }

        if (col >= start_col)
        {
            chars [col - start_col] = ch;
            styles [col - start_col] = style;
        }

        col++;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL paint_unicode_text (HDC hdc, RECT *r, const EDITORCTL_UNICODE_CHAR chars [], int count, int w)
{
    WCHAR *wchars = NULL;
    INT *widths = NULL;
    int i, n;

    wchars = (WCHAR *)_malloca (count * 2 * sizeof (WCHAR));
    widths = (INT *)_malloca (count * 2 * sizeof (INT));

    n = 0;
    for (i = 0; i < count; i++)
    {
        EDITORCTL_UNICODE_CHAR ch = chars [i];
        if (ch <= 0xD7FF || (ch >= 0xE000 && ch <= 0xFFFF))
        {
            wchars [n] = (WCHAR)ch;
            widths [n] = w;
            n++;
        }
        else if (ch >= 0x10000 && ch <= 0x10FFFF)
        {
            ch -= 0x10000;

            wchars [n] = (WCHAR)(0xD800 + (ch >> 10));
            widths [n] = 0;
            n++;

            wchars [n] = (WCHAR)(0xDC00 + (ch & 0x3FF));
            widths [n] = w;
            n++;
        }
    }

    if (!ExtTextOutW (hdc, r->left, r->top, ETO_CLIPPED | ETO_OPAQUE, r, wchars, n, widths)) goto error;

    _freea (wchars);
    _freea (widths);
    return TRUE;
error:
    if (wchars != NULL) _freea (wchars);
    if (widths != NULL) _freea (widths);
    return FALSE;
}

static BOOL paint_whitespace (EDITORCTL_EXTRA *extra, HDC hdc, int x, int y, const EDITORCTL_UNICODE_CHAR chars [], int count, BOOL selected)
{
    HDC mem_dc = NULL;
    int i, w, h;
    HBITMAP old_bitmap = NULL;

    if ((mem_dc = CreateCompatibleDC (hdc)) == NULL) goto error;
    if (SelectObject (mem_dc, extra->whitespace_icons) == NULL) goto error;

    w = extra->cell_size.cx;
    h = extra->cell_size.cy;

    for (i = 0; i < count; i++)
    {
        if (!BitBlt (hdc, x + i * w, y, w, h, mem_dc, chars [i] * w, selected ? h : 0, SRCCOPY)) goto error;
    }

    DeleteDC (mem_dc);

    return TRUE;
error:
    if (mem_dc != NULL)
        DeleteDC (mem_dc);

    return FALSE;
}

static BOOL paint_row (EDITORCTL_EXTRA *extra, HDC hdc, int row, int start_col, int width, const EDITORCTL_UNICODE_CHAR chars [], CELL_STYLE style)
{
    RECT r;

    r.left = start_col * extra->cell_size.cx - extra->scroll_location.x;
    r.top = row * extra->cell_size.cy - extra->scroll_location.y;
    r.right = (start_col + width) * extra->cell_size.cx - extra->scroll_location.x;
    r.bottom = (row + 1) * extra->cell_size.cy - extra->scroll_location.y;

    switch (style)
    {
    case STYLE_NONE:
        if (!FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1))) goto error;
        break;
    case STYLE_NORMAL:
    case STYLE_SELECTED:
        if (SetBkColor (hdc, GetSysColor (style == STYLE_NORMAL ? COLOR_WINDOW : COLOR_HIGHLIGHT)) == CLR_INVALID) goto error;
        if (SetTextColor (hdc, GetSysColor (style == STYLE_NORMAL ? COLOR_WINDOWTEXT : COLOR_HIGHLIGHTTEXT)) == CLR_INVALID) goto error;
        if (!paint_unicode_text (hdc, &r, chars, width, extra->cell_size.cx)) goto error;
        break;
    case STYLE_WHITESPACE:
    case STYLE_WHITESPACE_SELECTED:
        if (!paint_whitespace (extra, hdc, r.left, r.top, chars, width, style != STYLE_WHITESPACE)) goto error;
        break;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL paint_rectangle (EDITORCTL_EXTRA *extra, HWND hwnd, HDC hdc, int start_col, int start_row, int end_col, int end_row)
{
    int width;
    EDITORCTL_UNICODE_CHAR *chars = NULL;
    CELL_STYLE *styles = NULL;
    int row;

    width = end_col - start_col;

    chars = (EDITORCTL_UNICODE_CHAR *)_malloca (width * sizeof (EDITORCTL_UNICODE_CHAR));
    styles = (CELL_STYLE *)_malloca (width * sizeof (CELL_STYLE));

    for (row = start_row; row < end_row; row++)
    {
        int col, chunk_begin_col;
        CELL_STYLE current_style;
        EDITORCTL_UNICODE_CHAR *char_ptr;
        CELL_STYLE *style_ptr;

        if (!render_row (extra, hwnd, row, start_col, width, chars, styles)) goto error;

        chunk_begin_col = start_col;
        current_style = STYLE_NONE;
        char_ptr = chars;
        style_ptr = styles;

        for (col = start_col; col < end_col; col++)
        {
            EDITORCTL_UNICODE_CHAR c = *char_ptr++;
            CELL_STYLE s = *style_ptr++;

            if (s != current_style)
            {
                if (col > chunk_begin_col)
                {
                    int chunk_begin_offset = chunk_begin_col - start_col;

                    if (!paint_row (extra, hdc, row, chunk_begin_col, col - chunk_begin_col, chars + chunk_begin_offset, current_style)) goto error;
                }

                current_style = s;
                chunk_begin_col = col;
            }
        }

        if (col > chunk_begin_col)
        {
            int chunk_begin_offset = chunk_begin_col - start_col;

            if (!paint_row (extra, hdc, row, chunk_begin_col, col - chunk_begin_col, chars + chunk_begin_offset, current_style)) goto error;
        }
    }

    _freea (chars);
    _freea (styles);
    return TRUE;
error:
    if (chars != NULL) _freea (chars);
    if (styles != NULL) _freea (styles);
    return FALSE;
}

LRESULT editorctl_on_paint (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    PAINTSTRUCT ps;
    HDC hdc = NULL;
    HFONT old_font = NULL;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if ((hdc = BeginPaint (hwnd, &ps)) == NULL) goto error;

    old_font = (HFONT)SelectObject (hdc, extra->font ? extra->font : GetStockObject (SYSTEM_FONT));

    paint_rectangle (
        extra, hwnd, hdc,
        (ps.rcPaint.left + extra->scroll_location.x) / extra->cell_size.cx,
        (ps.rcPaint.top + extra->scroll_location.y) / extra->cell_size.cy,
        (ps.rcPaint.right + extra->scroll_location.x - 1) / extra->cell_size.cx + 1,
        (ps.rcPaint.bottom + extra->scroll_location.y - 1) / extra->cell_size.cy + 1);

    SelectObject (hdc, old_font);
    EndPaint (hwnd, &ps);

    return 0;
error:
    if (old_font != NULL) SelectObject (hdc, old_font);
    if (hdc != NULL) EndPaint (hwnd, &ps);
    return -1;
}
