#include "editorctl.h"

static int prev_word_boundary (EDITORCTL *editorctl, int offset)
{
    if (!text_is_word_boundary (&editorctl->editor.text, offset))
        text_prev_word_boundary (&editorctl->editor.text, &offset);
    return offset;
}

static int next_word_boundary (EDITORCTL *editorctl, int offset)
{
    if (!text_is_word_boundary (&editorctl->editor.text, offset))
        text_next_word_boundary (&editorctl->editor.text, &offset);
    return offset;
}

static int next_word_boundary2 (EDITORCTL *editorctl, int offset)
{
    int o;
    o = offset;
    text_next_word_boundary (&editorctl->editor.text, &o);
    if (!text_is_line_boundary (&editorctl->editor.text, o))
        return o;
    else
        return offset;
}

static int prev_row_boundary (EDITORCTL *editorctl, int offset)
{
    int row;

    row = layout_offset_to_row (&editorctl->editor.layout, offset);
    return editorctl->editor.layout.row_offsets.data [row];
}

static int next_row_boundary (EDITORCTL *editorctl, int offset)
{
    int row, col;

    row = layout_offset_to_row (&editorctl->editor.layout, offset);
    return layout_row_column_to_offset (&editorctl->editor.layout, row, editorctl->editor.layout.row_widths.data [row], &col);
}

BOOL editorctl_move_mouse_cursor (EDITORCTL *editorctl, int x, int y, BOOL selecting)
{
    int row, col, offset;

    row = (y + editorctl->scroll_location.y) / editorctl->cell_size.cy;
    col = (x + editorctl->scroll_location.x + editorctl->cell_size.cx * 2 / 3) / editorctl->cell_size.cx;

    if (row < 0) row = 0;
    if (col < 0) col = 0;

    if (row >= editorctl->editor.layout.row_offsets.length)
        row = editorctl->editor.layout.row_offsets.length - 1;

    offset = layout_row_column_to_offset (&editorctl->editor.layout, row, col, &col);

    if (!selecting)
        editorctl->mouse_anchor_offset = offset;

    switch (editorctl->mouse_select_mode)
    {
    case EDITORCTL_MSM_CHAR:
        editor_set_selection (&editorctl->editor, editorctl->mouse_anchor_offset, offset);
        break;
    case EDITORCTL_MSM_WORD:
        if (prev_word_boundary (editorctl, editorctl->mouse_anchor_offset) <= offset)
        {
            int o1, o2;

            o1 = next_word_boundary (editorctl, offset);
            o2 = next_word_boundary2 (editorctl, editorctl->mouse_anchor_offset);

            editor_set_selection (&editorctl->editor, prev_word_boundary (editorctl, editorctl->mouse_anchor_offset), max (o1, o2));
        }
        else
        {
            editor_set_selection (&editorctl->editor, next_word_boundary2 (editorctl, editorctl->mouse_anchor_offset), prev_word_boundary (editorctl, offset));
        }
        break;
    case EDITORCTL_MSM_ROW:
        if (prev_row_boundary (editorctl, editorctl->mouse_anchor_offset) <= offset)
        {
            editor_set_selection (&editorctl->editor, prev_row_boundary (editorctl, editorctl->mouse_anchor_offset), next_row_boundary (editorctl, offset));
        }
        else
        {
            editor_set_selection (&editorctl->editor, next_row_boundary (editorctl, editorctl->mouse_anchor_offset), prev_row_boundary (editorctl, offset));
        }
        break;
    }

    if (!editorctl_update (editorctl)) goto error;
    if (!editorctl_update_caret_pos (editorctl)) goto error;

    return TRUE;

error:
    return FALSE;
}
