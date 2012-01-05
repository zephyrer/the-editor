#include <assert.h>
#include <windows.h>

#include "editorctl.h"

static BOOL do_update_range (EDITORCTL *editorctl)
{
    RECT r;
    SCROLLINFO si;
    int w, h;
    int width, height;
    int sx, sy;

    assert (editorctl != NULL);

    if (!GetClientRect (editorctl->hwnd, &r)) goto error;

    w = r.right - r.left;
    width = editorctl->editor.layout.max_row_width * editorctl->cell_size.cx;
    sx = editorctl->scroll_location.x;
    if (sx + w > width)
        sx = max (0, width - w);

    h = r.bottom - r.top;
    height = editorctl->editor.layout.row_offsets.length * editorctl->cell_size.cy;
    sy = editorctl->scroll_location.y;
    if (sy + h > height)
        sy = max (0, height - h);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = editorctl->editor.layout.max_row_width * editorctl->cell_size.cx;
    si.nPage = w;
    SetScrollInfo (editorctl->hwnd, SB_HORZ, &si, TRUE);

    si.cbSize = sizeof (SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = editorctl->editor.layout.row_offsets.length * editorctl->cell_size.cy;
    si.nPage = h;
    SetScrollInfo (editorctl->hwnd, SB_VERT, &si, TRUE);

    if (!editorctl_scroll_to (editorctl, sx, sy)) goto error;

    return TRUE;
error:
    return FALSE;
}

BOOL editorctl_update_scroll_range (EDITORCTL *editorctl)
{
    RECT r1, r2;

    assert (editorctl != NULL);

    if (!GetClientRect (editorctl->hwnd, &r1)) goto error;
    if (!do_update_range (editorctl)) goto error;
    if (!GetClientRect (editorctl->hwnd, &r2)) goto error;

    if (r1.left != r2.left || r1.top != r2.top || r1.right != r2.right || r1.bottom != r2.bottom)
    {
        if (!do_update_range (editorctl)) goto error;
    }

    return TRUE;
error:
    return FALSE;
}
