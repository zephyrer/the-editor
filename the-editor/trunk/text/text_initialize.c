#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_initialize (TEXT *text, int initial_capacity)
{
    HANDLE heap = NULL;
    char *data = NULL;

    assert (text != NULL);
    assert (initial_capacity >= 0);

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((data = (char *)HeapAlloc (
        heap, 0, initial_capacity * sizeof (char))) == NULL)
        goto heap_error;

    text->length = 0;
    text->data = data;
    text->heap = heap;
    text->capacity = initial_capacity;

    return TRUE;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);

error:
    if (data != NULL)
        HeapFree (heap, 0, data);

    return FALSE;
}
