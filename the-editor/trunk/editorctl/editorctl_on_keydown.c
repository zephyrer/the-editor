#include "editorctl.h"

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

static BOOL left (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_backward (&it) && it.prev_char == '\r' && it.next_char == '\n');
    if (!editorctl_move_cursor (hwnd, it.offset)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL right (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_forward (&it) && it.prev_char == '\r' && it.next_char == '\n');
    if (!editorctl_move_cursor (hwnd, it.offset)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL home (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    EDITORCTL_TEXT_ITERATOR it;
    int offset;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (it.prev_char != '\r' && it.prev_char != '\n')
    {
        if (!editorctl_backward (&it)) break;
    }

    offset = it.offset;

    while (it.next_char == '\t' || it.next_char == ' ') editorctl_forward (&it);

    if (!it.is_end && it.next_char != '\r' && it.next_char != '\n' && it.offset != extra->caret_offset) offset = it.offset;

    if (!editorctl_move_cursor (hwnd, offset)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL end (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (it.next_char != '\r' && it.next_char != '\n')
    {
        if (!editorctl_forward (&it)) break;
    }
        
    if (!editorctl_move_cursor (hwnd, it.offset)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL backspace (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    if (extra->anchor_offset != extra->caret_offset)
    {
        if (!delete_selection (hwnd, extra)) goto error;
    }
    else
    {
        EDITORCTL_TEXT_ITERATOR it;

        if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
        while (editorctl_backward (&it) && it.prev_char == '\r' && it.next_char == '\n');
        if (!editorctl_replace_range (hwnd, it.offset, extra->caret_offset - it.offset, NULL, 0, it.offset)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL del (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    if (extra->anchor_offset != extra->caret_offset)
    {
        if (!delete_selection (hwnd, extra)) goto error;
    }
    else
    {
        EDITORCTL_TEXT_ITERATOR it;

        if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
        while (editorctl_forward (&it) && it.prev_char == '\r' && it.next_char == '\n');
        if (!editorctl_replace_range (hwnd, extra->caret_offset, it.offset - extra->caret_offset, NULL, 0, extra->caret_offset)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

LRESULT editorctl_on_keydown (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    BOOL control, alt, shift;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    control = (GetKeyState (VK_CONTROL) & 0x8000) != 0;
    alt = (GetKeyState (VK_MENU) & 0x8000) != 0;
    shift = (GetKeyState (VK_SHIFT) & 0x8000) != 0;

    if (!control && !alt && !shift)
    {
        switch (wParam)
        {
        case VK_LEFT:
            if (!left (hwnd, extra)) goto error;
            break;
        case VK_RIGHT:
            if (!right (hwnd, extra)) goto error;
            break;
        case VK_HOME:
            if (!home (hwnd, extra)) goto error;
            break;
        case VK_END:
            if (!end (hwnd, extra)) goto error;
            break;
        case VK_INSERT:
            if (!toggle_insert (hwnd, extra)) goto error;
            break;
        case VK_BACK:
            if (!backspace (hwnd, extra)) goto error;
            break;
        case VK_DELETE:
            if (!del (hwnd, extra)) goto error;
            break;
        }
    }

    return 0;
error:
    return -1;
}
