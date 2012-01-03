#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_row (TEXT *text, int *offset, int *col, int tab_width)
{
    int o, l, c;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);
    assert (col != NULL);
    assert (*col >= 0);
    assert (tab_width > 0);

    o = *offset;
    l = text->length;
    c = *col;
    while (o < l)
    {
        char ch;

        ch = text->data [o];
        o++;

        if (ch == '\t')
            c += (tab_width - c % tab_width);
        else c += 1;

        if (ch == '\n' || (ch == '\r' && (o >= l || text->data [o] != '\n')))
        {
            *offset = o;
            *col = c;
            return TRUE;
        }
    }

    return FALSE;
}
