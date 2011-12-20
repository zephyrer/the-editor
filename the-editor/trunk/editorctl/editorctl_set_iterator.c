#include <assert.h>

#include "editorctl.h"

BOOL editorctl_set_iterator (HWND hwnd, int offset, EDITORCTL_TEXT_ITERATOR *iterator)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    assert (offset >= 0);
    assert (offset <= extra->text_length);

    iterator->offset = offset;

    iterator->is_begin = (offset == 0);
    iterator->is_end = (offset == extra->text_length);

    iterator->prev_char = (offset == 0) ? 0 : extra->text [offset - 1];
    iterator->next_char = (offset == extra->text_length) ? 0 : extra->text [offset];

    iterator->ptr = extra->text + offset;
    iterator->text_length = extra->text_length;

    return TRUE;
error:
    return FALSE;
}
