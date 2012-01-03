#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_next_row_wrap (TEXT *text, int *offset, int *col, int tab_width, int wrap, int min_wrap, BOOL *new_line)
{
    int o, l, c, ro, rc;

    assert (text != NULL);
    assert (text->data != NULL);
    assert (offset != NULL);
    assert (*offset >= 0);
    assert (*offset <= text->length);
    assert (col != NULL);
    assert (*col >= 0);
    assert (*col <= min_wrap);
    assert (tab_width > 0);
    assert (min_wrap <= wrap);
    assert (new_line != NULL);

    o = *offset;
    l = text->length;
    c = *col;
    ro = -1;
    while (o < l)
    {
        char ch;
        int nc;

        ch = text->data [o];

        if (ch == '\t')
            nc = c + (tab_width - c % tab_width);
        else nc = c + 1;

        if (ch != '\n' && nc > wrap)
        {
            if (ro >= 0)
            {
                *offset = ro;
                *col = rc;
            }
            else
            {
                *offset = o;
                *col = c;
            }

            *new_line = FALSE;
            return TRUE;
        }

        o++;
        c = nc;

        if (ch == '\n' || (ch == '\r' && (o >= l || text->data [o] != '\n')))
        {
            *offset = o;
            *col = c;
            *new_line = TRUE;
            return TRUE;
        }

        if (c >= min_wrap && (ch == ' ' || ch == '\t'))
        {
            ro = o;
            rc = c;
        }
    }

    return FALSE;
}
