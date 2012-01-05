#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_prev_word_boundary (TEXT *text, int *offset)
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
        } while (o > 0 && !text_is_word_boundary (text, o));

        *offset = o;
        return TRUE;
    }
    else return FALSE;
}
