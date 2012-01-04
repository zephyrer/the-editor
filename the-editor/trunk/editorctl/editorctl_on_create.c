#include <windows.h>

#include "../editor/editor.h"
#include "editorctl.h"

LRESULT editorctl_on_create (EDITORCTL *editorctl, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HANDLE heap = NULL;

    editorctl = NULL;

    if ((heap = GetProcessHeap ()) == NULL) goto error;
    if ((editorctl = (EDITORCTL *)HeapAlloc (heap, HEAP_ZERO_MEMORY, sizeof (EDITORCTL))) == NULL) goto heap_error;

    editorctl->hwnd = hwnd;
    editorctl->heap = heap;

    editorctl->cell_size.cx = 8;
    editorctl->cell_size.cy = 16;

    editorctl->new_line = "\r\n";
    editorctl->new_line_length = 2;

    editorctl->editor.heap = NULL;
    if (!editor_initialize (&editorctl->editor, heap)) goto error;
    if (!editor_replace_range (&editorctl->editor, 0, 0, 17, "Hel\rlo,\r\n Wor\nld!", 0)) goto error;

    if (!editorctl_create_whitespace_icons (editorctl)) goto error;

    SetLastError (ERROR_SUCCESS);
    if (SetWindowLongPtr (hwnd, 0, (LONG)editorctl) == 0 && GetLastError () != ERROR_SUCCESS) goto error;

    return 0;

heap_error:
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);

error:
    if (editorctl != NULL)
    {
        if (editorctl->editor.heap != NULL)
            editor_destroy (&editorctl->editor);

        HeapFree (heap, 0, editorctl);
    }

    SetWindowLongPtr (hwnd, 0, (LPARAM)NULL);

    return -1;
}
