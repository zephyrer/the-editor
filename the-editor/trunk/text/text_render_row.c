#include <assert.h>
#include <windows.h>

#include "text.h"

void text_render_row (TEXT *text, int offset, int length, int start_column, int column, int buffer_length, int *char_buffer, int *offset_buffer, int tab_width)
{
    char ch;
    int o, eo, c;

    assert (text != NULL);
    assert (offset >= 0);
    assert (length >= 0);
    assert (offset + length <= text->length);
    assert (start_column >= 0);
    assert (column >= start_column);
    assert (buffer_length > 0);
    assert (char_buffer != NULL);
    assert (offset_buffer != NULL);

    ch = 0;
    o = offset;
    eo = offset + length;
    c = start_column;
    while (o < eo)
    {
        ch = text->data [o];
        if (ch == '\t')
        {
            int n;

            n = tab_width - c % tab_width;

            if (n == 1)
            {
                if (c >= column)
                {
                    char_buffer [c - column] = TEXT_TAB;
                    offset_buffer [c - column] = o;
                }
                c += 1;
                if (c >= column + buffer_length) return;
            }
            else
            {
                int i;

                for (i = 0; i < n; i++)
                {
                    if (c >= column)
                    {
                        char_buffer [c - column] = (i == 0) ? TEXT_TAB_BEGIN : (i == n - 1) ? TEXT_TAB_END : TEXT_TAB_MIDDLE;
                        offset_buffer [c - column] = o;
                    }
                    c += 1;
                    if (c >= column + buffer_length) return;
                }
                if (c >= column + buffer_length) return;
            }
        }
        else
        {
            if (c >= column)
            {
                char_buffer [c - column] = ch;
                offset_buffer [c - column] = o;
            }
            c += 1;
            if (c >= column + buffer_length) return;
        }

        o += 1;
    }

    if (eo != text->length && ch != '\r' && ch != '\n')
    {
        if (c >= column)
        {
            char_buffer [c - column] = TEXT_WRAP;
            offset_buffer [c - column] = -1;
        }
        c += 1;
        if (c >= column + buffer_length) return;
    }

    if (c < column) c = column;
    while (c < column + buffer_length)
    {
        char_buffer [c - column] = TEXT_NONE;
        offset_buffer [c - column] = -1;
        c += 1;
    }
}
