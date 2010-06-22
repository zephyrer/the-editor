#include "StdAfx.h"
#include "Editor.h"
#include "EditorDocument.h"
#include "EditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CEditorControl

BEGIN_MESSAGE_MAP(CEditorControl, CWnd)
    ON_WM_SIZE ()
    ON_WM_PAINT ()
    ON_WM_VSCROLL ()
    ON_WM_HSCROLL ()
    ON_WM_SETFOCUS ()
    ON_WM_KILLFOCUS ()
    ON_WM_LBUTTONDOWN ()
    ON_WM_MOUSEMOVE ()
    ON_WM_LBUTTONUP ()
    ON_WM_MOUSEWHEEL ()
    ON_WM_CHAR ()
    ON_WM_KEYDOWN ()
    ON_WM_CONTEXTMENU ()
END_MESSAGE_MAP()

BOOL CEditorControl::PreCreateWindow (CREATESTRUCT& cs)
{
    cs.lpszClass = AfxRegisterWndClass (0, ::LoadCursor(NULL, IDC_IBEAM),  NULL, NULL); 

    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.style |= CS_HREDRAW | CS_VREDRAW;

    return TRUE;
}

void CEditorControl::OnSize (UINT nType, int cx, int cy)
{
    if (cx != 0 && cy != 0)
        UpdateScrollBars ();

    CWnd::OnSize (nType, cx, cy);
}

void CEditorControl::OnPaint ()
{
    if (view.layout == NULL) return;
    CTextLayout &layout = *view.layout;

    CPaintDC paintDC (this);

    paintDC.OffsetViewportOrg (-GetScrollPos (SB_HORZ), -GetScrollPos (SB_VERT));

    CRect clip_rect;
    paintDC.GetClipBox (&clip_rect);

    CRect client_rect;
    GetClientRect (&client_rect);

    client_rect.OffsetRect (GetScrollPos (SB_HORZ), GetScrollPos (SB_VERT));

    if (clip_rect.IsRectEmpty ()) return;

    CDC &DC = paintDC;

    CBrush b;
    b.CreateSysColorBrush (COLOR_WINDOW);

    DC.FillRect (CRect (0, 0, client_rect.right, view.padding_top), &b);
    DC.FillRect (CRect (0, view.padding_top, view.padding_left, client_rect.bottom), &b);

    DC.SelectObject (view.GetFont ());

    int left = clip_rect.left - view.padding_left;
    if (left < 0) left = 0;
    int right = clip_rect.right - view.padding_left;
    if (right < 0) right = 0;
    int top = clip_rect.top - view.padding_top;
    if (top < 0) top = 0;
    int bottom = clip_rect.bottom - view.padding_top;
    if (bottom < 0) bottom = 0;

    if (left == right || top == bottom) return;

    unsigned int start_row = top / view.cell_size.cy;
    unsigned int end_row = (bottom - 1) / view.cell_size.cy;
    unsigned int row_count = end_row - start_row + 1;
    unsigned int start_column = left / view.cell_size.cx;
    unsigned int end_column = (right - 1) / view.cell_size.cx;
    unsigned int column_count = end_column - start_column + 1;

    if (row_count == 0 || column_count == 0) return;

    TEXTCELL *temp = (TEXTCELL *)alloca (row_count * column_count * sizeof (TEXTCELL));
    layout.GetCellsRange (start_row, start_column, row_count, column_count, temp);

    for (unsigned int r = 0; r < row_count; r++)
    {
        unsigned int row = start_row + r;

        int sel_start = -1;
        int sel_end = -1;

        if (view.cursor != NULL)
        {
            CTextSelection *selection = view.cursor->GetSelection ();

            if (selection != NULL)
            {
                for (unsigned int c = 0; c < column_count; c++)
                {
                    if (selection != NULL && selection->IsCellSelected (temp [r * column_count + c]))
                    {
                        if (sel_start < 0) sel_start = start_column + c;
                        sel_end = start_column + c + 1;
                    }
                }
            }
        }

        if (sel_start >= sel_end)
            PaintCharsRange (DC, row, start_column, column_count, false, &temp [r * column_count]);
        else
        {
            PaintCharsRange (DC, row, start_column, sel_start - start_column, false, &temp [r * column_count]);
            PaintCharsRange (DC, row, sel_start, sel_end - sel_start, true, &temp [r * column_count + sel_start - start_column]);
            PaintCharsRange (DC, row, sel_end, end_column - sel_end + 1, false, &temp [r * column_count + sel_end - start_column]);
        }
    }
}

void CEditorControl::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    SCROLLINFO si;

    si.cbSize = sizeof (si);
    GetScrollInfo (SB_HORZ, &si, SIF_ALL);

    CRect client_rect;
    GetClientRect (&client_rect);

    switch (nSBCode)
    {
    case SB_LEFT:
        Scroll (-si.nPos, 0);
        break;
    case SB_RIGHT:
        Scroll (si.nMax - si.nPos, 0);
        break;
    case SB_LINELEFT:
        Scroll (-view.cell_size.cx, 0);
        break;
    case SB_LINERIGHT:
        Scroll (view.cell_size.cx, 0);
        break;
    case SB_PAGELEFT:
        Scroll (-client_rect.Width (), 0);
        break;
    case SB_PAGERIGHT:
        Scroll (client_rect.Width (), 0);
        break;
    case SB_THUMBPOSITION:
        Scroll (si.nTrackPos - si.nPos, 0);
        break;
    case SB_THUMBTRACK:
        Scroll (si.nTrackPos - si.nPos, 0);
        break;
    }

    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEditorControl::OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    SCROLLINFO si;

    si.cbSize = sizeof (si);
    GetScrollInfo (SB_VERT, &si, SIF_ALL);

    CRect client_rect;
    GetClientRect (&client_rect);

    switch (nSBCode)
    {
    case SB_TOP:
        Scroll (0, -si.nPos);
        break;
    case SB_BOTTOM:
        Scroll (0, si.nMax - si.nPos);
        break;
    case SB_LINEUP:
        Scroll (0, -view.cell_size.cy);
        break;
    case SB_LINEDOWN:
        Scroll (0, view.cell_size.cy);
        break;
    case SB_PAGEUP:
        Scroll (0, -client_rect.Height ());
        break;
    case SB_PAGEDOWN:
        Scroll (0, client_rect.Height ());
        break;
    case SB_THUMBPOSITION:
        Scroll (0, si.nTrackPos - si.nPos);
        break;
    case SB_THUMBTRACK:
        Scroll (0, si.nTrackPos - si.nPos);
        break;
    }

    CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CEditorControl::OnSetFocus (CWnd* pOldWnd)
{
    UpdateCaret ();
}

void CEditorControl::OnKillFocus (CWnd* pNewWnd)
{
    DestroyCaret ();
}

void CEditorControl::OnLButtonDown (UINT nFlags, CPoint point)
{
    if (view.layout == NULL) return;
    if (view.cursor == NULL) return;

    CTextLayout &layout = *view.layout;
    CTextCursor &cursor = *view.cursor;

    unsigned int tc = GetTickCount ();
    if (click_counter > 0 && 
        click_counter < 3 &&
        tc - last_click_time <= GetDoubleClickTime () &&
        abs (point.x - last_click_point.x) <= GetSystemMetrics (SM_CXDOUBLECLK) &&
        abs (point.y - last_click_point.y) <= GetSystemMetrics (SM_CYDOUBLECLK))
        click_counter++;
    else click_counter = 1;

    last_click_time = tc;
    last_click_point = point;

    SetFocus ();

    unsigned int row;
    unsigned int column;
    ClientToRowColumn (point.x + 2, point.y, row, column);

    if (click_counter == 1)
    {
        if ((nFlags & MK_CONTROL) != 0)
        {
            cursor.WordClick (row, column);
            drag_type = 2;
        }
        else
        {
            cursor.Click (row, column, (nFlags & MK_SHIFT) != 0);
            drag_type = 1;
        }
    }
    else if (click_counter == 2)
    {
        cursor.WordClick (row, column);
        drag_type = 2;
    }
    else if (click_counter == 3)
    {
        cursor.LineClick (row, column);
        drag_type = 3;
    }

    EnsureCaretVisible ();

    UpdateCaret ();

    SetCapture ();

    ValidateCursor ();
}

void CEditorControl::OnMouseMove (UINT nFlags, CPoint point)
{
    if (view.layout == NULL) return;
    if (view.cursor == NULL) return;

    CTextLayout &layout = *view.layout;
    CTextCursor &cursor = *view.cursor;

    if (nFlags & MK_LBUTTON && drag_type != 0)
    {
        unsigned int row;
        unsigned int column;
        ClientToRowColumn (point.x + 2, point.y, row, column);

        switch (drag_type)
        {
        case 1:
            cursor.Drag (row, column);
            break;
        case 2:
            cursor.WordDrag (row, column);
            break;
        case 3:
            cursor.LineDrag (row, column);
            break;
        }

        EnsureCaretVisible ();
        UpdateCaret ();
        ValidateCursor ();
    }
}

void CEditorControl::OnLButtonUp (UINT nFlags, CPoint point)
{
    ReleaseCapture ();

    drag_type = 0;
}

BOOL CEditorControl::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
    if (nFlags & MK_SHIFT)
    {
        Scroll (-zDelta * view.cell_size.cx / 10, 0);
    }
    else
    {
        Scroll (0, -zDelta * view.cell_size.cy / 40);
    }

    return CWnd::OnMouseWheel (nFlags, zDelta, pt);
}

void CEditorControl::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (view.cursor == NULL) return;
    CTextCursor &cursor = *view.cursor;

    for (unsigned int i = 0; i < nRepCnt; i++)
    {
        switch (nChar)
        {
        case '\b':
            cursor.Backspace ();
            break;
        case '\r':
            cursor.NewLine ();
            break;
        case '\033':
            break;
        case '\t':
            cursor.InsertChar (nChar);
            break;
        case '\n':
            cursor.NewLine ();
            break;
        case '\177':
            cursor.WordBackspace ();
            break;
        default: 
            if (overwrite_mode && cursor.CanOverwrite ())
                cursor.OverwriteChar (nChar);
            else cursor.InsertChar (nChar);
            break; 
        }
    }

    UpdateScrollBars ();
    EnsureCaretVisible ();
    UpdateCaret ();
    ValidateCursor ();

    CWnd::OnChar (nChar, nRepCnt, nFlags);
}

void CEditorControl::OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (view.cursor == NULL) return;
    CTextCursor &cursor = *view.cursor;

    CRect client_rect;
    GetClientRect (&client_rect);

    unsigned int page_rows = client_rect.Height () / view.cell_size.cy;

    for (unsigned int i = 0; i < nRepCnt; i++)
    {
        bool shift = (GetKeyState (VK_SHIFT) & 0x80000) != 0;
        bool ctrl = (GetKeyState (VK_CONTROL) & 0x80000) != 0;

        switch (nChar)
        {
        case VK_LEFT:
            ctrl ? cursor.WordLeft (shift) : cursor.Left (shift);
            EnsureCaretVisible ();
            break;
        case VK_RIGHT:
            ctrl ? cursor.WordRight (shift) : cursor.Right (shift);
            EnsureCaretVisible ();
            break;
        case VK_UP:
            if (ctrl)
            {
                Scroll (0, -view.cell_size.cy);
            }
            else
            {
                cursor.Up (shift);
                EnsureCaretVisible ();
            }
            break;
        case VK_DOWN:
            if (ctrl)
            {
                Scroll (0, view.cell_size.cy);
            }
            else
            {
                cursor.Down (shift);
                EnsureCaretVisible ();
            }
            break;
        case VK_PRIOR:
            cursor.PageUp (page_rows, shift);
            EnsureCaretVisible ();
            break;
        case VK_NEXT:
            cursor.PageDown (page_rows, shift);
            EnsureCaretVisible ();
            break;
        case VK_HOME:
            ctrl ? cursor.TextBegin (shift) : cursor.Home (shift);
            EnsureCaretVisible ();
            break;
        case VK_END:
            ctrl ? cursor.TextEnd (shift) : cursor.End (shift);
            EnsureCaretVisible ();
            break;
        case VK_DELETE:
            ctrl ? cursor.WordDel () : cursor.Del ();
            UpdateScrollBars ();
            EnsureCaretVisible ();
            break;
        case VK_INSERT:
            overwrite_mode = !overwrite_mode;
            UpdateCaret ();
            break;
        }
    }

    UpdateCaret ();
    ValidateCursor ();

    CWnd::OnKeyDown (nChar, nRepCnt, nFlags);
}

void CEditorControl::OnContextMenu (CWnd* pWnd, CPoint point)
{
    CPoint p (point);
    ScreenToClient (&p);

    unsigned int row;
    unsigned int column;
    ClientToRowColumn (p.x + 2, p.y, row, column);

    view.cursor->RightClick (row, column);

    EnsureCaretVisible ();
    UpdateCaret ();
    ValidateCursor ();

    CMenu* menu_bar = AfxGetMainWnd ()->GetMenu ();
    CMenu* edit_menu = menu_bar->GetSubMenu (1);
    ASSERT (edit_menu);

    edit_menu->TrackPopupMenu (TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd ());

    CWnd::OnContextMenu (pWnd, point);
}

void CEditorControl::ClientToRowColumn (int x, int y, unsigned int &row, unsigned int &column)
{
    x += GetScrollPos (SB_HORZ);
    y += GetScrollPos (SB_VERT);

    if (y >= 0 && (unsigned int)y >= view.padding_top)
        row = (y - view.padding_top) / view.cell_size.cy;
    else row = 0;

    if (x >= 0 && (unsigned int)x >= view.padding_left)
        column = (x - view.padding_left) / view.cell_size.cx;
    else column = 0;
}

void CEditorControl::UpdateScrollBars ()
{
    if (view.layout == NULL) return;

    CTextLayout &layout = *view.layout;

    ShowScrollBar (SB_BOTH, TRUE);

    view.UpdateLayout ();

    CRect client_rect;

    GetClientRect (&client_rect);

    int x = GetScrollPos (SB_HORZ);
    int y = GetScrollPos (SB_VERT);

    SCROLLINFO si;

    si.cbSize = sizeof (si);
    si.fMask = SIF_RANGE | SIF_PAGE;

    si.nPos = 0;
    si.nMin = 0;
    si.nMax = view.padding_left + layout.GetWidth () * view.cell_size.cx + view.padding_right;
    si.nPage = client_rect.Width ();

    SetScrollInfo (SB_HORZ, &si);
    EnableScrollBar (SB_HORZ, (int)si.nPage >= (int)si.nMax ? ESB_DISABLE_BOTH : ESB_ENABLE_BOTH);

    si.nPos = 0;
    si.nMin = 0;
    si.nMax = view.padding_top + layout.GetHeight () * view.cell_size.cy + view.padding_bottom;
    si.nPage = client_rect.Height ();

    SetScrollInfo (SB_VERT, &si);
    EnableScrollBar (SB_VERT, (int)si.nPage >= (int)si.nMax ? ESB_DISABLE_BOTH : ESB_ENABLE_BOTH);

    if (x != GetScrollPos (SB_HORZ) || y != GetScrollPos (SB_VERT))
        Invalidate ();
}

void CEditorControl::UpdateCaret ()
{
    if (GetFocus () != this) return;

    if (view.cursor == NULL) return;
    CTextCursor &cursor = *view.cursor;

    int w = GetSystemMetrics (SM_CXBORDER);
    int h = view.cell_size.cy;

    if (overwrite_mode && cursor.CanOverwrite ())
    {
        w = view.cell_size.cx;
    }

    CreateSolidCaret (w, h);
    
    SetCaretPos (
        CPoint (
            view.padding_left + cursor.GetCursorColumn () * view.cell_size.cx - GetScrollPos (SB_HORZ),
            view.padding_top + cursor.GetCursorRow () * view.cell_size.cy - GetScrollPos (SB_VERT)));

    ShowCaret ();
}

void CEditorControl::Scroll (int dx, int dy)
{
    if (dx == 0 && dy == 0) return;

    SCROLLINFO si;

    si.cbSize = sizeof (si);

    GetScrollInfo (SB_HORZ, &si, SIF_ALL);

    int oldPos = si.nPos;

    si.nPos += dx;

    if ((int)si.nPos + (int)si.nPage > (int)si.nMax + 1)
        si.nPos = (int)si.nMax - (int)si.nPage + 1;

    if (si.nPos < 0)
        si.nPos = 0;

    SetScrollInfo (SB_HORZ, &si, SIF_POS);

    int deltaX = si.nPos - oldPos;

    GetScrollInfo (SB_VERT, &si, SIF_ALL);

    oldPos = si.nPos;

    si.nPos += dy;

    if ((int)si.nPos + (int)si.nPage > (int)si.nMax + 1)
        si.nPos = (int)si.nMax - (int)si.nPage + 1;

    if (si.nPos < 0)
        si.nPos = 0;

    SetScrollInfo (SB_VERT, &si, SIF_POS);

    int deltaY = si.nPos - oldPos;

    ScrollWindow (-deltaX, -deltaY);

    view.line_numbers_control.ScrollWindow (0, -deltaY);

    UpdateCaret ();
}

void CEditorControl::EnsureRectVisible (CRect &rect, CRect &margins)
{
    CRect client_rect;

    GetClientRect (client_rect);

    client_rect.OffsetRect (GetScrollPos (SB_HORZ), GetScrollPos (SB_VERT));

    int dx = 0;
    int dy = 0;

    if (rect.right > client_rect.right)
    {
        dx = margins.right - client_rect.right;

        if (client_rect.left + dx > rect.left)
            dx = rect.left - client_rect.left;
    }
    else if (rect.left < client_rect.left)
    {
        dx = margins.left - client_rect.left;

        if (client_rect.right + dx < rect.right)
            dx = rect.right - client_rect.right;
    }

    if (rect.bottom > client_rect.bottom)
    {
        dy = margins.bottom - client_rect.bottom;

        if (client_rect.top + dx > rect.top)
            dy = rect.top - client_rect.top;
    }
    else if (rect.top < client_rect.top)
    {
        dy = margins.top - client_rect.top;

        if (client_rect.bottom + dx < rect.bottom)
            dx = rect.bottom - client_rect.bottom;
    }

    Scroll (dx, dy);
}

void CEditorControl::EnsureCaretVisible ()
{
    if (view.cursor == NULL) return;
    CTextCursor &cursor = *view.cursor;

    unsigned int x = view.padding_left + cursor.GetCursorColumn () * view.cell_size.cx;
    unsigned int y = view.padding_top + cursor.GetCursorRow () * view.cell_size.cy;

    EnsureRectVisible (CRect (x, y, x + view.cell_size.cx, y + view.cell_size.cy), CRect (x - view.cell_size.cx * 8, y, x + view.cell_size.cx * 9, y + view.cell_size.cy));
}

void CEditorControl::PaintCharsRange (CDC &DC, int row, int start_column, int count, bool selected, TEXTCELL *cells)
{
    if (view.layout == NULL) return;
    CTextLayout &layout = *view.layout;

    if (cells == NULL)
    {
        cells = (TEXTCELL *)alloca (count * sizeof (TEXTCELL));
        layout.GetCellsRange (row, start_column, 1, count, cells);
    }

    TCHAR *chars = (TCHAR *)alloca (count * sizeof (TCHAR));
    int *widths = (int *)alloca (count * sizeof (int));

    for (int i = 0; i < count; i++)
    {
        if (cells [i].flags & TCF_PRINTABLE)
            chars [i] = cells [i].character;
        else chars [i] = L' ';
        widths [i] = view.cell_size.cx;
    }

    int x = view.padding_left + start_column * view.cell_size.cx;
    int y = view.padding_top + row * view.cell_size.cy;

    CRect rect (
        CPoint (x, y),
        CSize (count * view.cell_size.cx, view.cell_size.cy));

    if (selected)
    {
        DC.SetTextColor (GetSysColor (COLOR_HIGHLIGHTTEXT));
        DC.SetBkColor (GetSysColor (COLOR_HIGHLIGHT));
    }
    else
    {
        DC.SetTextColor (GetSysColor (COLOR_WINDOWTEXT));
        DC.SetBkColor (GetSysColor (COLOR_WINDOW));
    }

    DC.SetBkMode (TRANSPARENT);

    DC.ExtTextOut (x, y, ETO_OPAQUE, &rect, chars, count, widths);
}

void CEditorControl::ValidateCursor ()
{
    if (view.cursor == NULL) return;
    CTextCursor &cursor = *view.cursor;

    unsigned int start_dirty_row = cursor.GetStartDirtyRow ();
    unsigned int dirty_row_count = cursor.GetDirtyRowCount ();

    if (dirty_row_count == 0) return;

    CRect r, r1;

    GetClientRect (&r);
    view.line_numbers_control.GetClientRect (&r1);

    r.top = start_dirty_row * view.cell_size.cy + view.padding_top;
    r.bottom = r.top + dirty_row_count * view.cell_size.cy;
    r1.top = start_dirty_row * view.cell_size.cy + view.padding_top;
    r1.bottom = r.top + dirty_row_count * view.cell_size.cy;

    r.OffsetRect (0, -GetScrollPos (SB_VERT));
    r1.OffsetRect (0, -GetScrollPos (SB_VERT));

    InvalidateRect (&r);
    view.line_numbers_control.InvalidateRect (&r1);

    cursor.ResetDirtyRows ();
}

#pragma endregion

#pragma region CLineNumbersControl

BEGIN_MESSAGE_MAP (CLineNumbersControl, CWnd)
    ON_WM_PAINT ()
END_MESSAGE_MAP ()

BOOL CLineNumbersControl::PreCreateWindow (CREATESTRUCT& cs)
{
    cs.lpszClass = AfxRegisterWndClass (0, ::LoadCursor(NULL, IDC_ARROW), NULL, NULL); 

    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.style |= CS_HREDRAW | CS_VREDRAW;

    return TRUE;
}

void CLineNumbersControl::OnPaint ()
{
    if (view.layout == NULL) return;
    CTextLayout &layout = *view.layout;

    CPaintDC paintDC (this);

    paintDC.OffsetViewportOrg (0, -view.editor_control.GetScrollPos (SB_VERT));

    CRect clip_rect;
    paintDC.GetClipBox (&clip_rect);

    if (clip_rect.IsRectEmpty ()) return;

    CDC &DC = paintDC;

    CRect client_rect;
    GetClientRect (&client_rect);

    client_rect.OffsetRect (0, view.editor_control.GetScrollPos (SB_VERT));

    CBrush b;
    b.CreateSysColorBrush (COLOR_WINDOW);
    DC.FillRect (&CRect (0, 0, client_rect.right, view.padding_top), &b);
    DC.FillRect (&CRect (client_rect.right - view.line_numbers_padding_right, view.padding_top, client_rect.right, client_rect.bottom), &b);

    DC.SelectObject (view.GetFont ());
    DC.SetTextColor (view.line_numbers_color);
    DC.SetTextAlign (TA_RIGHT);

    unsigned int begin = client_rect.top > 0 && (unsigned int)clip_rect.top > view.padding_top ? ((unsigned int)clip_rect.top - view.padding_top) / view.cell_size.cy : 0;
    unsigned int end = client_rect.bottom > 0 && (unsigned int)clip_rect.bottom > view.padding_top ? ((unsigned int)clip_rect.bottom - view.padding_top) / view.cell_size.cy : 0;

    TEXTCELL tc;

    int prev_line = -1;
    if (begin > 0)
    {
        layout.GetCellAt (begin - 1, 0, tc);
        prev_line = tc.line;
    }

    for (unsigned int i = begin; i <= end; i++)
    {
        layout.GetCellAt (i, 0, tc);

        TCHAR temp [256];

        if (tc.line != prev_line)
            _sntprintf_s (temp, 256, 255, L"%d", tc.line + 1);
        else temp [0] = NULL;

        DC.ExtTextOut (
            client_rect.right - view.line_numbers_padding_right,
            i * view.cell_size.cy + view.padding_top,
            ETO_OPAQUE,
            &CRect (0, i * view.cell_size.cy + view.padding_top, client_rect.right - view.line_numbers_padding_right + 1, (i + 1) * view.cell_size.cy + view.padding_top),
            temp,
            _tcslen (temp),
            NULL);

        prev_line = tc.line;
    }
}

#pragma endregion

#pragma region CEditorView

IMPLEMENT_DYNCREATE(CEditorView, CView)

BEGIN_MESSAGE_MAP(CEditorView, CView)
    ON_WM_CREATE ()
    ON_WM_SIZE ()
    ON_WM_SETFOCUS ()
    ON_WM_CHAR ()

    ON_MESSAGE (WM_SETFONT, &CEditorView::OnSetFont)
    ON_MESSAGE (WM_GETFONT, &CEditorView::OnGetFont)

    ON_COMMAND (ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND (ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND (ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)

    ON_UPDATE_COMMAND_UI (ID_VIEW_LINE_NUMBERS, &CEditorView::OnUpdateViewLineNumbersMenu)
    ON_COMMAND_EX (ID_VIEW_LINE_NUMBERS, &CEditorView::OnViewLineNumbersCheck)
    ON_UPDATE_COMMAND_UI (ID_EDIT_UNDO, &CEditorView::OnUpdateUndoMenu)
    ON_COMMAND_EX (ID_EDIT_UNDO, &CEditorView::OnUndo)
    ON_UPDATE_COMMAND_UI (ID_EDIT_REDO, &CEditorView::OnUpdateRedoMenu)
    ON_COMMAND_EX (ID_EDIT_REDO, &CEditorView::OnRedo)

    ON_COMMAND_EX (ID_EDIT_SELECT_ALL, &CEditorView::OnSelectAll)
    ON_COMMAND_EX (ID_EDIT_CLEAR, &CEditorView::OnClear)

    ON_UPDATE_COMMAND_UI (ID_EDIT_COPY, &CEditorView::OnUpdateCopyMenu)
    ON_COMMAND_EX (ID_EDIT_COPY, &CEditorView::OnCopy)

    ON_UPDATE_COMMAND_UI (ID_EDIT_PASTE, &CEditorView::OnUpdatePasteMenu)
    ON_COMMAND_EX (ID_EDIT_PASTE, &CEditorView::OnPaste)

    ON_UPDATE_COMMAND_UI (ID_EDIT_CUT, &CEditorView::OnUpdateCutMenu)
    ON_COMMAND_EX (ID_EDIT_CUT, &CEditorView::OnCut)
END_MESSAGE_MAP()

CEditorView::CEditorView () : editor_control (*this), line_numbers_control (*this), clipboard (*this), layout (NULL), cursor (NULL)
{
    font.m_hObject = GetStockObject (ANSI_FIXED_FONT);

    line_numbers_padding_left = GetSystemMetrics (SM_CXFRAME);
    line_numbers_minimal_digits = 5;
    line_numbers_padding_right = GetSystemMetrics (SM_CXFRAME);
    line_numbers_border_width = GetSystemMetrics (SM_CXBORDER);

    padding_left = GetSystemMetrics (SM_CXFRAME);
    padding_right = GetSystemMetrics (SM_CXFRAME);
    padding_top = 0;
    padding_bottom = 0;

    line_numbers_color = GetSysColor (COLOR_GRAYTEXT);

    CFont fnt;
    
    fnt.CreatePointFont (100, L"Consolas");
}

CEditorView::~CEditorView()
{
    if (layout != NULL)
        delete layout;

    if (cursor != NULL)
        delete cursor;
}

void CEditorView::OnDraw (CDC* pDC)
{
}

BOOL CEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszClass = AfxRegisterWndClass (0, ::LoadCursor(NULL, IDC_ARROW), GetSysColorBrush (COLOR_BTNFACE), NULL); 

    if (!CView::PreCreateWindow(cs))
        return FALSE;

    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    return TRUE;
}

void CEditorView::OnInitialUpdate ()
{
    layout = new CTabbedTextLayout (GetDocument ()->GetText (), GetDocument ()->GetUndoManager ());
    cursor = new CNormalTextCursor (*layout, GetDocument ()->GetUndoManager (), clipboard);

    CView::OnInitialUpdate ();
}

void CEditorView::OnSetFocus (CWnd* pOldWnd)
{
    editor_control.SetFocus ();
}

void CEditorView::UpdateLayout ()
{
    if (layout == NULL) return;

    CRect client_rect;
    GetClientRect (&client_rect);

    if (line_numbers_control.GetStyle () & WS_VISIBLE)
    {
        unsigned int digits = 0;
        unsigned int height = layout->GetHeight ();
        while (height > 0)
        {
            digits++;
            height /= 10;
        }

        digits = max (digits, line_numbers_minimal_digits);

        int line_numbers_control_width = 
            line_numbers_padding_left +
            cell_size.cx * digits +
            line_numbers_padding_right;

        int editor_left = line_numbers_control_width +
            line_numbers_border_width;

        CRect nr = CRect (client_rect.left + editor_left, client_rect.top, client_rect.right, client_rect.bottom);
        editor_control.SetWindowPos (NULL, nr.left, nr.top, nr.Width (), nr.Height (), SWP_NOACTIVATE | SWP_NOZORDER);

        editor_control.GetClientRect (&client_rect);

        nr = CRect (0, client_rect.top, line_numbers_control_width, client_rect.bottom);
        line_numbers_control.SetWindowPos (NULL, nr.left, nr.top, nr.Width (), nr.Height (), SWP_NOACTIVATE | SWP_NOZORDER);
            line_numbers_control.Invalidate ();
    }
    else
    {
        editor_control.SetWindowPos (
            NULL, 
            client_rect.left, client_rect.top, 
            client_rect.Width (), client_rect.Height (), SWP_NOACTIVATE);
    }
}

void CEditorView::UpdateCellSize ()
{
    CDC *pDC = GetDC ();

    pDC->SelectObject (font);

    TEXTMETRIC tm;
    pDC->GetTextMetrics (&tm);

    cell_size.cx = tm.tmAveCharWidth;
    cell_size.cy = tm.tmHeight;
}

BOOL CEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}

void CEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

#ifdef _DEBUG
void CEditorView::AssertValid() const
{
    CView::AssertValid();
}

void CEditorView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CEditorDocument* CEditorView::GetDocument() const // non-debug version is inline
{
    if (m_pDocument == NULL) return NULL;
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorDocument)));
    return (CEditorDocument*)m_pDocument;
}
#endif //_DEBUG

int CEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    editor_control.Create (NULL, NULL, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL, CRect (), this, 0);
    line_numbers_control.Create (NULL, NULL, WS_VISIBLE | WS_CHILD, CRect (), this, 0);

    CFont * font = new CFont ();
    font->CreatePointFont (100, L"Consolas");
    SetFont (font);

    UpdateCellSize ();

    return CView::OnCreate (lpCreateStruct);
}

void CEditorView::OnSize (UINT nType, int cx, int cy)
{
    UpdateLayout ();

    CView::OnSize (nType, cx, cy);
}

LRESULT CEditorView::OnSetFont (WPARAM wParam, LPARAM lParam)
{
    font.m_hObject = (HGDIOBJ)wParam;

    UpdateCellSize ();

    return 0;
}

LRESULT CEditorView::OnGetFont (WPARAM wParam, LPARAM lParam)
{
    return (LRESULT)font.m_hObject;
}

void CEditorView::OnUpdateViewLineNumbersMenu (CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck (line_numbers_control.GetStyle () & WS_VISIBLE);

    pCmdUI->ContinueRouting ();
}

BOOL CEditorView::OnViewLineNumbersCheck (UINT nID)
{
    line_numbers_control.SetWindowPos (
        NULL, 0, 0, 0, 0,
    SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | (line_numbers_control.GetStyle () & WS_VISIBLE ? SWP_HIDEWINDOW : SWP_SHOWWINDOW));

    UpdateLayout ();

    return TRUE;
}

void CEditorView::OnUpdateUndoMenu (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (GetDocument () != NULL && GetDocument ()->GetUndoManager ().IsUndoAvailable ());
}

BOOL CEditorView::OnUndo (UINT nID)
{
    GetDocument ()->GetUndoManager ().Undo ();

    editor_control.UpdateScrollBars ();
    editor_control.ValidateCursor ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    ValidateLayout ();
    editor_control.UpdateWindow ();
    line_numbers_control.UpdateWindow ();

    return TRUE;
}

void CEditorView::OnUpdateRedoMenu (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (GetDocument () != NULL && GetDocument ()->GetUndoManager ().IsRedoAvailable ());
}

BOOL CEditorView::OnRedo (UINT nID)
{
    GetDocument ()->GetUndoManager ().Redo ();

    editor_control.UpdateScrollBars ();
    editor_control.ValidateCursor ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    ValidateLayout ();
    editor_control.UpdateWindow ();
    line_numbers_control.UpdateWindow ();

    return TRUE;
}

BOOL CEditorView::OnSelectAll (UINT nID)
{
    cursor->SelectAll ();
    editor_control.UpdateScrollBars ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    editor_control.ValidateCursor ();

    return TRUE;
}

BOOL CEditorView::OnClear (UINT nID)
{
    cursor->Del ();
    editor_control.UpdateScrollBars ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    editor_control.ValidateCursor ();

    return TRUE;
}

void CEditorView::OnUpdateCopyMenu (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (cursor->CanCopy ());
}

BOOL CEditorView::OnCopy (UINT nID)
{
    cursor->Copy ();
    editor_control.UpdateScrollBars ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    editor_control.ValidateCursor ();

    return TRUE;
}

void CEditorView::OnUpdatePasteMenu (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (cursor->CanPaste ());
}

BOOL CEditorView::OnPaste (UINT nID)
{
    cursor->Paste ();
    editor_control.UpdateScrollBars ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    editor_control.ValidateCursor ();

    return TRUE;
}

void CEditorView::OnUpdateCutMenu (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (cursor->CanCopy ());
}

BOOL CEditorView::OnCut (UINT nID)
{
    cursor->Cut ();
    editor_control.UpdateScrollBars ();
    editor_control.EnsureCaretVisible ();
    editor_control.UpdateCaret ();
    editor_control.ValidateCursor ();

    return TRUE;
}

void CEditorView::OnUpdate (CView* pSender, LPARAM lHint, CObject* pHint)
{
    if (pHint != NULL)
    {
        CDocChange &change = *(CDocChange *)pHint;

        unsigned int overlap = min (change.original_lines_count, change.new_lines_count);

        if (overlap > 0)
            layout->LinesChanged (change.first_line, overlap);

        if (change.original_lines_count > overlap)
            layout->LinesRemoved (change.first_line + overlap, change.original_lines_count - overlap);

        if (change.new_lines_count > overlap)
            layout->LinesInserted (change.first_line + overlap, change.new_lines_count - overlap);

        ValidateLayout ();
    }
    else CView::OnUpdate (pSender, lHint, pHint);
}

void CEditorView::ValidateLayout ()
{
    unsigned int top = 
        padding_top + 
        layout->GetFirstDirtyRow () * cell_size.cy - 
        editor_control.GetScrollPos (SB_VERT);
    unsigned int bottom = top + layout->GetDirtyRowsCount () * cell_size.cy;

    layout->ResetDirtyRows ();

    if (top <= bottom)
    {
        CRect client_rect;

        editor_control.GetClientRect (&client_rect);
        client_rect.top = max (client_rect.top, top);
        client_rect.bottom = min (client_rect.bottom, bottom);
        editor_control.InvalidateRect (&client_rect, false);

        line_numbers_control.GetClientRect (&client_rect);
        client_rect.top = max (client_rect.top, top);
        client_rect.bottom = min (client_rect.bottom, bottom);
        line_numbers_control.InvalidateRect (&client_rect, false);
    }
}

#pragma endregion
