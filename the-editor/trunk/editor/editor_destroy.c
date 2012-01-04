#include <assert.h>
#include <windows.h>

#include "../text/text.h"
#include "../layout/layout.h"
#include "editor.h"

BOOL editor_destroy (EDITOR *editor)
{
    assert (editor != NULL);
    
    if (!text_destroy (&editor->text)) goto error;
    if (!layout_destroy (&editor->layout)) goto error;

#ifdef _DEBUG
    ZeroMemory (editor, sizeof (EDITOR));
#endif

    return TRUE;

error:
    return FALSE;
}
