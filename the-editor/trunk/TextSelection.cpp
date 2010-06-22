#include "StdAfx.h"
#include "TextSelection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CContinuousTextSelection

CContinuousTextSelection::CContinuousTextSelection (CTextLayout &layout, unsigned int start_line, unsigned int start_position, unsigned int end_line, unsigned int end_position) :
    CTextSelection (layout)
{
    ASSERT (start_line < layout.GetText ().GetLinesCount ());
    ASSERT (start_position <= layout.GetText ().GetLineLength (start_line));
    ASSERT (end_line < layout.GetText ().GetLinesCount ());
    ASSERT (end_position <= layout.GetText ().GetLineLength (end_line));
    ASSERT (start_line < end_line || (start_line == end_line && start_position <= end_position));

    CContinuousTextSelection::start_line = start_line;
    CContinuousTextSelection::start_position = start_position;
    CContinuousTextSelection::end_line = end_line;
    CContinuousTextSelection::end_position = end_position;
}

unsigned int CContinuousTextSelection::GetStartLine ()
{
    return start_line;
}

unsigned int CContinuousTextSelection::GetStartPosition ()
{
    return start_position;
}

unsigned int CContinuousTextSelection::GetEndLine ()
{
    return end_line;
}

unsigned int CContinuousTextSelection::GetEndPosition ()
{
    return end_position;
}

bool CContinuousTextSelection::IsCellSelected (TEXTCELL &text_cell)
{
    return 
        (text_cell.line > start_line || (text_cell.line == start_line && text_cell.position >= start_position)) &&
        (text_cell.line < end_line || (text_cell.line == end_line && text_cell.position < end_position)) &&
        ((text_cell.flags & TCF_SELECTABLE) != 0);
}

#pragma endregion

#pragma region CBlockTextSelection

bool CBlockTextSelection::IsCellSelected (TEXTCELL &text_cell)
{
    return text_cell.row >= start_row && text_cell.row < start_row + row_count && text_cell.column >= start_column && text_cell.column < start_column + column_count;
}

#pragma endregion
