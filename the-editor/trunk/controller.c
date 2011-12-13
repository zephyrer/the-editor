#include <windows.h>
#include <assert.h>
#include <malloc.h>
#include <tchar.h>

#include "xalloc.h"
#include "controller.h"
#include "editorctl.h"

static model_coord_t controller_get_first_non_blank_position (controller_t *controller, model_coord_t line)
{
	model_coord_t ll;
	model_coord_t p;

    assert (controller != NULL);
	assert (line >= 0 && line < model_get_line_count (controller->model));

	ll = model_get_line_length (controller->model, line);
	for (p = 0; p < ll; p++)
		if (!_istspace (model_get_char_at (controller->model, line, p)))
			return p;

	return 0;
}

static model_coord_t controller_max_line_length (controller_t *controller, model_coord_t start_line, model_coord_t end_line)
{
    model_coord_t line;
    model_coord_t result = 0;

    assert (controller != NULL);
    assert (start_line >= 0 && start_line <= end_line && end_line < model_get_line_count (controller->model));

    for (line = start_line; line <= end_line; line++)
        result = max (result, model_get_line_length (controller->model, line));

    return result;
}

static void controller_update_max_line_length (controller_t *controller)
{
    assert (controller != NULL);

    controller->max_line_length = controller_max_line_length (controller, 0, model_get_line_count (controller->model) - 1);
}

static BOOL controller_is_word_boundary (controller_t *controller, model_coord_t line, model_coord_t position)
{
    model_char_t ch1, ch2;

    assert (controller != NULL);

    if (position == 0) return TRUE;

    if (position == model_get_line_length (controller->model, line))
        return TRUE;

    ch1 = model_get_char_at (controller->model, line, position - 1);
    ch2 = model_get_char_at (controller->model, line, position);

    return _istalnum (ch1) ^ _istalnum (ch2);
}

static model_coord_t controller_prev_word_boundary (controller_t *controller, model_coord_t line, model_coord_t position)
{
    model_coord_t p;

    assert (controller != NULL);
    assert (line >= 0 && line < model_get_line_count (controller->model));
    assert (position > 0 && position <= model_get_line_length (controller->model, line));

    for (p = position - 1; p >= 0; p--)
        if (controller_is_word_boundary (controller, line, p)) return p;

    return 0;
}

static model_coord_t controller_next_word_boundary (controller_t *controller, model_coord_t line, model_coord_t position)
{
    model_coord_t p;
    model_coord_t ll;

    assert (controller != NULL);
    assert (line >= 0 && line < model_get_line_count (controller->model));
    assert (position >= 0 && position < model_get_line_length (controller->model, line));

    ll = model_get_line_length (controller->model, line);
    for (p = position + 1; p <= ll; p++)
        if (controller_is_word_boundary (controller, line, p)) return p;

    return ll;
}

static void controller_update_view (controller_t *controller)
{
    SIZE ms;
    POINT cp;

    assert (controller != NULL);

    ms.cx = controller->max_line_length + 1;
    ms.cy = model_get_line_count (controller->model);
    SendMessage (controller->view, ECM_SET_MODEL_SIZE, 0, (LPARAM)&ms);

    cp.x = controller->cursor_position;
    cp.y = controller->cursor_line;
    SendMessage (controller->view, ECM_SET_CARET_POSITION, 0, (LPARAM)&cp);

    SendMessage (controller->view, ECM_SET_CARET_SHAPE, ECCS_INSERT, 0);
}

static void controller_ensure_cursor_visible (controller_t *controller)
{
	RECT r;
	POINT p;
	SIZE s;
	RECT cr;
	LONG w, h;

    assert (controller != NULL);

	GetClientRect (controller->view, &r);
	SendMessage (controller->view, ECM_GET_SCROLL_POSITION, 0, (LPARAM)&p);
	SendMessage (controller->view, ECM_GET_CELL_SIZE, 0, (LPARAM)&s);

	w = r.right - r.left;
	h = r.bottom - r.top;

	cr.left = controller->cursor_position * s.cx;
	cr.right = cr.left + s.cx;
	cr.top = controller->cursor_line * s.cy;
	cr.bottom = cr.top + s.cy;

	if (cr.right > w + p.x)
		p.x = cr.right - w;

	if (cr.left < p.x)
		p.x = cr.left;

	if (cr.bottom > h + p.y)
		p.y = cr.bottom - h;

	if (cr.top < p.y)
		p.y = cr.top;

	SendMessage (controller->view, ECM_SET_SCROLL_POSITION, 0, (LPARAM)&p);
}

static void controller_move_cursor (controller_t *controller, model_coord_t line, model_coord_t position, BOOL selecting)
{
    assert (controller != NULL);

    if (controller->cursor_line == line && controller->cursor_position == position)
        return;

    if (!selecting)
    {
        controller->anchor_line = line;
        controller->anchor_position = position;
    }

    controller->cursor_line = line;
    controller->cursor_position = position;
	controller->cursor_column = position;

	controller_ensure_cursor_visible (controller);

    controller_update_view (controller);

    InvalidateRect (controller->view, NULL, FALSE);
}

static void controller_replace_range (controller_t *controller, 
    model_coord_t start_line, model_coord_t start_position, model_coord_t end_line, model_coord_t end_position, 
    model_coord_t replacement_line_count, const model_coord_t replacement_line_lengths [], const model_char_t * replacement_lines [],
    model_coord_t new_cursor_line, model_coord_t new_cursor_position)
{
    model_coord_t old_max_ll, new_max_ll;

    assert (controller != NULL);

    old_max_ll = controller_max_line_length (controller, start_line, end_line);

    model_free_change (
        model_replace_range (
            controller->model, 
            start_line, start_position, end_line, end_position, 
            replacement_line_count, replacement_line_lengths, replacement_lines));

    new_max_ll = controller_max_line_length (controller, start_line, start_line + replacement_line_count - 1);

    if (controller->max_line_length <= new_max_ll)
        controller->max_line_length = new_max_ll;
    else if (controller->max_line_length == old_max_ll)
        controller_update_max_line_length (controller);

	controller_move_cursor (controller, new_cursor_line, new_cursor_position, FALSE);

    controller_update_view (controller);

    InvalidateRect (controller->view, NULL, FALSE);
}

static int controller_get_selection_direction (controller_t * controller)
{
    if (controller->anchor_line < controller->cursor_line ||
        (controller->anchor_line == controller->cursor_line && controller->anchor_position < controller->cursor_position))
        return 1;
    else if (controller->anchor_line > controller->cursor_line ||
        (controller->anchor_line == controller->cursor_line && controller->anchor_position > controller->cursor_position))
        return -1;
    else return 0;
}

static BOOL controller_is_selected (controller_t * controller, model_coord_t line, model_coord_t position)
{
    if (controller_get_selection_direction (controller) > 0)
    {
        if (line < controller->anchor_line || (line == controller->anchor_line && position < controller->anchor_position)) return FALSE;
        else if (line > controller->cursor_line || (line == controller->cursor_line && position >= controller->cursor_position)) return FALSE;
    }
    else
    {
        if (line < controller->cursor_line || (line == controller->cursor_line && position < controller->cursor_position)) return FALSE;
        else if (line > controller->anchor_line || (line == controller->anchor_line && position >= controller->anchor_position)) return FALSE;
    }

    return TRUE;
}

static void controller_delete_selection (controller_t *controller)
{
    model_coord_t l = 0;
    model_char_t *p = NULL;

    if (controller_get_selection_direction (controller) > 0)
    {
        controller_replace_range (controller, controller->anchor_line, controller->anchor_position, controller->cursor_line, controller->cursor_position, 1, &l, &p, controller->anchor_line, controller->anchor_position);
    }
    else
    {
        controller_replace_range (controller, controller->cursor_line, controller->cursor_position, controller->anchor_line, controller->anchor_position, 1, &l, &p, controller->cursor_line, controller->cursor_position);
    }
}

static void controller_vmove (controller_t *controller, model_coord_t delta, BOOL selecting)
{
	model_coord_t new_line, new_column;
	model_coord_t lc;

    assert (controller != NULL);

	new_line = controller->cursor_line + delta;
	new_column = controller->cursor_column;
	lc = model_get_line_count (controller->model);

	if(new_line >= lc)
	{
		new_line = lc - 1;

		if (controller->cursor_line == new_line)
			new_column = model_get_line_length (controller->model, new_line);
	}


	if (new_line < 0)
	{
		new_line = 0;

		if (controller->cursor_line == new_line)
			new_column = 0;
	}

    controller_move_cursor (controller, new_line, min (new_column, model_get_line_length (controller->model, new_line)), selecting);
	controller->cursor_column = new_column;
}

static model_coord_t controller_get_page_height (controller_t *controller)
{
	SIZE s = {1, 1};
	RECT r;
	model_coord_t result;

	GetClientRect (controller->view, &r);
	SendMessage (controller->view, ECM_GET_CELL_SIZE, 0, (LPARAM)&s);

	result = (r.bottom - r.top) / s.cy;
	return max (result, 1);
}

extern controller_t *controller_create (HWND view)
{
    controller_t *controller = (controller_t *)xalloc (sizeof (controller_t));

    controller->view = view;
    controller->model = model_create ();
    controller->max_line_length = 0;
    controller->anchor_line = 0;
    controller->anchor_position = 0;
    controller->cursor_line = 0;
    controller->cursor_position = 0;
	controller->cursor_column = 0;

    controller_update_view (controller);

    return controller;
}

extern void controller_free (controller_t *controller)
{
    assert (controller != NULL);

    model_free (controller->model);
    xfree (controller);
}

extern void controller_request_data (controller_t *controller, int start_row, int start_column, int end_row, int end_column, editorctl_unicode_char_t buffer [], editorctl_style_t styles [])
{
    model_coord_t line;
    int width = end_column - start_column;
    editorctl_unicode_char_t *bp = buffer;
    editorctl_style_t *sp = styles;
    model_char_t *chars = (model_char_t *)_malloca (width * sizeof (model_char_t));

    assert (controller != NULL);

    for (line = start_row; line < end_row; line++)
    {
        model_coord_t line_length = model_get_line_length (controller->model, line);

        int i = 0;

        if (start_column < line_length)
        {
            int count = min (line_length, end_column) - start_column;

            model_get_chars_range (controller->model, line, start_column, start_column + count, chars);
            for (; i < count; i++)
            {
				if (chars [i] != ' ')
				{
					*bp++ = chars [i];
					*sp++ = controller_is_selected (controller, line, start_column + i) ? ECS_SELECTED : ECS_NORMAL;
				}
				else
				{
					*bp++ = 0xB7;
					*sp++ = controller_is_selected (controller, line, start_column + i) ? ECS_SELECTED_WHITESPACE : ECS_WHITESPACE;
				}
            }
		}

        for (; i < width; i++)
        {
			if (i + start_column != line_length)
			{
				*bp++ = '\0';
		        *sp++ = ECS_NONE;
			}
			else
			{
				*bp++ = 0xB6;
				*sp++ = controller_is_selected (controller, line, line_length) ? ECS_SELECTED_WHITESPACE : ECS_WHITESPACE;
			}
        }
    }

    _freea (chars);
}

extern void controller_char (controller_t *controller, editorctl_unicode_char_t ch)
{
    model_coord_t l = 1;
    model_char_t c = ch;
    model_char_t * pc = &c;

    assert (controller != NULL);

    if (controller_get_selection_direction (controller) > 0)
        controller_replace_range (
            controller, 
            controller->anchor_line, controller->anchor_position, controller->cursor_line, controller->cursor_position, 
            1, &l, &pc,
            controller->anchor_line, controller->anchor_position + 1);
    else
        controller_replace_range (
            controller, 
            controller->cursor_line, controller->cursor_position, controller->anchor_line, controller->anchor_position, 
            1, &l, &pc,
            controller->cursor_line, controller->cursor_position + 1);
}

extern void controller_new_line (controller_t *controller)
{
    model_coord_t ll [] = {0, 0};
    model_char_t * lp [] = {NULL, NULL};

    assert (controller != NULL);

    if (controller_get_selection_direction (controller) > 0)
        controller_replace_range (
            controller, 
            controller->anchor_line, controller->anchor_position, controller->cursor_line, controller->cursor_position, 
            2, ll, lp,
            controller->anchor_line + 1, 0);
    else
        controller_replace_range (
            controller, 
            controller->cursor_line, controller->cursor_position, controller->anchor_line, controller->anchor_position, 
            2, ll, lp,
            controller->cursor_line + 1, 0);
}

extern void controller_backspace (controller_t *controller, BOOL word)
{
    model_coord_t l = 0;
    model_char_t * pc = NULL;
    
    assert (controller != NULL);

    if (controller_get_selection_direction (controller) == 0)
    {
        if (controller->cursor_position > 0)
        {
            model_coord_t new_position = word ? controller_prev_word_boundary (controller, controller->cursor_line, controller->cursor_position) : controller->cursor_position - 1;

            controller_replace_range (
                controller, 
                controller->cursor_line, new_position, controller->cursor_line, controller->cursor_position, 
                1, &l, &pc, controller->cursor_line, new_position);
        }
        else if (controller->cursor_line > 0)
        {
            model_coord_t prev_line_length = model_get_line_length (controller->model, controller->cursor_line - 1);

            controller_replace_range (
                controller, 
                controller->cursor_line - 1, prev_line_length, controller->cursor_line, 
                0, 1, &l, &pc, 
                controller->cursor_line - 1,
                prev_line_length);
        }
    }
    else controller_delete_selection (controller);
}

extern void controller_delete (controller_t *controller, BOOL word)
{
    model_coord_t l = 0;
    model_char_t * pc = NULL;
    
    assert (controller != NULL);

    if (controller_get_selection_direction (controller) == 0)
    {
        if (controller->cursor_position < model_get_line_length (controller->model, controller->cursor_line))
        {
            model_coord_t to_position = word ? controller_next_word_boundary (controller, controller->cursor_line, controller->cursor_position) : controller->cursor_position + 1;

            controller_replace_range (
                controller, 
                controller->cursor_line, controller->cursor_position, controller->cursor_line, to_position, 
                1, &l, &pc, controller->cursor_line, controller->cursor_position);
        }
        else if (controller->cursor_line < model_get_line_count (controller->model) - 1)
        {
            controller_replace_range (
                controller, 
                controller->cursor_line, controller->cursor_position, controller->cursor_line + 1, 0,
                1, &l, &pc, 
                controller->cursor_line,
                controller->cursor_position);
        }
    }
    else controller_delete_selection (controller);
}

extern void controller_left (controller_t *controller, BOOL word, BOOL selecting)
{
    assert (controller != NULL);

    if (controller->cursor_position > 0)
    {
        model_coord_t new_position = word ? controller_prev_word_boundary (controller, controller->cursor_line, controller->cursor_position) : controller->cursor_position - 1;

        controller_move_cursor (controller, controller->cursor_line, new_position, selecting);
    }
    else if (controller->cursor_line > 0)
    {
        controller_move_cursor (controller, controller->cursor_line - 1, model_get_line_length (controller->model, controller->cursor_line - 1), selecting);
    }
}

extern void controller_right (controller_t *controller, BOOL word, BOOL selecting)
{
    assert (controller != NULL);

    if (controller->cursor_position < model_get_line_length (controller->model, controller->cursor_line))
    {
        model_coord_t new_position = word ? controller_next_word_boundary (controller, controller->cursor_line, controller->cursor_position) : controller->cursor_position + 1;

        controller_move_cursor (controller, controller->cursor_line, new_position, selecting);
    }
    else if (controller->cursor_line < model_get_line_count (controller->model) - 1)
    {
        controller_move_cursor (controller, controller->cursor_line + 1, 0, selecting);
    }
}

extern void controller_word_left (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

    if (controller->cursor_position > 0)
    {
        controller_move_cursor (controller, controller->cursor_line, controller_prev_word_boundary (controller, controller->cursor_line, controller->cursor_position), selecting);
    }
    else if (controller->cursor_line > 0)
    {
        controller_move_cursor (controller, controller->cursor_line - 1, model_get_line_length (controller->model, controller->cursor_line - 1), selecting);
    }
}

extern void controller_word_right (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

    if (controller->cursor_position < model_get_line_length (controller->model, controller->cursor_line))
    {
        controller_move_cursor (controller, controller->cursor_line, controller_next_word_boundary (controller, controller->cursor_line, controller->cursor_position), selecting);
    }
    else if (controller->cursor_line < model_get_line_count (controller->model) - 1)
    {
        controller_move_cursor (controller, controller->cursor_line + 1, 0, selecting);
    }
}

extern void controller_up (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_vmove (controller, -1, selecting);
}

extern void controller_down (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_vmove (controller, +1, selecting);
}

extern void controller_home (controller_t *controller, BOOL selecting)
{
	model_coord_t fnbp;

    assert (controller != NULL);

	fnbp = controller_get_first_non_blank_position (controller, controller->cursor_line);

	if (controller->cursor_position == fnbp)
	{
		controller_move_cursor (controller, controller->cursor_line, 0, selecting);
	}
	else
	{
		controller_move_cursor (controller, controller->cursor_line, fnbp, selecting);
	}
}

extern void controller_end (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_move_cursor (controller, controller->cursor_line, model_get_line_length (controller->model, controller->cursor_line), selecting);
}

extern void controller_page_up (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_vmove (controller, -controller_get_page_height (controller), selecting);
}

extern void controller_page_down (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_vmove (controller, +controller_get_page_height (controller), selecting);
}

extern void controller_document_home (controller_t *controller, BOOL selecting)
{
    assert (controller != NULL);

	controller_move_cursor (controller, 0, 0, selecting);
}

extern void controller_document_end (controller_t *controller, BOOL selecting)
{
	model_coord_t lc;

    assert (controller != NULL);

	lc = model_get_line_count (controller->model);

	controller_move_cursor (controller, lc - 1, model_get_line_length (controller->model, lc - 1), selecting);
}

extern void controller_page_home (controller_t *controller, BOOL selecting)
{
	RECT r;
	POINT p = {0, 0};
	SIZE s = {1, 1};
	model_coord_t l1, l2, l;

    assert (controller != NULL);

	GetClientRect (controller->view, &r);
	SendMessage (controller->view, ECM_GET_SCROLL_POSITION, 0, (LPARAM)&p);
	SendMessage (controller->view, ECM_GET_CELL_SIZE, 0, (LPARAM)&s);

	l1 = (r.top + p.y + s.cy - 1) / s.cy;
	l2 = (r.bottom + p.y - s.cy + 1) / s.cy;

	if (l2 >= l1)
		l = l1;
	else
		l = (r.top + p.y) / s.cy;

	controller_vmove (controller, l - controller->cursor_line, selecting);
}

extern void controller_page_end (controller_t *controller, BOOL selecting)
{
	RECT r;
	POINT p = {0, 0};
	SIZE s = {1, 1};
	model_coord_t l1, l2, l;

    assert (controller != NULL);

	GetClientRect (controller->view, &r);
	SendMessage (controller->view, ECM_GET_SCROLL_POSITION, 0, (LPARAM)&p);
	SendMessage (controller->view, ECM_GET_CELL_SIZE, 0, (LPARAM)&s);

	l1 = (r.top + p.y + s.cy - 1) / s.cy;
	l2 = (r.bottom + p.y - s.cy + 1) / s.cy;

	if (l2 >= l1)
		l = l2;
	else 
		l = (r.bottom + p.y - 1) / s.cy;

	controller_vmove (controller, l - controller->cursor_line, selecting);
}

extern void controller_scroll_up (controller_t *controller)
{
    SendMessage (controller->view, WM_VSCROLL, SB_LINEUP, 0);
}

extern void controller_scroll_down (controller_t *controller)
{
    SendMessage (controller->view, WM_VSCROLL, SB_LINEDOWN, 0);
}
