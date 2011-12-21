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

static BOOL left (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL selecting)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_backward (&it) && editorctl_get_prev_char (&it) == '\r' && editorctl_get_next_char (&it) == '\n');
    if (!editorctl_move_cursor (hwnd, it.offset, selecting)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL right (HWND hwnd, EDITORCTL_EXTRA *extra, BOOL selecting)
{
    EDITORCTL_TEXT_ITERATOR it;

    if (!editorctl_set_iterator (hwnd, extra->caret_offset, &it)) goto error;
    while (editorctl_forward (&it) && editorctl_get_prev_char (&it) == '\r' && editorctl_get_next_char (&it) == '\n');
    if (!editorctl_move_cursor (hwnd, it.offset, selecting)) goto error;

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
        while (editorctl_backward (&it) && editorctl_get_prev_char (&it) == '\r' && editorctl_get_next_char (&it) == '\n');
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
        while (editorctl_forward (&it) && editorctl_get_prev_char (&it) == '\r' && editorctl_get_next_char (&it) == '\n');
        if (!editorctl_replace_range (hwnd, extra->caret_offset, it.offset - extra->caret_offset, NULL, 0, extra->caret_offset)) goto error;
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

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    control = (GetKeyState (VK_CONTROL) & 0x8000) != 0;
    alt = (GetKeyState (VK_MENU) & 0x8000) != 0;
    shift = (GetKeyState (VK_SHIFT) & 0x8000) != 0;

    if (!control && !alt)
    {
        switch (wParam)
        {
        case VK_LEFT:
            if (!left (hwnd, extra, shift)) goto error;
            break;
        case VK_RIGHT:
            if (!right (hwnd, extra, shift)) goto error;
            break;
        case VK_HOME:
            if (!home (hwnd, extra, shift)) goto error;
            break;
        case VK_END:
            if (!end (hwnd, extra, shift)) goto error;
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
                if (!backspace (hwnd, extra)) goto error;
            }
            break;
        case VK_DELETE:
            if (!shift)
            {
                if (!del (hwnd, extra)) goto error;
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

    return 0;
error:
    return -1;
}
