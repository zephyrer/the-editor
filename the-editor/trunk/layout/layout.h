#ifndef LAYOUT_H
#define LAYOUT_H

#include <windows.h>

#include "../intlist/intlist.h"
#include "../text/text.h"

typedef struct tagLAYOUT
{
    TEXT *text;
    int tab_width;
    int wrap;
    int min_wrap;

    int max_row_width;

    int first_dirty_row;
    int last_dirty_row;

    HANDLE heap;
    INTLIST row_offsets;
    INTLIST row_widths;
    INTLIST line_rows;
} LAYOUT;

BOOL layout_initialize (LAYOUT *layout, HANDLE heap, TEXT *text, int tab_width, int wrap, int min_wrap);
BOOL layout_destroy (LAYOUT *layout);

BOOL layout_update (LAYOUT *layout, int offset, int old_length, int new_length);

int layout_offset_to_row (LAYOUT *layout, int offset);
int layout_offset_to_column (LAYOUT *layout, int offset, int row);
int layout_offset_to_line (LAYOUT *layout, int offset);
int layout_line_to_offset (LAYOUT *layout, int line);
int layout_row_column_to_offset (LAYOUT *layout, int row, int column, int *real_column);

int layout_line_count (LAYOUT *layout);

void layout_add_dirty_range (LAYOUT *layout, int first_dirty_row, int last_dirty_row);
void layout_reset_dirty_rows (LAYOUT *layout);

#endif
