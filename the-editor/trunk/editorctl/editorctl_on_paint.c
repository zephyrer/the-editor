#include <malloc.h>
#include <assert.h>

#include "../text/text.h"
#include "../layout/layout.h"
#include "../editor/editor.h"
#include "editorctl.h"

typedef enum tagCELL_STYLE
{
    STYLE_NONE,
    STYLE_NORMAL,
    STYLE_SELECTED,
    STYLE_WHITESPACE,
    STYLE_WHITESPACE_SELECTED
} CELL_STYLE;

static void render_row (EDITORCTL *editorctl, int row, int start_col, int width, int chars [], CELL_STYLE styles [])
{
    if (row < editorctl->editor.layout.row_offsets.length)
    {
        int offset, length, i;
        int *offset_buffer;
        int sb, se;

        offset = editorctl->editor.layout.row_offsets.data [row];
        length = (row < editorctl->editor.layout.row_offsets.length - 1 ? editorctl->editor.layout.row_offsets.data [row + 1] : editorctl->editor.text.length) - offset;
        offset_buffer = (int *)_alloca (width * sizeof (int));
        sb = min (editorctl->editor.anchor_offset, editorctl->editor.caret_offset);
        se = max (editorctl->editor.anchor_offset, editorctl->editor.caret_offset);

        text_render_row (&editorctl->editor.text, offset, length, 0, start_col, width, chars, offset_buffer, editorctl->editor.layout.tab_width);
        for (i = 0; i < width; i++)
        {
            BOOL selected;

            selected = offset_buffer [i] >= sb && offset_buffer [i] < se;

            switch (chars [i])
            {
            case TEXT_NONE:
                chars [i] = 0;
                styles [i] = STYLE_NONE;
                break;
            case TEXT_WRAP:
                chars [i] = EDITORCTL_WHITESPACE_WRAP;
                styles [i] = STYLE_WHITESPACE;
                break;
            case TEXT_TAB:
                chars [i] = EDITORCTL_WHITESPACE_TAB;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            case TEXT_TAB_BEGIN:
                chars [i] = EDITORCTL_WHITESPACE_TAB_START;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            case TEXT_TAB_MIDDLE:
                chars [i] = EDITORCTL_WHITESPACE_TAB_MIDDLE;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            case TEXT_TAB_END:
                chars [i] = EDITORCTL_WHITESPACE_TAB_END;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            case '\r':
                chars [i] = EDITORCTL_WHITESPACE_CR;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            case '\n':
                chars [i] = EDITORCTL_WHITESPACE_LF;
                styles [i] = selected ? STYLE_WHITESPACE_SELECTED : STYLE_WHITESPACE;
                break;
            default:
                styles [i] = selected ? STYLE_SELECTED : STYLE_NORMAL;
                break;
            }
        }

        _freea (offset_buffer);
    }
    else
    {
        int i;

        for (i = 0; i < width; i++)
        {
            chars [i] = 0;
            styles [i] = STYLE_NONE;
        }
    }
}

static BOOL paint_unicode_text (HDC hdc, RECT *r, const int chars [], int count, int w)
{
    WCHAR *wchars = NULL;
    INT *widths = NULL;
    int i, n;

    wchars = (WCHAR *)_malloca (count * 2 * sizeof (WCHAR));
    widths = (INT *)_malloca (count * 2 * sizeof (INT));

    n = 0;
    for (i = 0; i < count; i++)
    {
        int ch = chars [i];
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

static BOOL paint_whitespace (EDITORCTL *editorctl, HDC hdc, int x, int y, const int chars [], int count, BOOL selected)
{
    HDC mem_dc = NULL;
    int i, w, h;
    HBITMAP old_bitmap = NULL;

    if ((mem_dc = CreateCompatibleDC (hdc)) == NULL) goto error;
    if (SelectObject (mem_dc, editorctl->whitespace_icons) == NULL) goto error;

    w = editorctl->cell_size.cx;
    h = editorctl->cell_size.cy;

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

static BOOL paint_row (EDITORCTL *editorctl, HDC hdc, int row, int start_col, int width, const int chars [], CELL_STYLE style)
{
    RECT r;

    r.left = start_col * editorctl->cell_size.cx - editorctl->scroll_location.x;
    r.top = row * editorctl->cell_size.cy - editorctl->scroll_location.y;
    r.right = (start_col + width) * editorctl->cell_size.cx - editorctl->scroll_location.x;
    r.bottom = (row + 1) * editorctl->cell_size.cy - editorctl->scroll_location.y;

    switch (style)
    {
    case STYLE_NONE:
        if (!FillRect (hdc, &r, (HBRUSH)(COLOR_WINDOW + 1))) goto error;
        break;
    case STYLE_NORMAL:
    case STYLE_SELECTED:
        if (SetBkColor (hdc, GetSysColor (style == STYLE_NORMAL ? COLOR_WINDOW : COLOR_HIGHLIGHT)) == CLR_INVALID) goto error;
        if (SetTextColor (hdc, GetSysColor (style == STYLE_NORMAL ? COLOR_WINDOWTEXT : COLOR_HIGHLIGHTTEXT)) == CLR_INVALID) goto error;
        if (!paint_unicode_text (hdc, &r, chars, width, editorctl->cell_size.cx)) goto error;
        break;
    case STYLE_WHITESPACE:
    case STYLE_WHITESPACE_SELECTED:
        if (!paint_whitespace (editorctl, hdc, r.left, r.top, chars, width, style != STYLE_WHITESPACE)) goto error;
        break;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL paint_rectangle (EDITORCTL *editorctl, HDC hdc, int start_col, int start_row, int end_col, int end_row)
{
    int width;
    int *chars = NULL;
    CELL_STYLE *styles = NULL;
    int row;

    width = end_col - start_col;

    chars = (int *)_malloca (width * sizeof (int));
    styles = (CELL_STYLE *)_malloca (width * sizeof (CELL_STYLE));

    for (row = start_row; row < end_row; row++)
    {
        int col, chunk_begin_col;
        CELL_STYLE current_style;
        int *char_ptr;
        CELL_STYLE *style_ptr;

        render_row (editorctl, row, start_col, width, chars, styles);

        chunk_begin_col = start_col;
        current_style = STYLE_NONE;
        char_ptr = chars;
        style_ptr = styles;

        for (col = start_col; col < end_col; col++)
        {
            int c = *char_ptr++;
            CELL_STYLE s = *style_ptr++;

            if (s != current_style)
            {
                if (col > chunk_begin_col)
                {
                    int chunk_begin_offset = chunk_begin_col - start_col;

                    if (!paint_row (editorctl, hdc, row, chunk_begin_col, col - chunk_begin_col, chars + chunk_begin_offset, current_style)) goto error;
                }

                current_style = s;
                chunk_begin_col = col;
            }
        }

        if (col > chunk_begin_col)
        {
            int chunk_begin_offset = chunk_begin_col - start_col;

            if (!paint_row (editorctl, hdc, row, chunk_begin_col, col - chunk_begin_col, chars + chunk_begin_offset, current_style)) goto error;
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

LRESULT editorctl_on_paint (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc = NULL;
    HFONT old_font = NULL;

    if ((hdc = BeginPaint (hwnd, &ps)) == NULL) goto error;

    old_font = (HFONT)SelectObject (hdc, editorctl->font ? editorctl->font : GetStockObject (SYSTEM_FONT));

    paint_rectangle (
        editorctl, hdc,
        (ps.rcPaint.left + editorctl->scroll_location.x) / editorctl->cell_size.cx,
        (ps.rcPaint.top + editorctl->scroll_location.y) / editorctl->cell_size.cy,
        (ps.rcPaint.right + editorctl->scroll_location.x - 1) / editorctl->cell_size.cx + 1,
        (ps.rcPaint.bottom + editorctl->scroll_location.y - 1) / editorctl->cell_size.cy + 1);

    SelectObject (hdc, old_font);
    EndPaint (hwnd, &ps);

    return 0;
error:
    if (old_font != NULL) SelectObject (hdc, old_font);
    if (hdc != NULL) EndPaint (hwnd, &ps);
    return -1;
}
