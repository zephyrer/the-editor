#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_prev_offset (TEXT *text, int *offset)
{
    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);

    if (*offset > 0)
    {
        (*offset)--;
        return TRUE;
    }
    else return FALSE;
}
