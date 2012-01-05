#include <assert.h>
#include <windows.h>
#include <limits.h>

#include "utils.h"

int utils_update_max_int (const int list [], int list_length, int offset, int length, int replacement_length, const int replacement [], int old_max)
{
    int replacement_max, original_max, head_max, tail_max, m;

    assert (list != NULL);
    assert (list_length > 0);
    assert (offset >= 0);
    assert (length >= 0);
    assert (offset + length <= list_length);
    assert (replacement_length >= 0);
    assert (replacement_length == 0 || replacement != NULL);
    assert (old_max == utils_max_int (list, list_length));

    replacement_max = replacement_length == 0 ? INT_MIN : utils_max_int (replacement, replacement_length);
    if (replacement_max >= old_max) return replacement_max;

    original_max = length == 0 ? INT_MIN : utils_max_int (list + offset, length);
    if (original_max < old_max) return old_max;

    head_max = offset == 0 ? INT_MIN : utils_max_int (list, offset);
    tail_max = offset + length >= list_length ? INT_MIN : utils_max_int (list + offset + length, list_length - offset - length);
    
    m = max (head_max, tail_max);
    return max (m, replacement_max);
}
