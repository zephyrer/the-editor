#include "editorctl.h"

static BOOL toggle_insert (EDITORCTL *editorctl)
{
    editorctl->overwrite = !editorctl->overwrite;

    if (GetFocus () == editorctl->hwnd)
    {
        if (!DestroyCaret ()) goto error;
        if (!CreateCaret (editorctl->hwnd, (HBITMAP) NULL, editorctl->overwrite ? editorctl->cell_size.cx : 1, editorctl->cell_size.cy)) goto error;
        if (!ShowCaret (editorctl->hwnd)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}

static void vmove (EDITORCTL *editorctl, int delta, BOOL selecting)
{
    if (delta < 0)
        editor_up (&editorctl->editor, -delta, selecting);
    else if (delta > 0)
        editor_down (&editorctl->editor, delta, selecting);
}

static BOOL append (EDITORCTL *editorctl)
{
    char *buffer = NULL;
    int i;

    if ((buffer = (char *)HeapAlloc (editorctl->heap, 0, 100000000)) == NULL) goto error;

    for (i = 0; i < 100000000; i++)
    {
        if (i % 100000 == 0)
            buffer [i] = "\r\n" [rand () % 2];
        else
            buffer [i] = "abcdef \t" [rand () % 8];
    }

    if (!editor_replace_range (&editorctl->editor, editorctl->editor.text.length, 0, 100000000, buffer, editorctl->editor.caret_offset)) goto error;

    HeapFree (editorctl->heap, 0, buffer);

    return TRUE;
error:
    if (buffer != NULL)
        HeapFree (editorctl->heap, 0, buffer);

    return FALSE;
}

LRESULT editorctl_on_keydown (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL control, alt, shift;
    int page, first, last, caret_row;
    RECT r;

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;
    page = (r.bottom - r.top) / editorctl->cell_size.cy;
    page = max (page, 1);

    first = (r.top + editorctl->scroll_location.y + editorctl->cell_size.cy - 1) / editorctl->cell_size.cy;
    last = (r.bottom + editorctl->scroll_location.y) / editorctl->cell_size.cy - 1;

    if (first > last)
    {
        first = (r.top + editorctl->scroll_location.y) / editorctl->cell_size.cy;
        last = (r.bottom + editorctl->scroll_location.y - 1) / editorctl->cell_size.cy;
    }

    caret_row = layout_offset_to_row (&editorctl->editor.layout, editorctl->editor.caret_offset);

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
                editor_left_word (&editorctl->editor, shift);
                break;
            case VK_RIGHT:
                editor_right_word (&editorctl->editor, shift);
                break;
            case VK_UP:
                if (!editorctl_scroll_to (editorctl, editorctl->scroll_location.x, max (0, editorctl->scroll_location.y - editorctl->cell_size.cy))) goto error;
                break;
            case VK_DOWN:
                if (!editorctl_scroll_to (editorctl, editorctl->scroll_location.x, max (0, min (editorctl->editor.layout.row_offsets.length * editorctl->cell_size.cy - r.bottom + r.top, editorctl->scroll_location.y + editorctl->cell_size.cy)))) goto error;
                break;
            case VK_HOME:
                editor_begin_text (&editorctl->editor, shift);
                break;
            case VK_END:
                editor_end_text (&editorctl->editor, shift);
                break;
            case VK_PRIOR:
                vmove (editorctl, first - caret_row, shift);
                break;
            case VK_NEXT:
                vmove (editorctl, last - caret_row, shift);
                break;
            case VK_BACK:
                if (!shift)
                {
                    if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
                        editor_delete_selection (&editorctl->editor);
                    else
                        editor_backspace_word (&editorctl->editor);
                }
                break;
            case VK_DELETE:
                if (!shift)
                {
                    if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
                        editor_delete_selection (&editorctl->editor);
                    else
                        editor_delete_word (&editorctl->editor);
                }
                break;
            case 0x41: // key `A'
                if (!shift) editor_set_selection (&editorctl->editor, 0, editorctl->editor.text.length);
                break;
            }
        }
        else
        {
            switch (wParam)
            {
            case VK_LEFT:
                editor_left (&editorctl->editor, shift);
                break;
            case VK_RIGHT:
                editor_right (&editorctl->editor, shift);
                break;
            case VK_HOME:
                editor_begin_line (&editorctl->editor, shift);
                break;
            case VK_END:
                editor_end_line (&editorctl->editor, shift);
                break;
            case VK_UP:
                editor_up (&editorctl->editor, 1, shift);
                break;
            case VK_DOWN:
                editor_down (&editorctl->editor, 1, shift);
                break;
            case VK_PRIOR:
                editor_up (&editorctl->editor, page, shift);
                break;
            case VK_NEXT:
                editor_down (&editorctl->editor, page, shift);
                break;
            case VK_INSERT:
                if (!shift)
                {
                    if (!toggle_insert (editorctl)) goto error;
                }
                break;
            case VK_BACK:
                if (!shift)
                {
                    if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
                        editor_delete_selection (&editorctl->editor);
                    else
                        editor_backspace (&editorctl->editor);
                }
                break;
            case VK_DELETE:
                if (!shift)
                {
                    if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
                        editor_delete_selection (&editorctl->editor);
                    else
                        editor_delete (&editorctl->editor);
                }
                break;
            case VK_RETURN:
                if (!shift)
                {
                    if (editorctl->editor.anchor_offset != editorctl->editor.caret_offset)
                        editor_replace_selection (&editorctl->editor, editorctl->new_line_length, editorctl->new_line);
                    else
                        editor_insert (&editorctl->editor, editorctl->new_line_length, editorctl->new_line);
                }
                break;
            case VK_F12:
                if (!shift)
                {
                    if (!append (editorctl)) goto error;
                }
                break;
            }
        }
    }

    if (!editorctl_update_scroll_range (editorctl)) goto error;
    if (!editorctl_update (editorctl)) goto error;
    if (!editorctl_update_caret_pos (editorctl, TRUE)) goto error;

    return 0;
error:
    return -1;
}
