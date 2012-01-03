#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_line (TEXT *text, int *offset)
{
    int o, l;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);

    o = *offset;
    l = text->length;
    while (o < l)
    {
        char ch;

        ch = text->data [o];
        o++;
        if (ch == '\n' || (ch == '\r' && (o >= l || text->data [o] != '\n')))
        {
            *offset = o;
            return TRUE;
        }
    }

    return FALSE;
}
