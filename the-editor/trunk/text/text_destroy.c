#include <assert.h>
#include <windows.h>

#include "text.h"

BOOL text_destroy (TEXT *text)
{
    assert (text != NULL);
    assert (text->data != NULL);

    if (!HeapFree (text->heap, 0, text->data)) goto error;

#ifdef _DEBUG
    text->length = 0;
    text->data = NULL;
    text->heap = NULL;
    text->capacity = 0;
#endif

    return TRUE;
error:
    return FALSE;
}
