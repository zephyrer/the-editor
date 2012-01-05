#include <assert.h>
#include <windows.h>

#include "utils.h"

int utils_bsearch_int (int value, const int list [], int length)
{
    assert (list != NULL);
    assert (length >= 0);

    if (length == 0)
        return -1;
    else
    {
        register int a, b, va, vb;

        a = 0;
        va = list [a];
        b = length - 1;
        vb = list [b];

        if (value < va)
            return -1;
        else if (value == va)
            return a;
        else if (value == vb)
            return b;
        else if (value > vb)
            return (-length - 1);
        else
        {
            while (b - a > 1)
            {
                 register int c, vc;

                c = (a + b) / 2;
                vc = list [c];

                if (value < vc)
                    b = c;
                else if (value == vc)
                    return c;
                else if (value > vc)
                    a = c;
                else assert (FALSE); // Impossible
            }

            return (-b - 1);
        }
    }
}
