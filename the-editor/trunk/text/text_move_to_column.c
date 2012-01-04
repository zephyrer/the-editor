#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_move_to_column (TEXT *text, int *offset, int *column, int to_column, int tab_width)
{
    int o, l, c;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);
    assert (*column >= 0);
    assert (*column < to_column);
    assert (tab_width > 0);

    o = *offset;
    l = text->length;
    c = *column;
    while (o < l)
    {
        char ch;
        int nc;

        ch = text->data [o];
        if (ch == '\t')
            nc = c + tab_width - c % tab_width;
        else nc = c + 1;

        if (nc > to_column)
        {
            *offset = o;
            *column = c;
            return TRUE;
        }

        o += 1;
        c = nc;

        if (o >= l || ch == '\n' || (ch == '\r' && (o >= l || text->data [o] != '\n')))
        {
            *offset = o;
            *col = c;
            return TRUE;
        }
    }

    return FALSE;
}
