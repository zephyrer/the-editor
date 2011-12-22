#include "editorctl.h"

static BOOL ensure_text_capacity (EDITORCTL_EXTRA *extra, int required_capacity)
{
    int current_capacity;

    if (extra->text != NULL)
    {
        if ((current_capacity = HeapSize (extra->heap, 0, extra->text)) == (SIZE_T)-1) goto error;
    }
    else current_capacity = 0;

    if (required_capacity > current_capacity)
    {
        int new_capacity;
        char *new_text;

        new_capacity = required_capacity * 3 / 2;

        if (extra->text != NULL)
        {
            if ((new_text = (char *)HeapReAlloc (extra->heap, 0, extra->text, new_capacity)) == NULL) goto error;
        }
        else
        {
            if ((new_text = (char *)HeapAlloc (extra->heap, 0, new_capacity)) == NULL) goto error;
        }

        extra->text = new_text;
    }

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_replace_range (HWND hwnd, int offset, int length, const char *buffer, int buffer_length, int new_caret_offset)
{
    EDITORCTL_EXTRA *extra = NULL;
    RECT r;
    int w, h, width, height, sx, sy;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    if (buffer_length > length)
    {
        if (!ensure_text_capacity (extra, extra->text_length + buffer_length - length)) goto error;
    }

    if (buffer_length != length)
    {
        CopyMemory (
            extra->text + offset + buffer_length,
            extra->text + offset + length,
            extra->text_length - offset - length);
    }

    CopyMemory (
        extra->text + offset,
        buffer,
        buffer_length);

    extra->text_length = extra->text_length + buffer_length - length;

    if (!editorctl_update (hwnd, offset, length, buffer_length)) goto error;

    if (!GetClientRect (hwnd, &r)) goto error;

    w = r.right - r.left;
    width = extra->column_count * extra->cell_size.cx;
    sx = extra->scroll_location.x;
    if (sx + w > width)
        sx = max (0, width - w);

    h = r.bottom - r.top;
    height = extra->row_count * extra->cell_size.cy;
    sy = extra->scroll_location.y;
    if (sy + h > height)
        sy = max (0, height - h);

    if (!editorctl_scroll_to (hwnd, sx, sy)) goto error;
    if (!editorctl_update_scroll_range (hwnd)) goto error;

    if (!editorctl_move_cursor (hwnd, new_caret_offset, FALSE)) goto error;

    return TRUE;
error:
    return FALSE;
}
