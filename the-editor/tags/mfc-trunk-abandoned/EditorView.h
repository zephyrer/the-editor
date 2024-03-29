#pragma once

#include "TextLayout.h"
#include "TextCursor.h"
#include "Clipboard.h"

class CEditorView;

class CEditorControl : public CWnd
{
protected:
    CEditorView &view;

    afx_msg void OnSize (UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg void OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSetFocus (CWnd* pOldWnd);
    afx_msg void OnKillFocus (CWnd* pNewWnd);
    afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
    afx_msg void OnMouseMove (UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel (UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnContextMenu (CWnd* pWnd, CPoint pos);

    virtual void ClientToRowColumn (int x, int y, unsigned int &row, unsigned int &column);
    virtual void PaintCharsRange (CDC &DC, int row, int start_column, int count, bool selected, TEXTCELL *cells = NULL);

    unsigned int click_counter;
    unsigned int last_click_time;
    CPoint last_click_point;
    unsigned int drag_type;

    bool overwrite_mode;

    DECLARE_MESSAGE_MAP ()

public:
    inline CEditorControl (CEditorView &view) : view (view), click_counter (0), last_click_time (0), drag_type (0), overwrite_mode (false) {}

    virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
    virtual void UpdateScrollBars ();
    virtual void UpdateCaret ();
    virtual void Scroll (int dx, int dy);
    virtual void EnsureRectVisible (CRect &rect, CRect &margins);
    virtual void EnsureCaretVisible ();
};

class CLineNumbersControl : public CWnd
{
protected:
    CEditorView &view;

    afx_msg void OnPaint ();

    DECLARE_MESSAGE_MAP ()

public:
    inline CLineNumbersControl (CEditorView &view) : view (view) {}

    virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
};

class CEditorView : public CView, CEditorLayoutListener, CTextCursorListener
{
    friend CEditorControl;
    friend CLineNumbersControl;

protected:
    CFont font;

    CSize cell_size;

    CEditorControl editor_control;
    CLineNumbersControl line_numbers_control;
    CClipboard clipboard;

    unsigned int line_numbers_padding_left, line_numbers_padding_right, line_numbers_minimal_digits, line_numbers_border_width;
    COLORREF line_numbers_color;

    unsigned int padding_left, padding_right, padding_top, padding_bottom;

    CEditorLayout *layout;
    CTextCursor *cursor;

    CEditorView ();
    DECLARE_DYNCREATE (CEditorView)

    virtual void OnDraw(CDC* pDC);
    virtual BOOL OnPreparePrinting (CPrintInfo* pInfo);
    virtual void OnBeginPrinting (CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting (CDC* pDC, CPrintInfo* pInfo);
    afx_msg void OnSetFocus (CWnd* pOldWnd);

    afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize (UINT nType, int cx, int cy);
    afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
    afx_msg LRESULT OnGetFont (WPARAM, LPARAM);

    afx_msg void OnUpdateViewLineNumbersMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnViewLineNumbersCheck (UINT nID);
    afx_msg void OnUpdateUndoMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnUndo (UINT nID);
    afx_msg void OnUpdateRedoMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnRedo (UINT nID);
    afx_msg BOOL OnSelectAll (UINT nID);
    afx_msg BOOL OnClear (UINT nID);


    afx_msg void OnUpdateCopyMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnCopy (UINT nID);

    afx_msg void OnUpdatePasteMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnPaste (UINT nID);

    afx_msg void OnUpdateCutMenu (CCmdUI* pCmdUI);
    afx_msg BOOL OnCut (UINT nID);

    DECLARE_MESSAGE_MAP ()

public:
    CEditorDocument* GetDocument () const;

    virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
    virtual void OnInitialUpdate ();

    virtual void UpdateLayout ();
    virtual void UpdateCellSize ();

    virtual ~CEditorView ();

    virtual void OnUpdate (CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnChange (unsigned int first_row, unsigned int old_row_count, unsigned int new_row_count, bool width_changed);
    virtual void OnChange (unsigned int start_dirty_row, unsigned int dirty_row_count, bool caret_moved);

#ifdef _DEBUG
    virtual void AssertValid () const;
    virtual void Dump (CDumpContext& dc) const;
#endif
};

#ifndef _DEBUG  // debug version in EditorView.cpp
inline CEditorDocument* CEditorView::GetDocument() const
   { return reinterpret_cast<CEditorDocument*>(m_pDocument); }
#endif
