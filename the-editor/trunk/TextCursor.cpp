#include "StdAfx.h"
#include "TextCursor.h"

#pragma region CTextCursor

CTextCursor::~CTextCursor ()
{
}

#pragma endregion

#pragma region CNormalTextCursor

void CNormalTextCursor::AddDirtyRowRange (unsigned int start_dirty_row, unsigned int dirty_row_count)
{
    if (CNormalTextCursor::dirty_row_count == 0)
    {
        CNormalTextCursor::start_dirty_row = start_dirty_row;
        CNormalTextCursor::dirty_row_count = dirty_row_count;
    }
    else if (dirty_row_count > 0)
    {
        unsigned int s = min (CNormalTextCursor::start_dirty_row, start_dirty_row);
        unsigned int e = max (CNormalTextCursor::start_dirty_row + CNormalTextCursor::dirty_row_count, start_dirty_row + dirty_row_count);
        CNormalTextCursor::start_dirty_row = s;
        CNormalTextCursor::dirty_row_count = e - s;
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

    TEXTCELL tc;

    layout.GetCellByLinePosition (line, position, tc);

    current_row = tc.row;
    current_column = tc.column;

    if (!selecting)
    {
        anchor_line = tc.line;
        anchor_position = tc.position;
    }

    current_line = tc.line;
    current_position = tc.position;

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

CNormalTextCursor::CNormalTextCursor (CTextLayout &layout, unsigned int row, unsigned int column) : CTextCursor (layout), selection (NULL)
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

void CNormalTextCursor::Click (unsigned int row, unsigned int column, bool selecting)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    MoveToLinePosition (tc.line, tc.position, selecting);
}

void CNormalTextCursor::Drag (unsigned int row, unsigned int column)
{
    TEXTCELL tc;

    layout.GetCellAt (row, column, tc);

    MoveToLinePosition (tc.line, tc.position, true);
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

void CNormalTextCursor::WordLeft (bool selecting) {}

void CNormalTextCursor::WordRight (bool selecting) {}

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

#pragma endregion
