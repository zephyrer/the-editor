#include <assert.h>
#include <windows.h>

#include "intlist.h"

BOOL intlist_destroy (INTLIST *list)
{
    assert (list != NULL);
    assert (list->data != NULL);

    if (!HeapFree (list->heap, 0, list->data)) goto error;

#ifdef _DEBUG
    list->length = 0;
    list->data = NULL;
    list->heap = NULL;
    list->capacity = 0;
#endif

    return TRUE;
error:
    return FALSE;
}
