#include <assert.h>
#include <windows.h>

#include "intlist.h"

BOOL intlist_initialize (INTLIST *list, int initial_capacity)
{
    HANDLE heap = NULL;
    int *data = NULL;

    assert (list != NULL);
    assert (initial_capacity >= 0);

    if ((heap = GetProcessHeap ()) == NULL) goto error;
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

error:
    if (data != NULL)
        HeapFree (heap, 0, data);

    return FALSE;
}
