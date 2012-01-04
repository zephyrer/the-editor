#include <assert.h>
#include <windows.h>

#include "utils.h"

int utils_max_int (const int list [], int length)
{
    int result, i;

    assert (list != NULL);
    assert (length > 0);

    result = list [0];
    for (i = 1; i < length; i++)
        result = max (result, list [i]);

    return result;
}
