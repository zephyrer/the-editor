#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_prev_position (TEXT *text, int *offset)
{
    int o;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);

    o = *offset;
    if (o > 0)
    {
        do
        {
            o--;
        } while (o > 0 && text->data [o - 1] == '\r' && text->data [o] == '\n');

        *offset = o;
        return TRUE;
    }
    else return FALSE;
}
