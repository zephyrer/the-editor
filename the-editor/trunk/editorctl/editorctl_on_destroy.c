#include <windows.h>

#include "editorctl.h"

LRESULT editorctl_on_destroy (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EDITORCTL_EXTRA *extra;

    extra = (EDITORCTL_EXTRA *)GetWindowLongPtr (hwnd, 0);

    if (extra != NULL)
    {
        if (extra->row_offsets.data != NULL) utils_destroy_int_list (&extra->row_offsets);
        if (extra->row_widths.data != NULL) utils_destroy_int_list (&extra->row_widths);
        if (extra->line_rows.data != NULL) utils_destroy_int_list (&extra->line_rows);
        HeapFree (extra->heap, 0, extra);
    }

    SetWindowLongPtr (hwnd, 0, (LPARAM)NULL);

    return 0;
}
