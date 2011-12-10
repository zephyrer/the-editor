#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <crtdbg.h>

#include <stdio.h>

#include "xalloc.h"
#include "model.h"

#define MIN_CAPACITY 16

static void * ensure_capacity (void *memblock, size_t capacity)
{
    assert (capacity >= 0);

    if (memblock != NULL && capacity <= _msize (memblock)) 
        return memblock;
    else 
    {
        size_t new_capacity = capacity * 3 / 2;

        if (new_capacity < MIN_CAPACITY) 
            new_capacity = MIN_CAPACITY;

        return xrealloc (memblock, new_capacity);
    }
}

static model_coord_t sum (model_coord_t count, const model_coord_t values [])
{
    model_coord_t result = 0;
    model_coord_t i;
    for (i = 0; i < count; i++)
        result += values [i];

    return result;
}

static struct model_change_simple_8_bit * model_replace_range_simple_8_bit_no_data (
    struct model_simple_8_bit *model, 
    model_coord_t start_line, model_coord_t start_position, model_coord_t end_line, model_coord_t end_position, 
    model_coord_t replacement_line_count, const model_coord_t replacement_line_lengths [])
{
    size_t start_offset = model->line_offsets [start_line] + start_position;
    size_t end_offset = model->line_offsets [end_line] + end_position;
    size_t original_length = end_offset - start_offset;
    size_t replacement_length = sum (replacement_line_count, replacement_line_lengths) + replacement_line_count - 1;
    size_t new_end_offset = start_offset + replacement_length;
    model_coord_t original_line_count = end_line - start_line + 1;
    model_coord_t end_line_remaining = model->line_lengths [end_line] - end_position;
    model_coord_t new_end_line = start_line + replacement_line_count - 1;
    model_coord_t new_end_position = replacement_line_count > 1 ? replacement_line_lengths [replacement_line_count - 1] : start_position + replacement_line_lengths [0];

    model_coord_t i;
    size_t offset;

    struct model_change_simple_8_bit *change = (struct model_change_simple_8_bit *)xalloc (
        sizeof (struct model_change_simple_8_bit) + 
        (original_line_count) * sizeof (model_coord_t) + 
        (original_length) * sizeof (unsigned char));

    change->type = MT_SIMPLE_8_BIT;
    change->model = model;

    change->start_line = start_line;
    change->start_position = start_position;
    change->end_line = new_end_line;
    change->end_position = new_end_position;

    change->original_lines_count = original_line_count;
    if (original_line_count == 1)
        change->original_line_lengths [0] = end_position - start_position;
    else
    {
        change->original_line_lengths [0] = model->line_lengths [start_line] - start_position;
        for (i = 1; i < original_line_count - 1; i++)
            change->original_line_lengths [i] = model->line_lengths [start_line + i];
        change->original_line_lengths [original_line_count - 1] = end_position;
    }

    memcpy (change->original_line_lengths + original_line_count, model->buffer + start_offset, original_length * sizeof (unsigned char));

    model->line_count = model->line_count - original_line_count + replacement_line_count ;
    model->buffer_size = model->buffer_size - original_length + replacement_length;

    model->line_lengths = (model_coord_t *)ensure_capacity (model->line_lengths, model->line_count * sizeof (model_coord_t));
    memcpy (model->line_lengths + new_end_line + 1, model->line_lengths + end_line + 1, (model->line_count - new_end_line - 1) * sizeof (model_coord_t));

    if (new_end_line == start_line)
        model->line_lengths [start_line] = start_position + replacement_line_lengths [0] + end_line_remaining;
    else
    {
        model->line_lengths [start_line] = start_position + replacement_line_lengths [0];
        model->line_lengths [new_end_line] = replacement_line_lengths [replacement_line_count - 1] + end_line_remaining;

        for (i = 1; i < replacement_line_count - 1; i++)
            model->line_lengths [start_line + i] = replacement_line_lengths [i];
    }

    model->line_offsets = (size_t *)ensure_capacity (model->line_offsets, model->line_count * sizeof (size_t));
    offset = model->line_offsets [start_line];
    for (i = start_line + 1; i < model->line_count; i++)
    {
        offset += model->line_lengths [i - 1] + 1;
        model->line_offsets [i] = offset;
    }

    model->buffer = (unsigned char *)ensure_capacity (model->buffer, model->buffer_size * sizeof (unsigned char));
    memcpy (model->buffer + new_end_offset, model->buffer + end_offset, (model->buffer_size - new_end_offset) * sizeof (unsigned char));

    return change;
}

static struct model_change_simple_8_bit * model_replace_range_simple_8_bit (
    struct model_simple_8_bit *model, 
    model_coord_t start_line, model_coord_t start_position, model_coord_t end_line, model_coord_t end_position, 
    model_coord_t replacement_line_count, const model_coord_t replacement_line_lengths [], const model_char_t * replacement_lines [])
{
    struct model_change_simple_8_bit *change = model_replace_range_simple_8_bit_no_data (model, start_line, start_position, end_line, end_position, replacement_line_count, replacement_line_lengths);
    size_t start_offset = model->line_offsets [start_line] + start_position;
    model_coord_t i;
    unsigned char *ptr;

    ptr = model->buffer + start_offset;
    for (i = 0; i < replacement_line_count; i++)
    {
        model_coord_t j;
        model_coord_t l = replacement_line_lengths [i];

        if (i > 0)
            *ptr++ = '\n';

        for (j = 0; j < l; j++)
            *ptr++ = (unsigned char)replacement_lines [i][j];
    }

    return change;
}

static struct model_change_simple_8_bit * model_undo_change_simple_8_bit (struct model_change_simple_8_bit *change)
{
    struct model_change_simple_8_bit *new_change = model_replace_range_simple_8_bit_no_data (change->model, change->start_line, change->start_position, change->end_line, change->end_position, change->original_lines_count, change->original_line_lengths);
    size_t start_offset = change->model->line_offsets [change->start_line] + change->start_position;
    model_coord_t i;
    size_t original_length = 0;

    for (i = 0; i < change->original_lines_count; i++)
    {
        if (i > 0)
            original_length++;

        original_length += change->original_line_lengths [i];
    }

    memcpy (change->model->buffer + start_offset, change->original_line_lengths + change->original_lines_count, original_length * sizeof (unsigned char));

    return new_change;
}

model_t * model_create ()
{
    model_t * result = (model_t *)xalloc (sizeof (struct model_simple_8_bit));
    if (result == NULL)
        return NULL;

    result->simple_8_bit.type = MT_SIMPLE_8_BIT;
    result->simple_8_bit.line_count = 1;
    result->simple_8_bit.line_offsets = (size_t *)ensure_capacity (NULL, 1 * sizeof (size_t));
    result->simple_8_bit.line_offsets [0] = 0;
    result->simple_8_bit.line_lengths = (model_coord_t *)ensure_capacity (NULL, 1 * sizeof (model_coord_t));
    result->simple_8_bit.line_lengths [0] = 0;
    result->simple_8_bit.buffer_size = 0;
    result->simple_8_bit.buffer = (unsigned char *)ensure_capacity (NULL, 0 * sizeof (unsigned char));

    return result;
}

void model_free (model_t *model)
{
    enum model_type_t type;
    
    assert (model != NULL);
    assert (model->type != MT_NONE);

    type = model->type;
    model->type = MT_NONE;

    switch (type)
    {
    case MT_SIMPLE_8_BIT:
        xfree (model->simple_8_bit.line_offsets);
        xfree (model->simple_8_bit.line_lengths);
        xfree (model->simple_8_bit.buffer);
        break;
    default:
        assert (0);
    }

    xfree (model);
}

model_coord_t model_get_line_count (model_t *model)
{
    assert (model != NULL);
    assert (model->type != MT_NONE);

    switch (model->type)
    {
    case MT_SIMPLE_8_BIT:
        return model->simple_8_bit.line_count;
    default:
        assert (0);
        return -1;
    }
}

model_coord_t model_get_line_length (model_t *model, model_coord_t line)
{
    assert (model != NULL);
    assert (model->type != MT_NONE);

    assert (line >= 0 && line < model_get_line_count (model));

    switch (model->type)
    {
    case MT_SIMPLE_8_BIT:
        return model->simple_8_bit.line_lengths [line];
    default:
        assert (0);
        return -1;
    }
}

model_char_t model_get_char_at (model_t *model, model_coord_t line, model_coord_t position)
{
    assert (model != NULL);
    assert (model->type != MT_NONE);

    assert (line >= 0 && line < model_get_line_count (model));
    assert (position >= 0 && position < model_get_line_length (model, line));

    switch (model->type)
    {
    case MT_SIMPLE_8_BIT:
        return model->simple_8_bit.buffer [model->simple_8_bit.line_offsets [line] + position];
    default:
        assert (0);
        return -1;
    }
}

void model_get_chars_range (model_t *model, model_coord_t line, model_coord_t start_position, model_coord_t end_position, model_char_t buffer [])
{
    assert (model != NULL);
    assert (model->type != MT_NONE);

    assert (line >= 0 && line < model_get_line_count (model));
    assert (start_position >= 0 && end_position >= start_position && end_position <= model_get_line_length (model, line));

    assert (buffer != NULL);

    switch (model->type)
    {
        unsigned char *ptr;
        model_coord_t count, i;

    case MT_SIMPLE_8_BIT:
        ptr = model->simple_8_bit.buffer + model->simple_8_bit.line_offsets [line] + start_position;
        for (count = end_position - start_position, i = 0; i < count; i++)
            buffer [i] = (model_char_t)ptr [i];
        break;
    default:
        assert (0);
    }
}

model_change_t * model_replace_range (
    model_t *model, 
    model_coord_t start_line, model_coord_t start_position, model_coord_t end_line, model_coord_t end_position, 
    model_coord_t replacement_line_count, const model_coord_t replacement_line_lengths [], const model_char_t * replacement_lines [])
{
    assert (model != NULL);
    assert (model->type != MT_NONE);

    assert (start_line >= 0);
    assert (end_line >= start_line && end_line < model_get_line_count (model));
    assert (start_position >= 0 && start_position <= model_get_line_length (model, start_line));
    assert ((end_line > start_line || end_position >= start_position) && end_position <= model_get_line_length (model, end_line));

    assert (replacement_line_count > 0);
    assert (replacement_line_lengths != NULL);
    assert (replacement_lines != NULL);

    switch (model->type)
    {
    case MT_SIMPLE_8_BIT:
        return (model_change_t *)model_replace_range_simple_8_bit (&model->simple_8_bit, start_line, start_position, end_line, end_position, replacement_line_count, replacement_line_lengths, replacement_lines);
    default:
        assert (0);
    }

    return NULL;
}

model_change_t * model_undo_change (model_change_t *change)
{
    assert (change != NULL);
    assert (change->type != MT_NONE);

    switch (change->type)
    {
    case MT_SIMPLE_8_BIT:
        return (model_change_t *)model_undo_change_simple_8_bit (&change->simple_8_bit);
    default:
        assert (0);
    }

    return NULL;
}

void model_free_change (model_change_t *change)
{
    assert (change != NULL);
    assert (change->type != MT_NONE);

    change->type = MT_NONE;
    xfree (change);
}

void model_print (model_t *model)
{
    int c = model_get_line_count (model);
    int i;

    for (i = 0; i < c; i++)
    {
        int l = model_get_line_length (model, i);
        int j;

        for (j = 0; j < l; j++)
            putchar (model_get_char_at (model, i, j));

        putchar ('\n');
    }
}

extern int main (int argc, const char * argv [])
{
    model_t * model = model_create ();

    model_coord_t replacement_line_lengths [] = {3, 2, 3};

    model_char_t line1 [] = {'f', 'o', 'o'};
    model_char_t line2 [] = {'a', 'b'};
    model_char_t line3 [] = {'b', 'a', 'r'};

    model_char_t *replacement_lines [] = {line1, line2, line3};

    model_change_t *ch1, *ch2, *ch3, *ch4, *ch5, *ch6;

    ch1 = model_replace_range (model, 0, 0, 0, 0, 3, replacement_line_lengths, replacement_lines);
    model_print (model);

    printf ("===\n");

    ch2 = model_replace_range (model, 1, 1, 1, 1, 3, replacement_line_lengths, replacement_lines);
    model_print (model);

    printf ("===\n");

    ch3 = model_undo_change (ch2);
    model_print (model);

    printf ("===\n");

    ch4 = model_undo_change (ch3);
    model_print (model);

    printf ("===\n");

    ch5 = model_undo_change (ch4);
    model_print (model);

    printf ("===\n");

    ch6 = model_undo_change (ch3);
    model_print (model);

    fgetc (stdin);

    model_free_change (ch1);
    model_free_change (ch2);
    model_free_change (ch3);
    model_free_change (ch4);
    model_free_change (ch5);
    model_free_change (ch6);
    model_free (model);

    _CrtDumpMemoryLeaks ();

    return 0;
}
