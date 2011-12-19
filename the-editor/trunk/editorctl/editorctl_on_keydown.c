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
    if (!editorctl_replace_range (hwnd, min (extra->anchor_offset, extra->caret_offset), abs (extra->anchor_offset - extra->caret_offset), NULL, 0)) goto error;
    extra->caret_offset = min (extra->anchor_offset, extra->caret_offset);
    extra->anchor_offset = extra->caret_offset;
    if (!editorctl_update_caret_pos (hwnd)) goto error;

    return TRUE;
error:
    return FALSE;
}

static BOOL left (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    if (extra->caret_offset > 0)
    {
        char *ptr;
        int start_offset;
        EDITORCTL_UNICODE_CHAR ch;

        ptr = extra->text + extra->caret_offset;
        ch = editorctl_get_prev_char (&ptr);
        start_offset = ptr - extra->text;
        if (ch == '\n' && ptr > extra->text)
        {
            ch = editorctl_get_prev_char (&ptr);
            if (ch == '\r')
                start_offset = ptr - extra->text;
        }

        extra->caret_offset = start_offset;
        extra->anchor_offset = start_offset;
        if (!editorctl_update_caret_pos (hwnd)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static BOOL right (HWND hwnd, EDITORCTL_EXTRA *extra)
{
    if (extra->caret_offset < extra->text_length)
    {
        char *ptr;
        int end_offset;
        EDITORCTL_UNICODE_CHAR ch;

        ptr = extra->text + extra->caret_offset;
        ch = editorctl_get_next_char (&ptr);
        end_offset = ptr - extra->text;
        if (ch == '\r' && ptr < extra->text + extra->text_length)
        {
            ch = editorctl_get_next_char (&ptr);
            if (ch == '\n')
                end_offset = ptr - extra->text;
        }

        extra->caret_offset = end_offset;
        extra->anchor_offset = end_offset;
        if (!editorctl_update_caret_pos (hwnd)) goto error;
    }

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
    else if (extra->caret_offset > 0)
    {
        char *ptr;
        int start_offset;
        EDITORCTL_UNICODE_CHAR ch;

        ptr = extra->text + extra->caret_offset;
        ch = editorctl_get_prev_char (&ptr);
        start_offset = ptr - extra->text;
        if (ch == '\n' && ptr > extra->text)
        {
            ch = editorctl_get_prev_char (&ptr);
            if (ch == '\r')
                start_offset = ptr - extra->text;
        }

        if (!editorctl_replace_range (hwnd, start_offset, extra->caret_offset - start_offset, NULL, 0)) goto error;
        extra->caret_offset = start_offset;
        extra->anchor_offset = start_offset;
        if (!editorctl_update_caret_pos (hwnd)) goto error;
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
    else if (extra->caret_offset < extra->text_length)
    {
        char *ptr;
        int end_offset;
        EDITORCTL_UNICODE_CHAR ch;

        ptr = extra->text + extra->caret_offset;
        ch = editorctl_get_next_char (&ptr);
        end_offset = ptr - extra->text;
        if (ch == '\r' && ptr < extra->text + extra->text_length)
        {
            ch = editorctl_get_next_char (&ptr);
            if (ch == '\n')
                end_offset = ptr - extra->text;
        }

        if (!editorctl_replace_range (hwnd, extra->caret_offset, end_offset - extra->caret_offset, NULL, 0)) goto error;
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
