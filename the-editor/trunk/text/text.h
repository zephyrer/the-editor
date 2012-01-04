#ifndef TEXT_H
#define TEXT_H

#include <windows.h>

typedef struct tagTEXT
{
    int length;
    char *data;

    HANDLE heap;
    int capacity;
} TEXT;

BOOL text_initialize (TEXT *text, HANDLE heap, int initial_capacity);
BOOL text_destroy (TEXT *text);
BOOL text_replace_range (TEXT *text, int offset, int length, int replacement_length, const char replacement []);

BOOL text_prev_offset (TEXT *text, int *offset);
BOOL text_next_offset (TEXT *text, int *offset);

BOOL text_prev_position (TEXT *text, int *offset);
BOOL text_next_position (TEXT *text, int *offset);

BOOL text_move_to_column (TEXT *text, int *offset, int *column, int to_column, int tab_width);
void text_move_to_offset (TEXT *text, int offset, int *column, int to_offset, int tab_width);

BOOL text_is_line_boundary (TEXT *text, int offset);

BOOL text_next_line (TEXT *text, int *offset);
BOOL text_next_row (TEXT *text, int *offset, int *col, int tab_width);
BOOL text_next_row_wrap (TEXT *text, int *offset, int *col, int tab_width, int wrap, int min_wrap, BOOL *new_line);

#endif
