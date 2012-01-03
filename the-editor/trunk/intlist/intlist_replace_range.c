#include <assert.h>
#include <windows.h>

#include "intlist.h"

BOOL intlist_replace_range (INTLIST *list, int offset, int length, int replacement_length, const int replacement [])
{
    int new_length;
    int *offset_ptr;

    assert (list != NULL);
    assert (list->data != NULL);
    assert (offset >= 0);
    assert (length >= 0);
    assert (offset + length <= list->length);
    assert (replacement_length >= 0);
    assert (replacement_length == 0 || replacement != NULL);

    new_length = list->length - length + replacement_length;
    if (new_length > list->capacity)
    {
        int new_capacity;
        int *new_data;

        new_capacity = new_length * 3 / 2;
        if ((new_data = (int *)HeapReAlloc (list->heap, 0, list->data, new_capacity * sizeof (int))) == NULL)
            goto heap_error;

        list->data = new_data;
        list->capacity = new_capacity;
    }

    offset_ptr = list->data + offset;
    CopyMemory (
        offset_ptr + replacement_length, 
        offset_ptr + length, 
        (list->length - offset - length) * sizeof (int));
    CopyMemory (
        offset_ptr,
        replacement,
        replacement_length * sizeof (int));

    return TRUE;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}
