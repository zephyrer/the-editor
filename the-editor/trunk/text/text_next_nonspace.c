#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_nonspace (TEXT *text, int *offset)
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
        if (ch == '\r' || ch == '\n') break;
        else if (ch != ' ' && ch != '\t')
        {
            *offset = o;
            return TRUE;
        }

        o += 1;
    }

    return FALSE;
}
