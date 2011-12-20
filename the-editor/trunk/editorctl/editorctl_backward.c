#include <assert.h>

#include "editorctl.h"

BOOL editorctl_backward (EDITORCTL_TEXT_ITERATOR *iterator)
{
    assert (iterator->offset >= 0);

    if (iterator->offset == 0) return FALSE;

    iterator->offset--;
    iterator->ptr--;

    iterator->is_begin = (iterator->offset == 0);
    iterator->is_end = FALSE;

    iterator->next_char = iterator->prev_char;
    iterator->prev_char = (iterator->offset == 0) ? 0 : *(iterator->ptr - 1);

    return TRUE;
}
