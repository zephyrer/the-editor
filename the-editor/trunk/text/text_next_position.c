#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_position (TEXT *text, int *offset)
{
    int o, l;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);

    o = *offset;
    l = text->length;
    if (o < l)
    {
        do
        {
            o++;
        } while (o < l && text->data [o - 1] == '\r' && text->data [o] == '\n');

        *offset = o;
        return TRUE;
    }
    else return FALSE;
}
