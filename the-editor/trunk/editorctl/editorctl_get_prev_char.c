#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_prev_char (EDITORCTL_TEXT_ITERATOR *iterator)
{
    return iterator->offset > 0 ? iterator->text [iterator->offset - 1] : 0;
}
