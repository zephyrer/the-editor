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

static void render_row (EDITORCTL_EXTRA *extra, int row, int start_col, int width, EDITORCTL_UNICODE_CHAR chars [], CELL_STYLE styles [])
{
    int col, end_col, state;
    char *ptr, *end_ptr;

    col = 0;
    end_col = start_col + width;

    if (row < extra->line_count)
    {
        state = 0;
        ptr = extra->text + extra->line_offsets [row];
        end_ptr = ptr + extra->line_lengths [row];
    }
    else
        state = 1;

    while (col < end_col)
    {
        EDITORCTL_UNICODE_CHAR ch;
        CELL_STYLE style;

        switch (state)
        {
        case 0: // Before char
            if (ptr >= end_ptr)
            {
                style = STYLE_WHITESPACE;
                ch = 0xB6;
                state = 1;
            }
            else
            {
                ch = editorctl_get_next_char (&ptr);
                if (ch == ' ')
                {
                    style = STYLE_WHITESPACE;
                    ch = 0xB7;
                }
                else if (ch == '\t')
                {
                    style = STYLE_WHITESPACE;
                    if ((col + 1) % extra->tab_width == 0)
                    {
                        ch = 0x2192;
                    }
                    else
                    {
                        ch = 0x2015;
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
            ch = 0;
            style = STYLE_NONE;
            break;
        case 2: // Inside the tab
            if ((col + 1) % extra->tab_width == 0)
            {
                ch = 0x2192;
                state = 0;
            }
            else
            {
                ch = 0x2015;
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

static BOOL paint_row (EDITORCTL_EXTRA *extra, HDC hdc, int row, int start_col, int width, const EDITORCTL_UNICODE_CHAR chars [], CELL_STYLE style)
{
    RECT r;
    COLORREF bk_color;
    COLORREF text_color;

    r.left = start_col * extra->cell_size.cx - extra->scroll_location.x;
    r.top = row * extra->cell_size.cy - extra->scroll_location.y;
    r.right = (start_col + width) * extra->cell_size.cx - extra->scroll_location.x;
    r.bottom = (row + 1) * extra->cell_size.cy - extra->scroll_location.y;

    if (style == STYLE_NONE)
    {
        if (!FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1))) goto error;
    }
    else
    {
        switch (style)
        {
        case STYLE_NORMAL:
            bk_color = GetSysColor (COLOR_WINDOW);
            text_color = GetSysColor (COLOR_WINDOWTEXT);
            break;
        case STYLE_SELECTED:
            bk_color = GetSysColor (COLOR_HIGHLIGHT);
            text_color = GetSysColor (COLOR_HIGHLIGHTTEXT);
            break;
        case STYLE_WHITESPACE:
            bk_color = GetSysColor (COLOR_WINDOW);
            text_color = GetSysColor (COLOR_GRAYTEXT);
            break;
        case STYLE_WHITESPACE_SELECTED:
            bk_color = GetSysColor (COLOR_HIGHLIGHT);
            text_color = GetSysColor (COLOR_GRAYTEXT);
            break;
        default:
            assert (FALSE);
        }

        if (SetBkColor (hdc, bk_color) == CLR_INVALID) goto error;
        if (SetTextColor (hdc, text_color) == CLR_INVALID) goto error;
        if (!paint_unicode_text (hdc, &r, chars, width, extra->cell_size.cx)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL paint_rectangle (EDITORCTL_EXTRA *extra, HDC hdc, int start_col, int start_row, int end_col, int end_row)
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

        render_row (extra, row, start_col, width, chars, styles);

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
    EDITORCTL_EXTRA *extra = NULL;
    PAINTSTRUCT ps;
    HDC hdc = NULL;
    HFONT old_font = NULL;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if ((hdc = BeginPaint (hwnd, &ps)) == NULL) goto error;

    old_font = (HFONT)SelectObject (hdc, extra->font ? extra->font : GetStockObject (SYSTEM_FONT));

    paint_rectangle (
        extra, hdc,
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