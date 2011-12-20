#include <assert.h>

#include "editorctl.h"

BOOL editorctl_backward (EDITORCTL_TEXT_ITERATOR *iterator)
{
    if (iterator->offset == 0) return FALSE;
    else
    {
        iterator->offset--;
        return TRUE;
    }
}
