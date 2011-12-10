#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <windows.h>

#include "editorctl.h"
#include "model.h"

struct controller
{
    HWND view;

    model_t *model;

    model_coord_t max_line_length;

    model_coord_t anchor_line;
    model_coord_t anchor_position;

    model_coord_t cursor_line;
    model_coord_t cursor_position;
	model_coord_t cursor_column;
};

typedef struct controller controller_t;

extern controller_t *controller_create (HWND view);

extern void controller_free (controller_t *controller);

extern void controller_request_data (controller_t *controller, int start_row, int start_column, int end_row, int end_column, editorctl_unicode_char_t buffer [], editorctl_style_t styles []);

extern void controller_char (controller_t *controller, editorctl_unicode_char_t ch);
extern void controller_new_line (controller_t *controller);
extern void controller_backspace (controller_t *controller, BOOL word);
extern void controller_delete (controller_t *controller, BOOL word);

extern void controller_left (controller_t *controller, BOOL word, BOOL selecting);
extern void controller_right (controller_t *controller, BOOL word, BOOL selecting);
extern void controller_up (controller_t *controller, BOOL selecting);
extern void controller_down (controller_t *controller, BOOL selecting);
extern void controller_home (controller_t *controller, BOOL selecting);
extern void controller_end (controller_t *controller, BOOL selecting);
extern void controller_page_up (controller_t *controller, BOOL selecting);
extern void controller_page_down (controller_t *controller, BOOL selecting);
extern void controller_document_home (controller_t *controller, BOOL selecting);
extern void controller_document_end (controller_t *controller, BOOL selecting);
extern void controller_page_home (controller_t *controller, BOOL selecting);
extern void controller_page_end (controller_t *controller, BOOL selecting);
extern void controller_scroll_up (controller_t *controller);
extern void controller_scroll_down (controller_t *controller);

#endif
