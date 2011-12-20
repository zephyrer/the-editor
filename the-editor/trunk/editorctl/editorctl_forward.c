#include <assert.h>

#include "editorctl.h"

BOOL editorctl_forward (EDITORCTL_TEXT_ITERATOR *iterator)
{
    if (iterator->offset >= iterator->text_length) return FALSE;
    else
    {
        iterator->offset++;
        return TRUE;
    }
}
