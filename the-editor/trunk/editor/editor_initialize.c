#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"
#include "editor.h"

BOOL editor_initialize (EDITOR *editor, HANDLE heap)
{
    assert (editor != NULL);
    assert (heap != NULL);

    ZeroMemory (editor, sizeof (EDITOR));

    if (!text_initialize (&editor->text, heap, 16)) goto error;
    if (!layout_initialize (&editor->layout, heap, &editor->text, 8, -1, -1)) goto error;

    editor->heap = heap;

    return TRUE;

error:
    if (editor->text.data != NULL)
        text_destroy (&editor->text);

    if (editor->layout.text != NULL)
        layout_destroy (&editor->layout);

    return FALSE;
}
