#include <assert.h>

#include "editorctl.h"

BOOL editorctl_set_iterator (HWND hwnd, int offset, EDITORCTL_TEXT_ITERATOR *iterator)
{
    EDITORCTL_EXTRA *extra;

    if ((extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0)) == NULL) goto error;

    iterator->offset = offset;
    iterator->text = extra->text;
    iterator->text_length = extra->text_length;

    return TRUE;
error:
    return FALSE;
}
