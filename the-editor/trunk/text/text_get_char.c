#include <assert.h>
#include <windows.h>

#include "text.h"

UINT32 text_get_char (TEXT *text, int offset)
{
    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset >= 0);
    assert (offset < text->length);

    return text->data [offset];
}
