#pragma once

#include "StdAfx.h"

#include "Text.h"

#pragma region TCF_XXXX

#define TCF_PRINTABLE           0x00000001u
#define TCF_SELECTABLE          0x00000002u
#define TCF_TAB                 0x00000004u
#define TCF_TABSTART            0x00000008u
#define TCF_TABEND              0x00000010u
#define TCF_EOL                 0x00000020u
#define TCF_EOF                 0x00000040u

#pragma endregion

struct TEXTCELL
{
    unsigned int flags;
    TCHAR character;
    unsigned int line;
    unsigned int position;
    unsigned int row;
    unsigned int column;
};

class CTextLayout : public CObject
{
protected:
    CText &text;

public:
    inline CTextLayout (CText &text) : text (text) {}
    inline CText & GetText () { return text; }

    virtual unsigned int GetWidth () = 0;
    virtual unsigned int GetHeight () = 0;
    virtual void GetCellAt (unsigned int row, unsigned int column, TEXTCELL &text_cell) = 0;
    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell) = 0;
    virtual void GetCellsRange (unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count, TEXTCELL buffer []) = 0;
    virtual void LinesChanged (unsigned int start_line, unsigned int count) = 0;
    virtual void LinesInserted (unsigned int index, unsigned int count) = 0;
    virtual void LinesRemoved (unsigned int start_line, unsigned int count) = 0;
};

class CAbstractTextLayout : public CTextLayout
{
protected:
    unsigned int width;
    vector <unsigned int> row_widths;

    virtual unsigned int GetRowWidth (unsigned int row);
    virtual unsigned int CalculateRowWidth (unsigned int row) = 0;

public:
    inline CAbstractTextLayout (CText &text) : CTextLayout (text), width (0) {}

    virtual unsigned int GetWidth ();
    virtual void GetCellAt (unsigned int row, unsigned int column, TEXTCELL &text_cell);
    virtual void RowsChanged (unsigned int start_row, unsigned int count);
    virtual void RowsInserted (unsigned int start_row, unsigned int count);
    virtual void RowsRemoved (unsigned int start_row, unsigned int count);
};

class CAbstractNonWrappingTextLayout : public CAbstractTextLayout
{
protected:
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []) = 0;

public:
    inline CAbstractNonWrappingTextLayout (CText &text) : CAbstractTextLayout (text) {}

    virtual unsigned int GetHeight ();
    virtual void GetCellsRange (unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count, TEXTCELL buffer []);
    virtual void LinesChanged (unsigned int start_line, unsigned int count);
    virtual void LinesInserted (unsigned int index, unsigned int count);
    virtual void LinesRemoved (unsigned int start_line, unsigned int count);
};

class CSimpleTextLayout : public CAbstractNonWrappingTextLayout
{
protected:
    virtual unsigned int CalculateRowWidth (unsigned int row);
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []);

public:
    inline CSimpleTextLayout (CText &text) : CAbstractNonWrappingTextLayout (text) {}

    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell);
};

class CTabbedTextLayout : public CAbstractNonWrappingTextLayout
{
protected:
    unsigned int tab_size;

    virtual unsigned int CalculateRowWidth (unsigned int row);
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []);

public:
    inline CTabbedTextLayout (CText &text) : CAbstractNonWrappingTextLayout (text), tab_size (8) {}
    CTabbedTextLayout (CText &text, unsigned int tab_size);

    inline unsigned int GetTabSize () { return tab_size; }
    virtual void SetTabSize (unsigned int tab_size);

    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell);
};
