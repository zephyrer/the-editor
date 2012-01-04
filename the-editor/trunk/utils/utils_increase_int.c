#include <assert.h>
#include <windows.h>

#include "utils.h"

void utils_increase_int (int list [], int list_length, int delta)
{
    int i;

    assert (list != NULL);
    assert (list_length >= 0);

    for (i = 0; i < list_length; i++)
        list [i] += delta;
}
