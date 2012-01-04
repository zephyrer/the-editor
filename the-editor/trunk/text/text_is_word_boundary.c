#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_is_word_boundary (TEXT *text, int offset)
{
    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset >= 0);
    assert (offset <= text->length);

    if (offset == 0 || offset == text->length) return TRUE;
    else
    {
        char pch, nch;

        pch = text->data [offset - 1];
        nch = text->data [offset];

        if (pch == '\n' || (pch == '\r' && nch != '\n') || nch == '\r' || (nch == '\n' && pch != '\r')) return TRUE;
        else if (isalnum (pch) && isalnum (nch)) return FALSE;
        else if (isspace (pch) && isspace (nch)) return FALSE;
        else return TRUE;
    }
}
