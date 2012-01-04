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

    HANDLE heap;
    INTLIST row_offsets;
    INTLIST row_widths;
    INTLIST line_rows;
} LAYOUT;

BOOL layout_initialize (LAYOUT *layout, HANDLE heap, TEXT *text, int tab_width, int wrap, int min_wrap);
BOOL layout_destroy (LAYOUT *layout);

BOOL layout_update (LAYOUT *layout, int offset, int old_length, int new_length, int *first_dirty_row, int *dirty_row_count);

int layout_offset_to_row (LAYOUT *layout, int offset);
int layout_offset_to_line (LAYOUT *layout, int offset);

#endif
