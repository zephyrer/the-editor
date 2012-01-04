#include <assert.h>
#include <windows.h>

#include "text.h"

void text_move_to_offset (TEXT *text, int offset, int *column, int to_offset, int tab_width)
{
    int o, c;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset >= 0);
    assert (offset <= to_offset);
    assert (to_offset <= text->length);
    assert (tab_width > 0);

    o = offset;
    c = *column;
    while (o < to_offset)
    {
        if (text->data [o++] == '\t')
            c += tab_width - c % tab_width;
        else c += 1;
    }

    *column = c;
}
