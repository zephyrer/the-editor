#pragma once

#include "TextLayout.h"

class CTextSelection : public CObject
{
protected:
    CEditorLayout &layout;

public:
    inline CTextSelection (CEditorLayout &layout) : layout (layout) {}

    virtual bool IsCellSelected (TEXTCELL &text_cell) = 0;
};

class CContinuousTextSelection : public CTextSelection
{
protected:
    unsigned int start_line;
    unsigned int start_position;
    unsigned int end_line;
    unsigned int end_position;

public:
    CContinuousTextSelection (CEditorLayout &layout, unsigned int start_line, unsigned int start_position, unsigned int end_line, unsigned int end_position);

    inline CContinuousTextSelection (CContinuousTextSelection &selection) :
        CTextSelection (selection.layout), start_line (selection.start_line), start_position (selection.start_position), end_line (selection.end_line), end_position (selection.end_position)
    {}

    virtual unsigned int GetStartLine ();
    virtual unsigned int GetStartPosition ();
    virtual unsigned int GetEndLine ();
    virtual unsigned int GetEndPosition ();

    virtual bool IsCellSelected (TEXTCELL &text_cell);
};

class CBlockTextSelection : public CTextSelection
{
protected:
    unsigned int start_row;
    unsigned int start_column;
    unsigned int row_count;
    unsigned int column_count;

public:
    inline CBlockTextSelection (CEditorLayout &layout, unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count) :
        CTextSelection (layout), start_row (start_row), start_column (start_column), row_count (row_count), column_count (column_count) {}

    virtual bool IsCellSelected (TEXTCELL &text_cell);
};
