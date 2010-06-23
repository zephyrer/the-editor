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

class CEditorLayoutListener
{
public:
    virtual void OnChange (unsigned int first_row, unsigned int old_row_count, unsigned int new_row_count, bool width_changed) = 0;
};

class CEditorLayout : public CObject
{
protected:
    CEditorLayoutListener *listener;

    virtual void FireOnChange (unsigned int first_row, unsigned int old_row_count, unsigned int new_row_count, bool width_changed);

public:
    inline CEditorLayout () : listener (NULL) 
    {
        // Do nothing
    }

    virtual void SetListener (CEditorLayoutListener *listener);

    virtual unsigned int GetWidth () = 0;
    virtual unsigned int GetHeight () = 0;
    virtual void GetCellAt (unsigned int row, unsigned int column, TEXTCELL &text_cell) = 0;
    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell) = 0;
    virtual void GetCellsRange (unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count, TEXTCELL buffer []) = 0;
    virtual void LinesChanged (unsigned int start_line, unsigned int count) = 0;
    virtual void LinesInserted (unsigned int index, unsigned int count) = 0;
    virtual void LinesRemoved (unsigned int start_line, unsigned int count) = 0;
};

class CAbstractTextEditorLayout : public CEditorLayout
{
protected:
    CText &text;

    unsigned int width;
    unsigned int height;

    vector <unsigned int> row_widths;

    virtual unsigned int GetRowWidth (unsigned int row);

    virtual unsigned int CalculateRowWidth (unsigned int row) = 0;
    virtual unsigned int CalculateHeight () = 0;

public:
    inline CAbstractTextEditorLayout (CText &text) : 
        text (text), width (0), height (0)
    {}

    virtual unsigned int GetWidth ();
    virtual unsigned int GetHeight ();
    virtual void GetCellAt (unsigned int row, unsigned int column, TEXTCELL &text_cell);
    virtual void RowsChanged (unsigned int start_row, unsigned int count);
    virtual void RowsInserted (unsigned int start_row, unsigned int count);
    virtual void RowsRemoved (unsigned int start_row, unsigned int count);
};

class CAbstractNonWrappingTextEditorLayout : public CAbstractTextEditorLayout
{
protected:
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []) = 0;
    virtual unsigned int CalculateHeight ();

public:
    inline CAbstractNonWrappingTextEditorLayout (CText &text) : CAbstractTextEditorLayout (text) {}

    virtual void GetCellsRange (unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count, TEXTCELL buffer []);
    virtual void LinesChanged (unsigned int start_line, unsigned int count);
    virtual void LinesInserted (unsigned int index, unsigned int count);
    virtual void LinesRemoved (unsigned int start_line, unsigned int count);
};

class CSimpleTextEditorLayout : public CAbstractNonWrappingTextEditorLayout
{
protected:
    virtual unsigned int CalculateRowWidth (unsigned int row);
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []);

public:
    inline CSimpleTextEditorLayout (CText &text) : CAbstractNonWrappingTextEditorLayout (text) {}

    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell);
};

class CTabbedTextEditorLayout : public CAbstractNonWrappingTextEditorLayout
{
protected:
    unsigned int tab_size;

    virtual unsigned int CalculateRowWidth (unsigned int row);
    virtual void RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer []);

public:
    inline CTabbedTextEditorLayout (CText &text) : CAbstractNonWrappingTextEditorLayout (text), tab_size (8) {}
    CTabbedTextEditorLayout (CText &text, unsigned int tab_size);

    inline unsigned int GetTabSize () { return tab_size; }
    virtual void SetTabSize (unsigned int tab_size);

    virtual void GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell);
};
