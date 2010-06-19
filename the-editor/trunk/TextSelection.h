#pragma once

#include "TextLayout.h"

class CTextSelection : public CObject
{
protected:
    CTextLayout &layout;

public:
    inline CTextSelection (CTextLayout &layout) : layout (layout) {}

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
    CContinuousTextSelection (CTextLayout &layout, unsigned int start_line, unsigned int start_position, unsigned int end_line, unsigned int end_position);

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
    inline CBlockTextSelection (CTextLayout &layout, unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count) :
        CTextSelection (layout), start_row (start_row), start_column (start_column), row_count (row_count), column_count (column_count) {}

    virtual bool IsCellSelected (TEXTCELL &text_cell);
};
