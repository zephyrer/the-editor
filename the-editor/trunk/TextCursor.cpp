#include "StdAfx.h"

#include "TextCursor.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
    ASSERT (start_dirty_line <= text.GetLinesCount ());
    ASSERT (start_dirty_line + dirty_line_count <= text.GetLinesCount ());

    if (dirty_line_count == 0) return;

    TEXTCELL tc;

    layout.GetCellByLinePosition (start_dirty_line, 0, tc);

    unsigned int start_row = tc.row;
    
    unsigned int end_dirty_line = start_dirty_line + dirty_line_count - 1;

    layout.GetCellByLinePosition (end_dirty_line, text.GetLineLength (end_dirty_line), tc);

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
    ASSERT (line < text.GetLinesCount ());
    ASSERT (position <= text.GetLineLength (line));

    if (selecting)
        MoveToLinePosition (line, position, anchor_line, anchor_position);
    else MoveToLinePosition (line, position, line, position);
}

void CNormalTextCursor::MoveToLinePosition (unsigned int line, unsigned int position, unsigned int aline, unsigned int aposition)
{
    ASSERT (line < text.GetLinesCount ());
    ASSERT (position <= text.GetLineLength (line));
    ASSERT (aline < text.GetLinesCount ());
    ASSERT (aposition <= text.GetLineLength (aline));

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
    ASSERT (line < text.GetLinesCount ());

    TCHAR buffer [256];

    unsigned int line_length = text.GetLineLength (line);
    unsigned int position = 0;
    while (position < line_length)
    {
        if (position % 256 == 0)
            text.GetCharsRange (line, position, min (256, line_length - position), buffer);

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
    ASSERT (line < text.GetLinesCount ());
    ASSERT (start <= text.GetLineLength (line));

    if (start == 0) return start;
    unsigned int line_length = text.GetLineLength (line);
    if (start == line_length) return line_length;

    TCHAR buffer [256];
    TCHAR pch;

    for (unsigned int i = 0; i <= start; i++)
    {
        unsigned int position = start - i;

        if (i % 256 == 0)
        {
            text.GetCharsRange (
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
    ASSERT (line < text.GetLinesCount ());
    ASSERT (start <= text.GetLineLength (line));

    if (start == 0) return start;
    unsigned int line_length = text.GetLineLength (line);
    if (start == line_length) return line_length;

    TCHAR buffer [256];
    TCHAR pch;

    unsigned int c = line_length - start;

    for (unsigned int i = 0; i <= c; i++)
    {
        unsigned int position = start + i - 1;

        if (i % 256 == 0)
        {
            text.GetCharsRange (
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
        text.ReplaceCharsRange (
            sline,
            sposition,
            eposition - sposition,
            0, NULL);
    }
    else
    {
        unsigned int height = layout.GetHeight ();

        text.ReplaceCharsRange (
            sline,
            sposition,
            text.GetLineLength (sline) - sposition,
            0, NULL);

        text.ReplaceCharsRange (
            eline,
            0,
            eposition,
            0, NULL);

        text.RemoveLinesAt (sline + 1, eline - sline - 1);

        text.JoinLines (sline);
    }
}

CNormalTextCursor::CNormalTextCursor (CText &text, CEditorLayout &layout, unsigned int row, unsigned int column, CUndoManager &undo_manager, CClipboard &clipboard) : 
    CTextCursor (text, layout), selection (NULL), undo_manager (undo_manager), clipboard (clipboard)
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
    return current_position < text.GetLineLength (current_line);
}

bool CNormalTextCursor::CanCopy ()
{
    return selection != NULL;
}

bool CNormalTextCursor::CanPaste ()
{
    return clipboard.HasText ();
}

void CNormalTextCursor::Click (unsigned int row, unsigned int column, bool selecting)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    MoveToLinePosition (tc.line, tc.position, selecting);
}

void CNormalTextCursor::RightClick (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (selection == NULL || !selection->IsCellSelected (tc))
        MoveToLinePosition (tc.line, tc.position, false);
}

void CNormalTextCursor::WordClick (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    if (tc.position == text.GetLineLength (tc.line))
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

    if (tc.line < text.GetLinesCount () - 1)
        MoveToLinePosition (tc.line + 1, 0, tc.line, 0);
    else MoveToLinePosition (tc.line, text.GetLineLength (tc.line), tc.line, 0);
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
                anchor_line, GetNextWordBoundary (anchor_line, anchor_position < text.GetLineLength (anchor_line) ? anchor_position + 1 : anchor_position));
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

        if (tc.line < text.GetLinesCount () - 1)
        {
            dl = tc.line + 1;
            dp = 0;
        }
        else
        {
            dl = tc.line;
            dp = text.GetLineLength (tc.line);
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
        MoveToLinePosition (current_line - 1, text.GetLineLength (current_line - 1), selecting);
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
    else if (current_position < text.GetLineLength (current_line))
        MoveToLinePosition (current_line, current_position + 1, selecting);
    else if (current_line < text.GetLinesCount () - 1)
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
            MoveToLinePosition (current_line - 1, text.GetLineLength (current_line - 1), selecting);
    }
    else
        MoveToLinePosition (current_line, GetPreviousWordBoundary (current_line, current_position - 1), selecting);
}

void CNormalTextCursor::WordRight (bool selecting)
{
    if (current_position == text.GetLineLength (current_line))
    {
        if (current_line < text.GetLinesCount () - 1)
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

    unsigned int line_length = text.GetLineLength (current_line);

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
    unsigned int line = text.GetLinesCount () - 1;
    unsigned int position = text.GetLineLength (line);

    MoveToLinePosition (line, position, selecting);
}

void CNormalTextCursor::SelectAll ()
{
    anchor_line = 0;
    anchor_position = 0;

    unsigned int l = text.GetLinesCount () - 1;
    MoveToLinePosition (l, text.GetLineLength (l), true);
}

void CNormalTextCursor::InsertChar (TCHAR ch)
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    text.InsertCharAt (current_line, current_position, ch);

    MoveToLinePosition (current_line, current_position + 1, false);

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::OverwriteChar (TCHAR ch)
{
    if (current_position == text.GetLineLength (current_line))
        return;

    undo_manager.StartTransaction ();

    text.SetCharAt (current_line, current_position, ch);

    MoveToLinePosition (current_line, current_position + 1, false);

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
            text.RemoveCharAt (current_line, current_position);
        }
        else
        {
            if (current_line > 0)
            {
                unsigned int height = layout.GetHeight ();

                unsigned int pll = text.GetLineLength (current_line - 1);
                MoveToLinePosition (current_line - 1, pll, false);
                text.JoinLines (current_line);
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
        if (current_position < text.GetLineLength (current_line))
        {
            text.RemoveCharAt (current_line, current_position);
        }
        else
        {
            if (current_line < text.GetLinesCount () - 1)
            {
                unsigned int height = layout.GetHeight ();

                text.JoinLines (current_line);
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
            text.ReplaceCharsRange (current_line, to, from - to, 0, NULL);
        }
        else
        {
            if (current_line > 0)
            {
                unsigned int height = layout.GetHeight ();

                unsigned int pll = text.GetLineLength (current_line - 1);
                MoveToLinePosition (current_line - 1, pll, false);
                text.JoinLines (current_line);
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
        if (current_position < text.GetLineLength (current_line))
        {
            text.ReplaceCharsRange (current_line, current_position, GetNextWordBoundary (current_line, current_position + 1) - current_position, 0, NULL);
        }
        else
        {
            if (current_line < text.GetLinesCount () - 1)
            {
                unsigned int height = layout.GetHeight ();

                text.JoinLines (current_line);
            }
        }
    }

    undo_manager.FinishTransaction ();
}

void CNormalTextCursor::NewLine ()
{
    undo_manager.StartTransaction ();

    DeleteSelection ();

    text.BreakLineAt (current_line, current_position);
    MoveToLinePosition (current_line + 1, 0, false);

    undo_manager.FinishTransaction ();
}

bool CNormalTextCursor::Copy ()
{
    if (selection == NULL) return false;

    unsigned int sline = selection->GetStartLine ();
    unsigned int sposition = selection->GetStartPosition ();
    unsigned int eline = selection->GetEndLine ();
    unsigned int eposition = selection->GetEndPosition ();

    unsigned int length = 0;

    TCHAR *unicodetext;

    if (sline == eline)
    {
        length = eposition - sposition;

        unicodetext = new TCHAR [length + 1];
        text.GetCharsRange (sline, sposition, length, unicodetext);
        unicodetext [length] = 0;
    }
    else
    {
        length = text.GetLineLength (sline) - sposition + eposition + 2;
        for (unsigned int i = sline + 1; i < eline; i++)
            length += text.GetLineLength (i) + 2;

        unicodetext = new TCHAR [length + 1];

        unsigned int n = 0;
        text.GetCharsRange (sline, sposition, text.GetLineLength (sline) - sposition, &unicodetext [n]);
        n += text.GetLineLength (sline) - sposition;
        unicodetext [n] = L'\r';
        n++;
        unicodetext [n] = L'\n';
        n++;

        for (unsigned int i = sline + 1; i < eline; i++)
        {
            text.GetCharsRange (i, 0, text.GetLineLength (i), &unicodetext [n]);
            n += text.GetLineLength (i);
            unicodetext [n] = L'\r';
            n++;
            unicodetext [n] = L'\n';
            n++;
        }

        text.GetCharsRange (eline, 0, eposition, &unicodetext [n]);

        unicodetext [length] = 0;
    }

    bool result = clipboard.SetText (unicodetext);

    delete [] unicodetext;

    return result;
}

void CNormalTextCursor::Paste ()
{
    TCHAR *text = clipboard.GetText ();
    if (text != NULL)
    {
        undo_manager.StartTransaction ();

        DeleteSelection ();

        vector <int> length;
        vector <TCHAR *> characters;

        unsigned int line_start = 0;
        unsigned int i;
        for (i = 0; text [i] != 0; i++)
        {
            if (text [i] == L'\n' || text [i] == L'\r')
            {
                length.push_back (i - line_start);
                characters.push_back (&text [line_start]);
                if ((text [i + 1] == L'\n' || text [i + 1] == L'\r') && (text [i + 1] != text [i])) i++;
                line_start = i + 1;
            }
        }
        length.push_back (i - line_start);
        characters.push_back (&text [line_start]);

        unsigned int line = current_line;

        CNormalTextCursor::text.ReplaceCharsRange (line, current_position, 0, length [0], characters [0]);

        unsigned int cnt = length.size ();
        if (cnt > 1)
        {
            CNormalTextCursor::text.BreakLineAt (line, current_position + length [0]);
            line++;

            unsigned int *pl = new unsigned int [cnt - 2];
            TCHAR **pc = new TCHAR * [cnt - 2];
            for (i = 0; i < cnt - 2; i++)
            {
                pl [i] = length [i + 1];
                pc [i] = characters [i + 1];
            }

            CNormalTextCursor::text.InsertLinesAt (line, cnt - 2, pl, pc);
            delete [] pl;
            delete [] pc;
            line += cnt - 2;

            CNormalTextCursor::text.ReplaceCharsRange (line, 0, 0, length [cnt - 1], characters [cnt - 1]);
            MoveToLinePosition (line, length [cnt - 1], false);
        }
        else MoveToLinePosition (line, current_position + length [0], false);

        undo_manager.FinishTransaction ();

        delete [] text;
    }
}

void CNormalTextCursor::Cut ()
{
    if (selection == NULL) return;

    undo_manager.StartTransaction ();

    if (Copy ()) DeleteSelection ();

    undo_manager.FinishTransaction ();
}

#pragma endregion
