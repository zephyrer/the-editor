#include <assert.h>
#include <windows.h>

#include "intlist.h"

BOOL intlist_append (INTLIST *list, int value)
{
    assert (list != NULL);
    
    return intlist_replace_range (list, list->length, 0, 1, &value);
}
