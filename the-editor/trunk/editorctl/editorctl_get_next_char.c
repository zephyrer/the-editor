#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_next_char (EDITORCTL_TEXT_ITERATOR *iterator)
{
    iterator->offset++;
    return *iterator->ptr++;
}
