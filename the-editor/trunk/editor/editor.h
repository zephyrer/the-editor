#ifndef EDITOR_H
#define EDITOR_H

#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"

typedef UINT32 EDITOR_CHAR;

typedef struct tagEDITOR
{
    int anchor_offset;
    int caret_offset;

    HANDLE heap;
    TEXT text;
    LAYOUT layout;
} EDITOR;

BOOL editor_initialize (EDITOR *editor, HANDLE heap);
BOOL editor_destroy (EDITOR *editor);

BOOL editor_left (EDITOR *editor, BOOL selecting);
BOOL editor_right (EDITOR *editor, BOOL selecting);

BOOL editor_left_word (EDITOR *editor, BOOL selecting);
BOOL editor_right_word (EDITOR *editor, BOOL selecting);

BOOL editor_begin_row (EDITOR *editor, BOOL selecting);
BOOL editor_end_row (EDITOR *editor, BOOL selecting);

BOOL editor_begin_text (EDITOR *editor, BOOL selecting);
BOOL editor_end_text (EDITOR *editor, BOOL selecting);

BOOL editor_up (EDITOR *editor, int rows, BOOL selecting);
BOOL editor_down (EDITOR *editor, int rows, BOOL selecting);

BOOL editor_move_caret (EDITOR *editor, int offset, BOOL selecting);
BOOL editor_set_selection (EDITOR *editor, int anchor_offset, int caret_offset, BOOL selecting);

BOOL editor_insert (EDITOR *editor, EDITOR_CHAR ch);
BOOL editor_overwrite (EDITOR *editor, EDITOR_CHAR ch);
BOOL editor_new_line (EDITOR *editor);

BOOL editor_delete (EDITOR *editor);
BOOL editor_backspace (EDITOR *editor);

BOOL editor_delete_word (EDITOR *editor);
BOOL editor_backspace_word (EDITOR *editor);

#endif
