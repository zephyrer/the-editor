#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_word_boundary (TEXT *text, int *offset)
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
        } while (o < l && !text_is_word_boundary (text, o));

        *offset = o;
        return TRUE;
    }
    else return FALSE;
}
