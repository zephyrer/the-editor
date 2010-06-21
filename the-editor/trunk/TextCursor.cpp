#include "StdAfx.h"
#include "TextCursor.h"

#pragma region CTextCursor

CTextCursor::~CTextCursor ()
{
}

#pragma endregion

#pragma region CNormalTextCursor

class CNormalTextCursorAction : public CUndoableAction
{
protected:
    CNormalTextCursor &cursor;

public:
    inline CNormalTextCursorAction (CNormalTextCursor &cursor) : cursor (cursor) {}
};

class CAddDirtyRowRangeAction : public CNormalTextCursorAction
{
protected:
    unsigned int start_dirty_row;
    unsigned int dirty_row_count;

public:
    inline CAddDirtyRowRangeAction (CNormalTextCursor &cursor, unsigned int start_dirty_row, unsigned int dirty_row_count) :
        CNormalTextCursorAction (cursor), start_dirty_row (start_dirty_row), dirty_row_count (dirty_row_count)
    {}
    
    virtual void Undo ()
    {
        cursor.AddDirtyRowRange (start_dirty_row, dirty_row_count);
    }
};

class CMoveAction : public CNormalTextCursorAction
{
protected:
    unsigned int anchor_line, anchor_position;
    unsigned int current_line, current_position;
    unsigned int current_row, current_column;
    unsigned int cursor_row, cursor_column;

public:
    inline CMoveAction (
        CNormalTextCursor &cursor, 
        unsigned int anchor_line, unsigned int anchor_position, 
        unsigned int current_line, unsigned int current_position,
        unsigned int current_row, unsigned int current_column,
        unsigned int cursor_row, unsigned int cursor_column) :
        CNormalTextCursorAction (cursor),
        anchor_line (anchor_line), anchor_position (anchor_position), 
        current_line (current_line), current_position (current_position),
        current_row (current_row), current_column (current_column),
        cursor_row (cursor_row), cursor_column (cursor_column)
    {}
        
    virtual void Undo ()
    {
        if (cursor.undo_manager.IsWithinTransaction ())
            cursor.undo_manager.AddAction (
                new CMoveAction (
                    cursor,
                    cursor.anchor_line, cursor.anchor_position,
                    cursor.current_line, cursor.current_position,
                    cursor.current_row, cursor.current_column,
                    cursor.cursor_row, cursor.cursor_column));

        cursor.anchor_line = anchor_line;
        cursor.anchor_position = anchor_position;
        cursor.current_line = current_line;
        cursor.current_position = current_position;
        cursor.current_row = current_row;
        cursor.current_column = current_column;
        cursor.cursor_row = cursor_row;
        cursor.cursor_column = cursor_column;

		cursor.UpdateSelection ();
    }
};

void CNormalTextCursor::AddDirtyRowRange (unsigned int start_dirty_row, unsigned int dirty_row_count)
{
    if (dirty_row_count > 0)
    {
        if (CNormalTextCursor::dirty_row_count == 0)
        {
            CNormalTextCursor::start_dirty_row = start_dirty_row;
            CNormalTextCursor::dirty_row_count = dirty_row_count;
        }
        else
        {
            unsigned int s = min (CNormalTextCursor::start_dirty_row, start_dirty_row);
            unsigned int e = max (CNormalTextCursor::start_dirty_row + CNormalTextCursor::dirty_row_count, start_dirty_row + dirty_row_count);
            CNormalTextCursor::start_dirty_row = s;
            CNormalTextCursor::dirty_row_count = e - s;
        }

        if (undo_manager.IsWithinTransaction ())
            undo_manager.AddAction (new CAddDirtyRowRangeAction (*this, start_dirty_row, dirty_row_count));
    }
}

void CNormalTextCursor::AddDirtyLineRange (unsigned int start_dirty_line, unsigned int dirty_line_count)
{
    ASSERT (start_dirty_line <= layout.GetText ().GetLinesCount ());
    ASSERT (start_dirty_line + dirty_line_count <= layout.GetText ().GetLinesCount ());

    if (dirty_line_count == 0) return;

    TEXTCELL tc;

    layout.GetCellByLinePosition (start_dirty_line, 0, tc);

    unsigned int start_row = tc.row;
    
    unsigned int end_dirty_line = start_dirty_line + dirty_line_count - 1;

    layout.GetCellByLinePosition (end_dirty_line, layout.GetText ().GetLineLength (end_dirty_line), tc);

    unsigned int end_row = tc.row;

    AddDirtyRowRange (start_row, end_row - start_row + 1);
}

void CNormalTextCursor::UpdateSelection ()
{
    CContinuousTextSelection *new_selection;

    if (current_line == anchor_line && current_position == anchor_position)
    {
        new_selection = NULL;
    }
    else if (current_line < anchor_line || (current_line == anchor_line && current_position < anchor_position))
        new_selection = new CContinuousTextSelection (layout, current_line, current_position, anchor_line, anchor_position);
    else 
        new_selection = new CContinuousTextSelection (layout, anchor_line, anchor_position, current_line, current_position);

    if (new_selection == NULL)
    {
        if (selection == NULL) return;

        unsigned int start_line = selection->GetStartLine ();
        unsigned int end_line = selection->GetEndLine ();

        AddDirtyLineRange (start_line, end_line - start_line + 1);
    }
    else
    {
        if (selection == NULL)
        {
            unsigned int start_line = new_selection->GetStartLine ();
            unsigned int end_line = new_selection->GetEndLine ();

            AddDirtyLineRange (start_line, end_line - start_line + 1);
        }
        else
        {
            unsigned int s = UINT_MAX;
            unsigned int e = 0;

            if (selection->GetStartLine () != new_selection->GetStartLine () ||
                selection->GetStartPosition () != new_selection->GetStartPosition ())
            {
                s = min (s, min (selection->GetStartLine (), new_selection->GetStartLine ()));
                e = max (e, max (selection->GetStartLine (), new_selection->GetStartLine ()));
            }

            if (selection->GetEndLine () != new_selection->GetEndLine () ||
                selection->GetEndPosition () != new_selection->GetEndPosition ())
            {
                s = min (s, min (selection->GetEndLine (), new_selection->GetEndLine ()));
                e = max (e, max (selection->GetEndLine (), new_selection->GetEndLine ()));
            }

            if (s <= e)
                AddDirtyLineRange (s, e - s + 1);
        }
    }

    if (selection != NULL)
        delete selection;

    selection = new_selection;
}

void CNormalTextCursor::MoveToRowColumn (unsigned int row, unsigned int column, bool selecting)
{
    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (
            new CMoveAction (
                *this,
                anchor_line, anchor_position,
                current_line, current_position,
                current_row, current_column,
                cursor_row, cursor_column));

    unsigned int height = layout.GetHeight ();
    if (row >= height)
        row = height - 1;

    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    current_row = row;
    current_column = column;

    if (!selecting)
    {
        anchor_line = tc.line;
        anchor_position = tc.position;
    }

    current_line = tc.line;
    current_position = tc.position;

    layout.GetCellByLinePosition (tc.line, tc.position, tc);

    cursor_row = tc.row;
    cursor_column = tc.column;

    UpdateSelection ();
}

void CNormalTextCursor::MoveToLinePosition (unsigned int line, unsigned int position, bool selecting)
{
    ASSERT (line < layout.GetText ().GetLinesCount ());
    ASSERT (position <= layout.GetText ().GetLineLength (line));

    if (selecting)
        MoveToLinePosition (line, position, anchor_line, anchor_position);
    else MoveToLinePosition (line, position, line, position);
}

void CNormalTextCursor::MoveToLinePosition (unsigned int line, unsigned int position, unsigned int aline, unsigned int aposition)
{
    ASSERT (line < layout.GetText ().GetLinesCount ());
    ASSERT (position <= layout.GetText ().GetLineLength (line));
    ASSERT (aline < layout.GetText ().GetLinesCount ());
    ASSERT (aposition <= layout.GetText ().GetLineLength (aline));

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (
            new CMoveAction (
                *this,
                anchor_line, anchor_position,
                current_line, current_position,
                current_row, current_column,
                cursor_row, cursor_column));

    TEXTCELL tc;

    layout.GetCellByLinePosition (line, position, tc);

    current_row = tc.row;
    current_column = tc.column;

    anchor_line = aline;
    anchor_position = aposition;

    current_line = line;
    current_position = position;

    cursor_row = tc.row;
    cursor_column = tc.column;

    UpdateSelection ();
}

unsigned int CNormalTextCursor::GetFirstNonBlankPosition (unsigned int line)
{
    ASSERT (line < layout.GetText ().GetLinesCount ());

    TCHAR buffer [256];

    unsigned int line_length = layout.GetText ().GetLineLength (line);
    unsigned int position = 0;
    while (position < line_length)
    {
        if (position % 256 == 0)
            layout.GetText ().GetCharsRange (line, position, min (256, line_length - position), buffer);

        TCHAR ch = buffer [position % 256];

        if (!_istspace (ch)) return position;

        position++;
    }

    return 0;
}

bool CNormalTextCursor::IsWordBoundary (TCHAR ch1, TCHAR ch2)
{
    if (_istalnum (ch1))
    {
        if (_istalnum (ch2)) return false;
        if (_istspace (ch2)) return false;
        return true;
    }

    if (_istspace (ch1))
    {
        if (_istspace (ch2)) return false;
        return true;
    }

    return true;
}

unsigned int CNormalTextCursor::GetPreviousWordBoundary (unsigned int line, unsigned int start)
{
    ASSERT (line < layout.GetText ().GetLinesCount ());
    ASSERT (start <= layout.GetText ().GetLineLength (line));

    if (start == 0) return start;
    unsigned int line_length = layout.GetText ().GetLineLength (line);
    if (start == line_length) return line_length;

    TCHAR buffer [256];
    TCHAR pch;

    for (unsigned int i = 0; i <= start; i++)
    {
        unsigned int position = start - i;

        if (i % 256 == 0)
        {
            layout.GetText ().GetCharsRange (
                line, 
                position >= 256 ? position - 255 : 0, 
                position >= 256 ? 256 : position + 1, 
                position >= 256 ? buffer : &buffer [255 - position]);
        }

        TCHAR ch = buffer [255 - i % 256];

        if (i > 0)
        {
            if (IsWordBoundary (ch, pch)) return position + 1;
        }

        pch = ch;
    }

    return 0;
}

unsigned int CNormalTextCursor::GetNextWordBoundary (unsigned int line, unsigned int start)
{
    ASSERT (line < layout.GetText ().GetLinesCount ());
    ASSERT (start <= layout.GetText ().GetLineLength (line));

    if (start == 0) return start;
    unsigned int line_length = layout.GetText ().GetLineLength (line);
    if (start == line_length) return line_length;

    TCHAR buffer [256];
    TCHAR pch;

    unsigned int c = line_length - start;

    for (unsigned int i = 0; i <= c; i++)
    {
        unsigned int position = start + i - 1;

        if (i % 256 == 0)
        {
            layout.GetText ().GetCharsRange (
                line,
                position,
                min (256, line_length - position), 
                buffer);
        }

        TCHAR ch = buffer [i % 256];

        if (i > 0)
        {
            if (IsWordBoundary (pch, ch)) return position;
        }

        pch = ch;
    }

    return line_length;
}

void CNormalTextCursor::DeleteSelection ()
{
    if (selection == NULL) return;

	unsigned int sline = selection->GetStartLine ();
	unsigned int sposition = selection->GetStartPosition ();
	unsigned int eline = selection->GetEndLine ();
	unsigned int eposition = selection->GetEndPosition ();

	MoveToLinePosition (sline, sposition, false);

    if (sline == eline)
    {
        layout.GetText ().ReplaceCharsRange (
            sline,
            sposition,
            eposition - sposition,
            0, NULL);

        layout.LinesChanged (sline, 1);
        AddDirtyLineRange (sline, 1);
    }
    else
    {
        unsigned int height = layout.GetHeight ();

        layout.GetText ().ReplaceCharsRange (
            sline,
            sposition,
            layout.GetText ().GetLineLength (sline) - sposition,
            0, NULL);
        layout.LinesChanged (sline, 1);

        layout.GetText ().ReplaceCharsRange (
            eline,
            0,
            eposition,
            0, NULL);
        layout.LinesChanged (eline, 1);

        for (unsigned int i = sline + 1; i < eline; i++)
            layout.GetText ().RemoveLineAt (sline + 1);
        layout.LinesRemoved (sline + 1, eline - sline - 1);

        layout.GetText ().JoinLines (sline);
        layout.LinesChanged (sline, 1);
        layout.LinesRemoved (sline + 1, 1);

        AddDirtyRowRange (0, height);
    }
}

CNormalTextCursor::CNormalTextCursor (CTextLayout &layout, unsigned int row, unsigned int column, CUndoManager &undo_manager) : 
    CTextCursor (layout), selection (NULL), undo_manager (undo_manager)
{
    unsigned int height = layout.GetHeight ();
    if (row >= height)
        row = height - 1;

    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    current_row = row;
    current_column = column;
    anchor_line = tc.line;
    anchor_position = tc.position;
    current_line = tc.line;
    current_position = tc.position;

    layout.GetCellByLinePosition (tc.line, tc.position, tc);

    cursor_row = tc.row;
    cursor_column = tc.column;
}

CNormalTextCursor::~CNormalTextCursor ()
{
    if (selection != NULL)
        delete selection;
}

unsigned int CNormalTextCursor::GetCursorRow ()
{
    return cursor_row;
}

unsigned int CNormalTextCursor::GetCursorColumn ()
{
    return cursor_column;
}

CTextSelection *CNormalTextCursor::GetSelection ()
{
    return selection;
}

unsigned int CNormalTextCursor::GetStartDirtyRow ()
{
    return start_dirty_row;
}

unsigned int CNormalTextCursor::GetDirtyRowCount ()
{
    return dirty_row_count;
}

void CNormalTextCursor::ResetDirtyRows ()
{
    start_dirty_row = 0;
    dirty_row_count = 0;
}

bool CNormalTextCursor::CanOverwrite ()
{
    return current_position < layout.GetText ().GetLineLength (current_line);
}

bool CNormalTextCursor::CanCopy ()
{
    return selection != NULL;
}

void CNormalTextCursor::Click (unsigned int row, unsigned int column, bool selecting)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    MoveToLinePosition (tc.line, tc.position, selecting);
}

void CNormalTextCursor::WordClick (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (tc.position == layout.GetText ().GetLineLength (tc.line))
    {
        if (tc.position > 0)
            MoveToLinePosition (tc.line, GetNextWordBoundary (tc.line, tc.position), tc.line, GetPreviousWordBoundary (tc.line, tc.position - 1));
        else 
            MoveToLinePosition (tc.line, tc.position, false);
    }
    else
    {
        MoveToLinePosition (tc.line, GetNextWordBoundary (tc.line, tc.position + 1), tc.line, GetPreviousWordBoundary (tc.line, tc.position));
    }
}

void CNormalTextCursor::LineClick (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (tc.line < layout.GetText ().GetLinesCount () - 1)
        MoveToLinePosition (tc.line + 1, 0, tc.line, 0);
    else MoveToLinePosition (tc.line, layout.GetText ().GetLineLength (tc.line), tc.line, 0);
}

void CNormalTextCursor::Drag (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    MoveToLinePosition (tc.line, tc.position, true);
}

void CNormalTextCursor::WordDrag (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (tc.line < anchor_line || (tc.line == anchor_line && tc.position < anchor_position))
    {
        if (current_line < anchor_line || (current_line == anchor_line && current_position < anchor_position))
        {
            MoveToLinePosition (tc.line, GetPreviousWordBoundary (tc.line, tc.position), true);
        }
        else
        {
            MoveToLinePosition (
                tc.line, GetPreviousWordBoundary (tc.line, tc.position), 
                anchor_line, GetNextWordBoundary (anchor_line, anchor_position < layout.GetText ().GetLineLength (anchor_line) ? anchor_position + 1 : anchor_position));
        }
    }
    else
    {
        if (current_line < anchor_line || (current_line == anchor_line && current_position < anchor_position))
        {
            MoveToLinePosition (
                tc.line, GetNextWordBoundary (tc.line, tc.position), 
                anchor_line, GetPreviousWordBoundary (anchor_line, anchor_position > 0 ? anchor_position - 1 : anchor_position));
        }
        else
        {
            MoveToLinePosition (tc.line, GetNextWordBoundary (tc.line, tc.position), true);
        }
    }
}

void CNormalTextCursor::LineDrag (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (tc.line < anchor_line || (tc.line == anchor_line && tc.position < anchor_position))
    {
        if (current_line < anchor_line || (current_line == anchor_line && current_position < anchor_position))
        {
            MoveToLinePosition (tc.line, 0, true);
        }
        else
        {
            MoveToLinePosition (tc.line, 0, current_line, current_position);
        }
    }
    else
    {
        unsigned int dl, dp;

        if (tc.line < layout.GetText ().GetLinesCount () - 1)
        {
            dl = tc.line + 1;
            dp = 0;
        }
        else
        {
            dl = tc.line;
            dp = layout.GetText ().GetLineLength (tc.line);
        }

        if (current_line < anchor_line || (current_line == anchor_line && current_position < anchor_position))
        {
            MoveToLinePosition (dl, dp, current_line, current_position);
        }
        else
        {
            MoveToLinePosition (dl, dp, true);
        }
    }
}

void CNormalTextCursor::Left (bool selecting)
{
    if (selection != NULL && !selecting)
    {
        MoveToLinePosition (selection->GetStartLine (), selection->GetStartPosition (), selecting);
    }
    else if (current_position > 0)
        MoveToLinePosition (current_line, current_position - 1, selecting);
    else if (current_line > 0)
    {
        MoveToLinePosition (current_line - 1, layout.GetText ().GetLineLength (current_line - 1), selecting);
    }
    else
    {
        // Do nothing
    }
}

void CNormalTextCursor::Right (bool selecting)
{
    if (selection != NULL && !selecting)
    {
        MoveToLinePosition (selection->GetEndLine (), selection->GetEndPosition (), selecting);
    }
    else if (current_position < layout.GetText ().GetLineLength (current_line))
        MoveToLinePosition (current_line, current_position + 1, selecting);
    else if (current_line < layout.GetText ().GetLinesCount () - 1)
    {
        MoveToLinePosition (current_line + 1, 0, selecting);
    }
    else
    {
        // Do nothing
    }
}

void CNormalTextCursor::Up (bool selecting)
{
    if (current_row > 0)
        MoveToRowColumn (current_row - 1, current_column, selecting);
}

void CNormalTextCursor::Down (bool selecting)
{
    MoveToRowColumn (current_row + 1, current_column, selecting);
}

void CNormalTextCursor::WordLeft (bool selecting)
{
    if (current_position == 0)
    {
        if (current_line > 0)
            MoveToLinePosition (current_line - 1, layout.GetText ().GetLineLength (current_line - 1), selecting);
    }
    else
        MoveToLinePosition (current_line, GetPreviousWordBoundary (current_line, current_position - 1), selecting);
}

void CNormalTextCursor::WordRight (bool selecting)
{
    if (current_position == layout.GetText ().GetLineLength (current_line))
    {
        if (current_line < layout.GetText ().GetLinesCount () - 1)
            MoveToLinePosition (current_line + 1, 0, selecting);
    }
    else
        MoveToLinePosition (current_line, GetNextWordBoundary (current_line, current_position + 1), selecting);
}

void CNormalTextCursor::PageUp (unsigned int page_rows, bool selecting)
{
    MoveToRowColumn (current_row >= page_rows ? current_row - page_rows : 0, current_column, selecting);
}

void CNormalTextCursor::PageDown (unsigned int page_rows, bool selecting)
{
    MoveToRowColumn (current_row + page_rows, current_column, selecting);
}

void CNormalTextCursor::Home (bool selecting)
{
    TEXTCELL row_start;

    layout.GetCellAt (cursor_row, 0, row_start);

    unsigned int position = GetFirstNonBlankPosition (current_line);

    if (current_position == 0)
        MoveToLinePosition (current_line, position, selecting);
    else
    {
        unsigned int p = 0;

        if (row_start.position < current_position)
            p = row_start.position;

        if (position > p && position < current_position)
            p = position;

        MoveToLinePosition (current_line, p, selecting);
    }
}

void CNormalTextCursor::End (bool selecting)
{
    TEXTCELL row_end;

    layout.GetCellAt (cursor_row, layout.GetWidth (), row_end);

    unsigned int line_length = layout.GetText ().GetLineLength (current_line);

    unsigned int p = line_length;

    if (row_end.position < p && row_end.position > current_position)
        p = row_end.position;

    MoveToLinePosition (current_line, p, selecting);
}

void CNormalTextCursor::TextBegin (bool selecting)
{
    MoveToLinePosition (0, 0, selecting);
}

void CNormalTextCursor::TextEnd (bool selecting)
{
    unsigned int line = layout.GetText ().GetLinesCount () - 1;
    unsigned int position = layout.GetText ().GetLineLength (line);

    MoveToLinePosition (line, position, selecting);
}

void CNormalTextCursor::SelectAll ()
{
    anchor_line = 0;
    anchor_position = 0;

    unsigned int l = layout.GetText ().GetLinesCount () - 1;
    MoveToLinePosition (l, layout.GetText ().GetLineLength (l), true);
}

void CNormalTextCursor::InsertChar (TCHAR ch)
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    layout.GetText ().InsertCharAt (current_line, current_position, ch);
    layout.LinesChanged (current_line, 1);

    MoveToLinePosition (current_line, current_position + 1, false);
    AddDirtyLineRange (current_line, 1);

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::OverwriteChar (TCHAR ch)
{
    if (current_position == layout.GetText ().GetLineLength (current_line))
        return;

    undo_manager.StartTransaction ();

    layout.GetText ().SetCharAt (current_line, current_position, ch);
    layout.LinesChanged (current_line, 1);

    MoveToLinePosition (current_line, current_position + 1, false);
    AddDirtyLineRange (current_line, 1);

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::Backspace ()
{
    undo_manager.StartTransaction ();

    if (selection != NULL) 
        DeleteSelection ();
    else
    {
        if (current_position > 0)
        {
            MoveToLinePosition (current_line, current_position - 1, false);
            layout.GetText ().RemoveCharAt (current_line, current_position);
            layout.LinesChanged (current_line, 1);
            AddDirtyLineRange (current_line, 1);
        }
        else
        {
            if (current_line > 0)
            {
                unsigned int height = layout.GetHeight ();

                unsigned int pll = layout.GetText ().GetLineLength (current_line - 1);
                MoveToLinePosition (current_line - 1, pll, false);
                layout.GetText ().JoinLines (current_line);
                layout.LinesChanged (current_line, 1);
                layout.LinesRemoved (current_line + 1, 1);
                AddDirtyRowRange (0, height);
            }
        }
    }

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::Del ()
{
    undo_manager.StartTransaction ();

    if (selection != NULL) 
        DeleteSelection ();
    else
    {
        if (current_position < layout.GetText ().GetLineLength (current_line))
        {
            layout.GetText ().RemoveCharAt (current_line, current_position);
            layout.LinesChanged (current_line, 1);
            AddDirtyLineRange (current_line, 1);
        }
        else
        {
            if (current_line < layout.GetText ().GetLinesCount () - 1)
            {
                unsigned int height = layout.GetHeight ();

                layout.GetText ().JoinLines (current_line);
                layout.LinesChanged (current_line, 1);
                layout.LinesRemoved (current_line + 1, 1);
                AddDirtyRowRange (0, height);
            }
        }
    }

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::WordBackspace ()
{
    undo_manager.StartTransaction ();

    if (selection != NULL) 
        DeleteSelection ();
    else
    {
        if (current_position > 0)
        {
            unsigned int to = GetPreviousWordBoundary (current_line, current_position - 1);
			unsigned int from = current_position;

            MoveToLinePosition (current_line, to, false);
            layout.GetText ().ReplaceCharsRange (current_line, to, from - to, 0, NULL);
            layout.LinesChanged (current_line, 1);
            AddDirtyLineRange (current_line, 1);
        }
        else
        {
            if (current_line > 0)
            {
                unsigned int height = layout.GetHeight ();

                unsigned int pll = layout.GetText ().GetLineLength (current_line - 1);
                MoveToLinePosition (current_line - 1, pll, false);
                layout.GetText ().JoinLines (current_line);
                layout.LinesChanged (current_line, 1);
                layout.LinesRemoved (current_line + 1, 1);
                AddDirtyRowRange (0, height);
            }
        }
    }

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::WordDel ()
{
    undo_manager.StartTransaction ();

    if (selection != NULL) 
        DeleteSelection ();
    else
    {
        if (current_position < layout.GetText ().GetLineLength (current_line))
        {
            layout.GetText ().ReplaceCharsRange (current_line, current_position, GetNextWordBoundary (current_line, current_position + 1) - current_position, 0, NULL);
            layout.LinesChanged (current_line, 1);
            AddDirtyLineRange (current_line, 1);
        }
        else
        {
            if (current_line < layout.GetText ().GetLinesCount () - 1)
            {
                unsigned int height = layout.GetHeight ();

                layout.GetText ().JoinLines (current_line);
                layout.LinesChanged (current_line, 1);
                layout.LinesRemoved (current_line + 1, 1);
                AddDirtyRowRange (0, height);
            }
        }
    }

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::NewLine ()
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    layout.GetText ().BreakLineAt (current_line, current_position);
    layout.LinesChanged (current_line, 1);
    layout.LinesInserted (current_line + 1, 1);
    MoveToLinePosition (current_line + 1, 0, false);
    AddDirtyLineRange (current_line - 1, layout.GetText ().GetLinesCount () - current_line + 1);

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::Copy (CWnd &owner)
{
    if (selection == NULL) return;

    unsigned int sline = selection->GetStartLine ();
    unsigned int sposition = selection->GetStartPosition ();
    unsigned int eline = selection->GetEndLine ();
    unsigned int eposition = selection->GetEndPosition ();

    unsigned int length = 0;

    TCHAR *unicodetext;

    if (sline == eline)
    {
        length = eposition - sposition;

        unicodetext = (TCHAR *)GlobalAlloc (GMEM_FIXED, (length + 1) * sizeof (TCHAR));
        layout.GetText ().GetCharsRange (sline, sposition, length, unicodetext);
        unicodetext [length] = 0;
    }
    else
    {
        length = layout.GetText ().GetLineLength (sline) - sposition + eposition + 1;
        for (unsigned int i = sline + 1; i < eline; i++)
            length += layout.GetText ().GetLineLength (i) + 1;

        unicodetext = (TCHAR *)GlobalAlloc (GMEM_FIXED, (length + 1) * sizeof (TCHAR));

        unsigned int n = 0;
        layout.GetText ().GetCharsRange (sline, sposition, layout.GetText ().GetLineLength (sline) - sposition, &unicodetext [n]);
        n += layout.GetText ().GetLineLength (sline) - sposition;
        unicodetext [n] = L'\n';
        n++;

        for (unsigned int i = sline + 1; i < eline; i++)
        {
            layout.GetText ().GetCharsRange (i, 0, layout.GetText ().GetLineLength (i), &unicodetext [n]);
            n += layout.GetText ().GetLineLength (i);
            unicodetext [n] = L'\n';
            n++;
        }

        layout.GetText ().GetCharsRange (eline, 0, eposition, &unicodetext [n]);

        unicodetext [length] = 0;
    }

    char *text = (char *)GlobalAlloc (GMEM_FIXED, (length + 1) * sizeof (char));
    WideCharToMultiByte (CP_ACP, 0, unicodetext, length, text, length + 1, "?", NULL);
    text [length] = 0;

    if (!owner.OpenClipboard ())
    {
        AfxMessageBox (_T ("Cannot open the Clipboard"));
        return;
    }

    if (!EmptyClipboard ())
    {
        AfxMessageBox (_T ("Cannot empty the Clipboard"));
        return;
    }

    if (::SetClipboardData (CF_TEXT, text) == NULL)
    {
        CString msg;
        msg.Format (_T ("Unable to set Clipboard data, error: %d"), GetLastError ());
        AfxMessageBox (msg);
        CloseClipboard ();
        GlobalFree (text);
        GlobalFree (unicodetext);
        return;
    }

    if (::SetClipboardData (CF_UNICODETEXT, unicodetext) == NULL)
    {
        CString msg;
        msg.Format (_T ("Unable to set Clipboard data, error: %d"), GetLastError ());
        AfxMessageBox (msg);
        CloseClipboard ();
        GlobalFree (text);
        GlobalFree (unicodetext);
        return;
    }

    CloseClipboard();
}

void CNormalTextCursor::Paste (CWnd &owner)
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    layout.GetText ().BreakLineAt (current_line, current_position);
    layout.LinesChanged (current_line, 1);
    layout.LinesInserted (current_line + 1, 1);
    MoveToLinePosition (current_line + 1, 0, false);
    AddDirtyLineRange (current_line - 1, layout.GetText ().GetLinesCount () - current_line + 1);

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::Cut (CWnd &owner)
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    layout.GetText ().BreakLineAt (current_line, current_position);
    layout.LinesChanged (current_line, 1);
    layout.LinesInserted (current_line + 1, 1);
    MoveToLinePosition (current_line + 1, 0, false);
    AddDirtyLineRange (current_line - 1, layout.GetText ().GetLinesCount () - current_line + 1);

    undo_manager.FinishTransaction ();
}

#pragma endregion
