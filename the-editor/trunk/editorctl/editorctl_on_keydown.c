#include "editorctl.h"

static BOOL is_word_boundary (EDITORCTL_TEXT_ITERATOR *it)
{
    EDITORCTL_UNICODE_CHAR pch, nch;

    pch = editorctl_get_prev_char (it);
    nch = editorctl_get_next_char (it);

    if (pch == '\r' && nch == '\n') return FALSE;
    else if (pch == '\r' || pch == '\n' || nch == '\r' || nch == '\n'|| nch == 0 || nch == 0) return TRUE;
    else if (iswalnum (pch) && iswalnum (nch)) return FALSE;
    else if (iswspace (pch) && iswspace (nch)) return FALSE;
    else return TRUE;
}

static BOOL is_boundary (EDITORCTL_TEXT_ITERATOR *it)
{
    EDITORCTL_UNICODE_CHAR pch, nch;

    pch = editorctl_get_prev_char (it);
    nch = editorctl_get_next_char (it);

    if (pch == '\r' && nch == '\n') return FALSE;
    else return TRUE;
}

static BOOL toggle_insert (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    extra->overwrite = !extra->overwrite;

    if (GetFocus () == hwnd)
    {
        if (!DestroyCaret ()) goto error;
        if (!CreateCaret (hwnd, (HBITMAP) NULL, extra->overwrite ? extra->cell_size.cx : 1, extra->cell_size.cy)) goto error;
        if (!editorctl_update_caret_pos (hwnd)) goto error;
        if (!ShowCaret (hwnd)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL delete_selection (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    if (!editorctl_replace_range (hwnd, min (extra->anchor_offset, extra->caret_offset), abs (extra->anchor_offset - extra->caret_offset), NULL, 0, min (extra->anchor_offset, extra->caret_offset))) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL hmove (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL forward, BOOL word, BOOL selecting)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (forward ? editorctl_forward (&it) : editorctl_backward (&it))
    {
        if (word ? is_word_boundary (&it) : is_boundary (&it)) break;
    }
    if (!editorctl_move_cursor (hwnd, it.offset, selecting)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL vmove (HWND hwnd, EDITORCTL_EXTRA *extra, int delta, BOOL selecting)
{
    int row, col, offset, real_col;

    if (extra->caret_column >= 0)
    {
        if (!editorctl_offset_to_row (hwnd, extra->caret_offset, &row)) goto error;
        col = extra->caret_column;
    }
    else if (!editorctl_offset_to_rc (hwnd, extra->caret_offset, &row, &col)) goto error;

    if (delta > 0 && row == extra->row_count - 1)
    {
        if (!editorctl_offset_to_rc (hwnd, extra->text_length, &row, &col)) goto error;
    }
    else if (delta < 0 && row == 0)
    {
        col = 0;
    }
    else
    {
        row += delta;
        if (row >= extra->row_count)
        {
            row = extra->row_count - 1;
        }
        if (row < 0) row = 0;
    }

    if (!editorctl_rc_to_offset (hwnd, row, col, &offset, &real_col)) goto error;

    if (!editorctl_move_cursor (hwnd, offset, selecting)) goto error;
    extra->caret_column = col;

    return TRUE;
error:
    return FALSE;
}

static BOOL home (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL selecting)
{
    EDITORCTL_TEXT_ITERATOR it;
    int offset;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_get_prev_char (&it) != '\r' && editorctl_get_prev_char (&it) != '\n')
    {
        if (!editorctl_backward (&it)) break;
    }

    offset = it.offset;

    while (editorctl_get_next_char (&it) == '\t' || editorctl_get_next_char (&it) == ' ') editorctl_forward (&it);

    if (it.offset < it.text_length && editorctl_get_next_char (&it) != '\r' && editorctl_get_next_char (&it) != '\n' && it.offset != extra->caret_offset) offset = it.offset;

    if (!editorctl_move_cursor (hwnd, offset, selecting)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL end (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL selecting)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_get_next_char (&it) != '\r' && editorctl_get_next_char (&it) != '\n')
    {
        if (!editorctl_forward (&it)) break;
    }
        
    if (!editorctl_move_cursor (hwnd, it.offset, selecting)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL del (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL word, BOOL forward)
{
    if (extra->anchor_offset != extra->caret_offset)
    {
        if (!delete_selection (hwnd, extra)) goto error;
    }
    else
    {
        EDITORCTL_TEXT_ITERATOR it;
        int offset, length;

        if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
        while (forward ? editorctl_forward (&it) : editorctl_backward (&it))
        {
            if (word ? is_word_boundary (&it) : is_boundary (&it)) break;
        }

        offset = forward ? extra->caret_offset : it.offset;
        length = (forward ? it.offset : extra->caret_offset) - offset;

        if (!editorctl_replace_range (hwnd, offset, length, NULL, 0, offset)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL append (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    char *buffer = NULL;
    int i;

    if ((buffer = (char *)HeapAlloc (extra->heap, 0, 100000000)) == NULL) goto error;

    for (i = 0; i < 100000000; i++)
    {
        if (i % 100000 == 0)
            buffer [i] = "\r\n" [rand () % 2];
        else
            buffer [i] = "abcdef \t" [rand () % 8];
    }

    if (!editorctl_replace_range (hwnd, extra->text_length, 0, buffer, 100000000, extra->caret_offset)) goto error;

    HeapFree (extra->heap, 0, buffer);

    return TRUE;
error:
    if (buffer != NULL)
        HeapFree (extra->heap, 0, buffer);

    return FALSE;
}

LRESULT editorctl_on_keydown (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    BOOL control, alt, shift;
    int page, first, last, caret_row;
    RECT r;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;
    page = (r.bottom - r.top) / extra->cell_size.cy;
    page = max (page, 1);

    first = (r.top + extra->scroll_location.y + extra->cell_size.cy - 1) / extra->cell_size.cy;
    last = (r.bottom + extra->scroll_location.y) / extra->cell_size.cy - 1;

    if (first > last)
    {
        first = (r.top + extra->scroll_location.y) / extra->cell_size.cy;
        last = (r.bottom + extra->scroll_location.y - 1) / extra->cell_size.cy;
    }

    if (!editorctl_offset_to_row (hwnd, extra->caret_offset, &caret_row)) goto error;

    control = (GetKeyState (VK_CONTROL) & 0x8000) != 0;
    alt = (GetKeyState (VK_MENU) & 0x8000) != 0;
    shift = (GetKeyState (VK_SHIFT) & 0x8000) != 0;

    if (!alt)
    {
        if (control)
        {
            switch (wParam)
            {
            case VK_LEFT:
                if (!hmove (hwnd, extra, FALSE, TRUE, shift)) goto error;
                break;
            case VK_RIGHT:
                if (!hmove (hwnd, extra, TRUE, TRUE, shift)) goto error;
                break;
            case VK_UP:
                if (!editorctl_scroll_to (hwnd, extra->scroll_location.x, max (0, extra->scroll_location.y - extra->cell_size.cy))) goto error;
                break;
            case VK_DOWN:
                if (!editorctl_scroll_to (hwnd, extra->scroll_location.x, max (0, min (extra->row_count * extra->cell_size.cy - r.bottom + r.top, extra->scroll_location.y + extra->cell_size.cy)))) goto error;
                break;
            case VK_HOME:
                if (!editorctl_move_cursor (hwnd, 0, shift)) goto error;
                break;
            case VK_END:
                if (!editorctl_move_cursor (hwnd, extra->text_length, shift)) goto error;
                break;
            case VK_PRIOR:
                if (!vmove (hwnd, extra, first - caret_row, shift)) goto error;
                break;
            case VK_NEXT:
                if (!vmove (hwnd, extra, last - caret_row, shift)) goto error;
                break;
            case VK_BACK:
                if (!shift)
                {
                    if (!del (hwnd, extra, TRUE, FALSE)) goto error;
                }
                break;
            case VK_DELETE:
                if (!shift)
                {
                    if (!del (hwnd, extra, TRUE, TRUE)) goto error;
                }
                break;
            case 0x41: // key `A'
                if (!shift)
                {
                    if (!editorctl_move_cursor (hwnd, 0, FALSE)) goto error;
                    if (!editorctl_move_cursor (hwnd, extra->text_length, TRUE)) goto error;
                }
                break;
            }
        }
        else
        {
            switch (wParam)
            {
            case VK_LEFT:
                if (!hmove (hwnd, extra, FALSE, FALSE, shift)) goto error;
                break;
            case VK_RIGHT:
                if (!hmove (hwnd, extra, TRUE, FALSE, shift)) goto error;
                break;
            case VK_HOME:
                if (!home (hwnd, extra, shift)) goto error;
                break;
            case VK_END:
                if (!end (hwnd, extra, shift)) goto error;
                break;
            case VK_UP:
                if (!vmove (hwnd, extra, -1, shift)) goto error;
                break;
            case VK_DOWN:
                if (!vmove (hwnd, extra, 1, shift)) goto error;
                break;
            case VK_PRIOR:
                if (!vmove (hwnd, extra, -page, shift)) goto error;
                break;
            case VK_NEXT:
                if (!vmove (hwnd, extra, page, shift)) goto error;
                break;
            case VK_INSERT:
                if (!shift)
                {
                    if (!toggle_insert (hwnd, extra)) goto error;
                }
                break;
            case VK_BACK:
                if (!shift)
                {
                    if (!del (hwnd, extra, FALSE, FALSE)) goto error;
                }
                break;
            case VK_DELETE:
                if (!shift)
                {
                    if (!del (hwnd, extra, FALSE, TRUE)) goto error;
                }
                break;
            case VK_F12:
                if (!shift)
                {
                    if (!append (hwnd, extra)) goto error;
                }
                break;
            }
        }
    }

    return 0;
error:
    return -1;
}
