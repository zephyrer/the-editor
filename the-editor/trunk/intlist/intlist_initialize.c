#include <assert.h>
#include <windows.h>

#include "intlist.h"

BOOL intlist_initialize (INTLIST *list, HANDLE heap, int initial_capacity)
{
    int *data = NULL;

    assert (list != NULL);
    assert (heap != NULL);
    assert (initial_capacity >= 0);

    if ((data = (int *)HeapAlloc (
        heap, 0, initial_capacity * sizeof (int))) == NULL)
        goto heap_error;

    list->length = 0;
    list->data = data;
    list->heap = heap;
    list->capacity = initial_capacity;

    return TRUE;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
    if (data != NULL)
        HeapFree (heap, 0, data);

    return FALSE;
}
