#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_is_line_boundary (TEXT *text, int offset)
{
    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset >= 0);
    assert (offset <= text->length);

    if (offset == 0) return FALSE;
    else
    {
        char pch;

        pch = text->data [offset - 1];

        return pch == '\n' || (pch == '\r' && (offset >= text->length || text->data [offset] != '\n'));
    }
}
