#ifndef MODEL_H
#define MODEL_H

typedef int model_coord_t;
typedef unsigned int model_char_t;

enum model_type_t
{
    MT_NONE = 0,
    MT_SIMPLE_8_BIT
};

struct model_simple_8_bit
{
    enum model_type_t type;

    model_coord_t line_count;
    size_t *line_offsets;
    model_coord_t *line_lengths;

    size_t buffer_size;
    unsigned char *buffer;
};

union model
{
    enum model_type_t type;
    struct model_simple_8_bit simple_8_bit;
};

struct model_change_simple_8_bit
{
    enum model_type_t type;

    struct model_simple_8_bit *model;

    model_coord_t start_line;
    model_coord_t start_position;
    model_coord_t end_line;
    model_coord_t end_position;

    model_coord_t original_lines_count;
    model_coord_t original_line_lengths [0];
};

union model_change
{
    enum model_type_t type;
    struct model_change_simple_8_bit simple_8_bit;
};

typedef union model model_t;
typedef union model_change model_change_t;

extern model_t * model_create ();
extern void model_free (model_t *model);

extern model_coord_t model_get_line_count (model_t *model);
extern model_coord_t model_get_line_length (model_t *model, model_coord_t line);
extern model_char_t model_get_char_at (model_t *model, model_coord_t line, model_coord_t position);
extern void model_get_chars_range (model_t *model, model_coord_t line, model_coord_t start_position, model_coord_t end_position, model_char_t buffer []);

extern model_change_t * model_replace_range (
    model_t *model, 
    model_coord_t start_line, model_coord_t start_position, model_coord_t end_line, model_coord_t end_position, 
    model_coord_t replacement_line_count, const model_coord_t replacement_line_lengths [], const model_char_t * replacement_lines []);

extern model_change_t * model_undo_change (model_change_t * change);
extern void model_free_change (model_change_t * change);

#endif