#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_prev_char (EDITORCTL_TEXT_ITERATOR *iterator)
{
    iterator->offset--;
    return *--iterator->ptr;
}
