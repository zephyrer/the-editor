#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_next_char (EDITORCTL_TEXT_ITERATOR *iterator)
{
    return iterator->offset < iterator->text_length ? iterator->text [iterator->offset] : 0;
}
