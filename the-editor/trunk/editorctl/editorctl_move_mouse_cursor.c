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

static BOOL set_selection (HWND hwnd, EDITORCTL_EXTRA *extra, int anchor, int caret, int mouse)
{
    int r1, r2, r3, r4, r, c;

    if (!editorctl_offset_to_row (hwnd, extra->anchor_offset, &r1)) goto error;
    if (!editorctl_offset_to_row (hwnd, extra->caret_offset, &r2)) goto error;
    if (!editorctl_offset_to_row (hwnd, anchor, &r3)) goto error;
    if (!editorctl_offset_to_row (hwnd, caret, &r4)) goto error;

    if (extra->anchor_offset != extra->caret_offset)
    {
        if (!editorctl_invalidate_rows (hwnd, min (r1, r3), max (r1, r3) + 1)) goto error;
        if (!editorctl_invalidate_rows (hwnd, min (r2, r4), max (r2, r4) + 1)) goto error;
    }
    else
    {
        if (!editorctl_invalidate_rows (hwnd, min (r3, r4), max (r3, r4) + 1)) goto error;
    }

    extra->anchor_offset = anchor;
    extra->caret_offset = caret;

    if (!editorctl_offset_to_rc (hwnd, mouse, &r, &c)) goto error;
    if (!editorctl_ensure_cell_visible (hwnd, r, c)) goto error;
    if (!editorctl_update_caret_pos (hwnd)) goto error;

    return TRUE;
error:
    return FALSE;
}

static int prev_word_boundary (HWND hwnd, int offset)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, offset, &it)) goto error;

    while (!is_word_boundary (&it))
    {
        if (!editorctl_backward (&it)) break;
    }

    return it.offset;
error:
    return offset;
}

static int next_word_boundary (HWND hwnd, int offset)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, offset, &it)) goto error;

    do 
    {
        if (!editorctl_forward (&it)) break;
    } while (!is_word_boundary (&it));

    return it.offset;
error:
    return offset;
}

static int prev_row_boundary (HWND hwnd, int offset)
{
    int row, col, result;

    if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;
    if (!editorctl_rc_to_offset (hwnd, row, 0, &result, &col)) goto error;

    return result;
error:
    return offset;
}

static int next_row_boundary (HWND hwnd, int offset)
{
    EDITORCTL_EXTRA *extra;
    int row, col, result;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (!editorctl_offset_to_rc (hwnd, offset, &row, &col)) goto error;
    if (!editorctl_rc_to_offset (hwnd, row, extra->row_widths [row], &result, &col)) goto error;

    return result;
error:
    return offset;
}

BOOL editorctl_move_mouse_cursor (HWND hwnd, int x, int y, BOOL selecting)
{
    EDITORCTL_EXTRA *extra;
    int row, col, offset;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    row = (y + extra->scroll_location.y) / extra->cell_size.cy;
    col = (x + extra->scroll_location.x + extra->cell_size.cx * 2 / 3) / extra->cell_size.cx;

    if (row >= extra->row_count)
        row = extra->row_count - 1;

    if (!editorctl_rc_to_offset (hwnd, row, col, &offset, &col)) goto error;

    if (!selecting)
        extra->mouse_anchor_offset = offset;

    switch (extra->mouse_select_mode)
    {
    case EDITORCTL_MSM_CHAR:
        if (!set_selection (hwnd, extra, extra->mouse_anchor_offset, offset, offset)) goto error;
        break;
    case EDITORCTL_MSM_WORD:
        if (prev_word_boundary (hwnd, extra->mouse_anchor_offset) <= offset)
        {
            if (!set_selection (hwnd, extra, prev_word_boundary (hwnd, extra->mouse_anchor_offset), next_word_boundary (hwnd, offset), offset)) goto error;
        }
        else
        {
            if (!set_selection (hwnd, extra, next_word_boundary (hwnd, extra->mouse_anchor_offset), prev_word_boundary (hwnd, offset), offset)) goto error;
        }
        break;
    case EDITORCTL_MSM_ROW:
        if (prev_row_boundary (hwnd, extra->mouse_anchor_offset) <= offset)
        {
            if (!set_selection (hwnd, extra, prev_row_boundary (hwnd, extra->mouse_anchor_offset), next_row_boundary (hwnd, offset), offset)) goto error;
        }
        else
        {
            if (!set_selection (hwnd, extra, next_row_boundary (hwnd, extra->mouse_anchor_offset), prev_row_boundary (hwnd, offset), offset)) goto error;
        }
        break;
    }

    return TRUE;
error:
    return FALSE;
}
