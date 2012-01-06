#ifndef EDITOR_H
#define EDITOR_H

#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"

typedef struct tagEDITOR
{
    int anchor_offset;
    int caret_offset;
    int caret_column;

    HANDLE heap;
    TEXT text;
    LAYOUT layout;
} EDITOR;

BOOL editor_initialize (EDITOR *editor, HANDLE heap);
BOOL editor_destroy (EDITOR *editor);

void editor_left (EDITOR *editor, BOOL selecting);
void editor_right (EDITOR *editor, BOOL selecting);

void editor_left_word (EDITOR *editor, BOOL selecting);
void editor_right_word (EDITOR *editor, BOOL selecting);

void editor_begin_line (EDITOR *editor, BOOL selecting);
void editor_end_line (EDITOR *editor, BOOL selecting);

void editor_begin_text (EDITOR *editor, BOOL selecting);
void editor_end_text (EDITOR *editor, BOOL selecting);

void editor_up (EDITOR *editor, int rows, BOOL selecting);
void editor_down (EDITOR *editor, int rows, BOOL selecting);

void editor_move_caret (EDITOR *editor, int offset, BOOL selecting);
void editor_set_selection (EDITOR *editor, int anchor_offset, int caret_offset);

BOOL editor_can_overwrite (EDITOR *editor);

BOOL editor_replace_selection (EDITOR *editor, int length, const char data []);
BOOL editor_insert (EDITOR *editor, int length, const char data []);
BOOL editor_overwrite (EDITOR *editor, int length, const char data []);

BOOL editor_delete_selection (EDITOR *editor);
BOOL editor_delete (EDITOR *editor);
BOOL editor_backspace (EDITOR *editor);

BOOL editor_delete_word (EDITOR *editor);
BOOL editor_backspace_word (EDITOR *editor);

BOOL editor_replace_range (EDITOR *editor, int offset, int length, int replacement_length, const char replacement [], int caret_offset);

#endif
