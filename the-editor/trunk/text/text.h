#ifndef TEXT_H
#define TEXT_H

#include <windows.h>

#define TEXT_NONE (-1)
#define TEXT_WRAP (-2)
#define TEXT_TAB_BEGIN (-3)
#define TEXT_TAB_MIDDLE (-4)
#define TEXT_TAB_END (-5)
#define TEXT_TAB (-6)

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

BOOL text_prev_word_boundary (TEXT *text, int *offset);
BOOL text_next_word_boundary (TEXT *text, int *offset);

BOOL text_next_nonspace (TEXT *text, int *offset);

void text_move_to_column (TEXT *text, int *offset, int *column, int to_column, int tab_width);
void text_move_to_offset (TEXT *text, int offset, int *column, int to_offset, int tab_width);

BOOL text_next_line (TEXT *text, int *offset);
BOOL text_next_row (TEXT *text, int *offset, int *col, int tab_width);
BOOL text_next_row_wrap (TEXT *text, int *offset, int *col, int tab_width, int wrap, int min_wrap, BOOL *new_line);

BOOL text_is_word_boundary (TEXT *text, int offset);
BOOL text_is_line_boundary (TEXT *text, int offset);

void text_render_row (TEXT *text, int offset, int length, int start_column, int column, int buffer_length, int *char_buffer, int *offset_buffer, int tab_width);

#endif
