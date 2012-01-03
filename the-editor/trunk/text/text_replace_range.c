#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_replace_range (TEXT *text, int offset, int length, int replacement_length, const char replacement [])
{
    int new_length;
    char *offset_ptr;

    assert (text != NULL);
    assert (offset >= 0);
    assert (length >= 0);
    assert (offset + length <= text->length);
    assert (replacement_length >= 0);
    assert (replacement_length == 0 || replacement != NULL);


    new_length = text->length - length + replacement_length;
    if (new_length > text->capacity)
    {
        int new_capacity;
        char *new_data;

        new_capacity = new_length * 3 / 2;
        if ((new_data = (char *)HeapReAlloc (text->heap, 0, text->data, new_capacity * sizeof (char))) == NULL)
            goto heap_error;

        text->data = new_data;
        text->capacity = new_capacity;
    }

    offset_ptr = text->data + offset;
    CopyMemory (
        offset_ptr + replacement_length, 
        offset_ptr + length, 
        (text->length - offset - length) * sizeof (char));
    CopyMemory (
        offset_ptr,
        replacement,
        replacement_length * sizeof (char));

    return TRUE;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}
