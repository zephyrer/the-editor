#include <assert.h>

#include "editorctl.h"

BOOL editorctl_forward (EDITORCTL_TEXT_ITERATOR *iterator)
{
    assert (iterator->offset <= iterator->text_length);

    if (iterator->offset == iterator->text_length) return FALSE;

    iterator->offset++;
    iterator->ptr++;

    iterator->is_begin = FALSE;
    iterator->is_end = (iterator->offset == iterator->text_length);

    iterator->prev_char = iterator->next_char;
    iterator->next_char = (iterator->offset == iterator->text_length) ? 0 : *iterator->ptr;

    return TRUE;
}
