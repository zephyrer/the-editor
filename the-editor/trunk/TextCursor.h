#pragma once

#include "TextLayout.h"
#include "TextSelection.h"

class CTextCursor : public CObject
{
protected:
    CTextLayout &layout;

public:
    inline CTextCursor (CTextLayout &layout) : layout (layout) {}

    virtual ~CTextCursor ();

    virtual unsigned int GetCursorRow () = 0;
    virtual unsigned int GetCursorColumn () = 0;
    virtual CTextSelection *GetSelection () = 0;
    virtual unsigned int GetStartDirtyRow () = 0;
    virtual unsigned int GetDirtyRowCount () = 0;
    virtual void ResetDirtyRows () = 0;

    virtual void Click (unsigned int row, unsigned int column, bool selecting) = 0;
    virtual void WordClick (unsigned int row, unsigned int column) = 0;
    virtual void LineClick (unsigned int row, unsigned int column) = 0;
    virtual void Drag (unsigned int row, unsigned int column) = 0;
    virtual void WordDrag (unsigned int row, unsigned int column) = 0;
    virtual void LineDrag (unsigned int row, unsigned int column) = 0;
    virtual void Left (bool selecting) = 0;
    virtual void Right (bool selecting) = 0;
    virtual void Up (bool selecting) = 0;
    virtual void Down (bool selecting) = 0;
    virtual void WordLeft (bool selecting) = 0;
    virtual void WordRight (bool selecting) = 0;
    virtual void PageUp (unsigned int page_rows, bool selecting) = 0;
    virtual void PageDown (unsigned int page_rows, bool selecting) = 0;
    virtual void Home (bool selecting) = 0;
    virtual void End (bool selecting) = 0;
    virtual void TextBegin (bool selecting) = 0;
    virtual void TextEnd (bool selecting) = 0;

    virtual void InsertChar (TCHAR ch) = 0;
    virtual void Backspace () = 0;
    virtual void Del () = 0;
    virtual void WordBackspace () = 0;
    virtual void WordDel () = 0;
    virtual void NewLine () = 0;
};

class CNormalTextCursorAction;
class CAddDirtyRowRangeAction;
class CMoveAction;

class CNormalTextCursor : public CTextCursor
{
    friend CNormalTextCursorAction;
    friend CAddDirtyRowRangeAction;
    friend CMoveAction;

protected:
    unsigned int anchor_line, anchor_position;
    unsigned int current_line, current_position;
    unsigned int current_row, current_column;
    unsigned int cursor_row, cursor_column;
    unsigned int start_dirty_row, dirty_row_count;

    CContinuousTextSelection *selection;

    CUndoManager &undo_manager;

    virtual void AddDirtyRowRange (unsigned int start_dirty_row, unsigned int dirty_row_count);
    virtual void AddDirtyLineRange (unsigned int start_dirty_line, unsigned int dirty_line_count);
    virtual void UpdateSelection ();
    virtual void MoveToRowColumn (unsigned int row, unsigned int column, bool selecting);
    virtual void MoveToLinePosition (unsigned int line, unsigned int position, bool selecting);
    virtual void MoveToLinePosition (unsigned int line, unsigned int position, unsigned int aline, unsigned int aposition);
    virtual unsigned int GetFirstNonBlankPosition (unsigned int line);
    virtual bool IsWordBoundary (TCHAR ch1, TCHAR ch2);
    virtual unsigned int GetPreviousWordBoundary (unsigned int line, unsigned int start);
    virtual unsigned int GetNextWordBoundary (unsigned int line, unsigned int start);

    virtual void DeleteSelection ();

public:
    inline CNormalTextCursor (CTextLayout &layout, CUndoManager &undo_manager) : 
        CTextCursor (layout), 
        anchor_line (0), anchor_position (0),
        current_line (0), current_position (0),
        current_row (0), current_column (0),
        cursor_row (0), cursor_column (0),
        start_dirty_row (0), dirty_row_count (0),
        selection (NULL),
        undo_manager (undo_manager)
    {}
    
    virtual ~CNormalTextCursor ();

    CNormalTextCursor (CTextLayout &layout, unsigned int row, unsigned int column, CUndoManager &undo_manager);

    virtual unsigned int GetCursorRow ();
    virtual unsigned int GetCursorColumn ();
    virtual CTextSelection *GetSelection ();
    virtual unsigned int GetStartDirtyRow ();
    virtual unsigned int GetDirtyRowCount ();
    virtual void ResetDirtyRows ();

    virtual void Click (unsigned int row, unsigned int column, bool selecting);
    virtual void WordClick (unsigned int row, unsigned int column);
    virtual void LineClick (unsigned int row, unsigned int column);
    virtual void Drag (unsigned int row, unsigned int column);
    virtual void WordDrag (unsigned int row, unsigned int column);
    virtual void LineDrag (unsigned int row, unsigned int column);
    virtual void Left (bool selecting);
    virtual void Right (bool selecting);
    virtual void Up (bool selecting);
    virtual void Down (bool selecting);
    virtual void WordLeft (bool selecting);
    virtual void WordRight (bool selecting);
    virtual void PageUp (unsigned int page_rows, bool selecting);
    virtual void PageDown (unsigned int page_rows, bool selecting);
    virtual void Home (bool selecting);
    virtual void End (bool selecting);
    virtual void TextBegin (bool selecting);
    virtual void TextEnd (bool selecting);

    virtual void InsertChar (TCHAR ch);
    virtual void Backspace ();
    virtual void Del ();
    virtual void WordBackspace ();
    virtual void WordDel ();
    virtual void NewLine ();
};
