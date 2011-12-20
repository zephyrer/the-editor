#include "editorctl.h"

static BOOL is_eol (EDITORCTL_EXTRA *extra, int offset)
{
    char *ptr;

    if (offset < extra->text_length)
    {
        EDITORCTL_UNICODE_CHAR ch;

        ptr = extra->text + offset;
        ch = editorctl_get_next_char (&ptr);
        return ch == '\r' || ch == '\n';
    }
    else return TRUE;
}

static BOOL insert (HWND hwnd, EDITORCTL_EXTRA *extra, char *buffer, int buffer_length, BOOL overwrite)
{
    int start_offset;
    int length;

    if (extra->anchor_offset != extra->caret_offset)
    {
        start_offset = min (extra->anchor_offset, extra->caret_offset);
        length = abs (extra->anchor_offset - extra->caret_offset);
    }
    else
    {
        start_offset = extra->caret_offset;

        if (overwrite && !is_eol (extra, start_offset))
        {
            char *ptr;

            ptr = extra->text + start_offset;
            editorctl_get_next_char (&ptr);
            length = ptr - extra->text - start_offset;
        }
        else length = 0;
    }

    if (!editorctl_replace_range (hwnd, start_offset, length, buffer, buffer_length, start_offset + buffer_length)) goto error;

    return TRUE;
error:
    return FALSE;
}

LRESULT editorctl_on_char (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;
    char buffer [EDITORCTL_MAX_CHAR_WIDTH];
    char *ptr;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    switch (wParam)
    {
    case 0x0A: 
    case 0x0D: 
        if (!insert (hwnd, extra, extra->new_line, extra->new_line_length, FALSE)) goto error;
        break;
    case 0x08: 
    case 0x1B: 
    case 0x7F:
        break;
    default:
        ptr = buffer;
        editorctl_set_next_char (wParam, &ptr);
        if (!insert (hwnd, extra, buffer, ptr - buffer, extra->overwrite)) goto error;
    }

    return 0;
error:
    return -1;
}
