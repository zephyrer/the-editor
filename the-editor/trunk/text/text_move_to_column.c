#include <assert.h>
#include <windows.h>

#include "text.h"

void text_move_to_column (TEXT *text, int *offset, int *column, int to_column, int tab_width)
{
    int o, l, c;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);
    assert (*column >= 0);
    assert (*column <= to_column);
    assert (tab_width > 0);

    o = *offset;
    l = text->length;
    c = *column;
    while (o < l)
    {
        char ch;
        int nc;

        ch = text->data [o];

        if (ch == '\n' || ch == '\r') break;
        else if (ch == '\t')
            nc = c + tab_width - c % tab_width;
        else nc = c + 1;

        if (nc > to_column) break;

        o += 1;
        c = nc;
    }

    *offset = o;
    *column = c;
}
