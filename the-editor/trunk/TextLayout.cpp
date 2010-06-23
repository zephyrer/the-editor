#include "StdAfx.h"
#include "TextLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CEditorLayout

void CEditorLayout::FireOnChange (unsigned int first_row, unsigned int old_row_count, unsigned int new_row_count, bool width_changed)
{
    if (listener != NULL && (old_row_count > 0 || new_row_count == 0 > 0 || width_changed))
        listener->OnChange (first_row, old_row_count, new_row_count, width_changed);
}

void CEditorLayout::SetListener (CEditorLayoutListener *listener)
{
    ASSERT (CEditorLayout::listener == NULL);
    ASSERT (listener != NULL);

    CEditorLayout::listener = listener;
}

#pragma endregion

#pragma region CAbstractTextEditorLayout

unsigned int CAbstractTextEditorLayout::GetRowWidth (unsigned int row)
{
    ASSERT (row < GetHeight ());

    if (row >= row_widths.size ())
        row_widths.resize (row + 1, 0);

    unsigned int w = row_widths [row];
    if (w == 0)
    {
        w = CalculateRowWidth (row);
        row_widths [row] = w;
    }

    return w;
}

unsigned int CAbstractTextEditorLayout::GetWidth ()
{
    if (width == 0)
    {
        unsigned int w = 0;

        for (unsigned int height = GetHeight (), row = 0; row < height; row++)
        {
            unsigned int row_width = GetRowWidth (row);
            w = max (w, row_width);
        }

        width = w;
    }

    return width;
}

unsigned int CAbstractTextEditorLayout::GetHeight ()
{
    if (height == 0) 
        height = CalculateHeight ();

    return height;
}

void CAbstractTextEditorLayout::GetCellAt (unsigned int row, unsigned int column, TEXTCELL &text_cell)
{
    GetCellsRange (row, column, 1, 1, &text_cell);
}

void CAbstractTextEditorLayout::RowsChanged (unsigned int start_row, unsigned int count)
{
    ASSERT (start_row <= GetHeight ());
    ASSERT (start_row + count <= GetHeight ());

    if (count > 0)
    {
        if (start_row <= row_widths.size ())
        {
            unsigned int mc = min (count, row_widths.size () - start_row);
            for (unsigned int r = 0; r < mc; r++)
                row_widths [start_row + r] = 0;

            width = 0;
        }

        FireOnChange (start_row, count, count, true);
    }
}

void CAbstractTextEditorLayout::RowsInserted (unsigned int start_row, unsigned int count)
{
    ASSERT (start_row <= height);

    if (count > 0)
    {
        height += count;

        if (start_row <= row_widths.size ())
        {
            row_widths.insert (row_widths.begin () + start_row, count, 0);

            width = 0;
        }

        FireOnChange (start_row, 0, count, true);
    }
}

void CAbstractTextEditorLayout::RowsRemoved (unsigned int start_row, unsigned int count)
{
    ASSERT (start_row <= height);
    ASSERT (start_row + count <= height);

    if (count > 0)
    {
        height -= count;

        if (start_row <= row_widths.size ())
        {
            unsigned int mw = 0;
            unsigned int mc = min (count, row_widths.size () - start_row);
            for (unsigned int r = 0; r < mc; r++)
                mw = max (mw, row_widths [start_row]);

            row_widths.erase (row_widths.begin () + start_row, row_widths.begin () + start_row + mc);

            if (width == mw)
                width = 0;
        }

        FireOnChange (start_row, count, 0, true);
    }
}

#pragma endregion

#pragma region CAbstractNonWrappingTextEditorLayout

unsigned int CAbstractNonWrappingTextEditorLayout::CalculateHeight ()
{
    return text.GetLinesCount ();
}

void CAbstractNonWrappingTextEditorLayout::GetCellsRange (unsigned int start_row, unsigned int start_column, unsigned int row_count, unsigned int column_count, TEXTCELL buffer [])
{
    ASSERT (buffer != NULL);

    unsigned int height = GetHeight ();
    unsigned int lines_count = text.GetLinesCount ();
    unsigned int last_line_length = text.GetLineLength (lines_count - 1);

    for (unsigned int r = 0; r < row_count; r++)
    {
        unsigned int row = start_row + r;
        unsigned int line = row;

        if (row < height)
        {
            unsigned int row_width = GetRowWidth (row);
            unsigned int line_length = text.GetLineLength (line);

            if (start_column < row_width)
                RenderRow (row, start_column, min (column_count, row_width - start_column), &buffer [r * column_count]);

            if (start_column + column_count > row_width)
            {
                unsigned int sc;

                if (start_column < row_width)
                    sc = row_width - start_column;
                else sc = 0;

                for (unsigned int c = sc; c < column_count; c++)
                {
                    int column = start_column + c;

                    TEXTCELL &text_cell = buffer [r * column_count + c];

                    text_cell.flags = 0;
                    text_cell.character = 0;
                    text_cell.line = line;
                    text_cell.position = line_length;
                    text_cell.row = row;
                    text_cell.column = column;
                }
            }
        }
        else
        {
                for (unsigned int c = 0; c < column_count; c++)
                {
                    int column = start_column + c;

                    TEXTCELL &text_cell = buffer [r * column_count + c];

                    text_cell.flags = 0;
                    text_cell.character = 0;
                    text_cell.line = lines_count - 1;
                    text_cell.position = last_line_length;
                    text_cell.row = row;
                    text_cell.column = column;
                }
        }
    }
}

void CAbstractNonWrappingTextEditorLayout::LinesChanged (unsigned int start_line, unsigned int count)
{
    ASSERT (start_line <= text.GetLinesCount ());
    ASSERT (start_line + count <= text.GetLinesCount ());

    if (count > 0) RowsChanged (start_line, count);
}

void CAbstractNonWrappingTextEditorLayout::LinesInserted (unsigned int start_line, unsigned int count)
{
    ASSERT (start_line <= text.GetLinesCount ());

    if (count > 0) RowsInserted (start_line, count);
}

void CAbstractNonWrappingTextEditorLayout::LinesRemoved (unsigned int start_line, unsigned int count)
{
    ASSERT (start_line <= text.GetLinesCount ());

    if (count > 0) RowsRemoved (start_line, count);
}

#pragma endregion

#pragma region CSimpleTextEditorLayout

unsigned int CSimpleTextEditorLayout::CalculateRowWidth (unsigned int row)
{
    ASSERT (row < text.GetLinesCount ());

    return text.GetLineLength (row) + 1;
}

void CSimpleTextEditorLayout::RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer [])
{
    ASSERT (row < GetHeight ());
    ASSERT (start_column <= GetRowWidth (row));
    ASSERT (start_column + count <= GetRowWidth (row));
    ASSERT (buffer != NULL);

    unsigned int line_length = text.GetLineLength (row);
    unsigned int mc = min (count, line_length - start_column);
    TCHAR *temp = (TCHAR *)alloca (mc * sizeof (TCHAR));

    for (unsigned int c = 0; c < mc; c++)
    {
        TEXTCELL &text_cell = buffer [c];
        unsigned int column = start_column + c;

        if (column == line_length)
        {
            if (row == text.GetLinesCount () - 1)
                text_cell.flags = TCF_SELECTABLE | TCF_EOF;
            else
                text_cell.flags = TCF_SELECTABLE | TCF_EOL;

            text_cell.character = 0;
        }
        else
        {
            text_cell.flags = TCF_PRINTABLE | TCF_SELECTABLE;
            text_cell.character = temp [c];
        }

        text_cell.line = row;
        text_cell.position = column;
        text_cell.row = row;
        text_cell.column = column;
    }
}

void CSimpleTextEditorLayout::GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell)
{
    ASSERT (line < text.GetLinesCount ());
    ASSERT (position <= text.GetLineLength (line));

    GetCellAt (line, position, text_cell);
}

#pragma endregion

#pragma region CTabbedTextEditorLayout

unsigned int CTabbedTextEditorLayout::CalculateRowWidth (unsigned int row)
{
    ASSERT (row < GetHeight ());

    int line_length = text.GetLineLength (row);

    TCHAR *temp = (TCHAR *)alloca (line_length * sizeof (TCHAR));

    text.GetCharsRange (row, 0, line_length, temp);

    int n = 0;
    for (int position = 0; position <= line_length; position++)
    {
        if (position == line_length)
            n++;
        else if (temp [position] != L'\t')
            n++;
        else n += tab_size - n % tab_size;
    }

    return n;
}

void CTabbedTextEditorLayout::RenderRow (unsigned int row, unsigned int start_column, unsigned int count, TEXTCELL buffer [])
{
    ASSERT (row < GetHeight ());
    ASSERT (start_column <= GetRowWidth (row));
    ASSERT (start_column + count <= GetRowWidth (row));
    ASSERT (buffer != NULL);

    unsigned int line = row;
    unsigned int line_length = text.GetLineLength (line);
    unsigned int mc = min (start_column + count, line_length);

    TCHAR *temp = (TCHAR *)alloca (mc * sizeof (TCHAR));
    text.GetCharsRange (row, 0, mc, temp);

    unsigned int column = 0;
    for (unsigned int position = 0; position <= mc; position++)
    {
        if (position == line_length)
        {
            if (column >= start_column)
            {
                TEXTCELL &text_cell = buffer [column - start_column];

                if (row == text.GetLinesCount () - 1)
                    text_cell.flags = TCF_SELECTABLE | TCF_EOF;
                else
                    text_cell.flags = TCF_SELECTABLE | TCF_EOL;

                text_cell.character = 0;
                text_cell.line = row;
                text_cell.position = position;
                text_cell.row = row;
                text_cell.column = column;
            }

            column++;
        }
        else if (temp [position] != L'\t')
        {
            if (column >= start_column)
            {
                TEXTCELL &text_cell = buffer [column - start_column];

                text_cell.flags = TCF_PRINTABLE | TCF_SELECTABLE;

                text_cell.character = temp [position];
                text_cell.line = row;
                text_cell.position = position;
                text_cell.row = row;
                text_cell.column = column;
            }

            column++;
        }
        else
        {
            unsigned int oc = column;
            unsigned int nc = column + tab_size - column % tab_size;

            while (column < nc)
            {
                if (column >= start_column)
                {
                    TEXTCELL &text_cell = buffer [column - start_column];

                    text_cell.flags = TCF_SELECTABLE | TCF_TAB | (column == oc ? TCF_TABSTART : 0) | (column == nc - 1 ? TCF_TABEND : 0);

                    text_cell.character = 0;
                    text_cell.line = row;
                    text_cell.position = position;
                    text_cell.row = row;
                    text_cell.column = column;
                }

                column++;

                if (column >= start_column + count)
                    break;
            }
        }

        if (column >= start_column + count)
            break;
    }
}

CTabbedTextEditorLayout::CTabbedTextEditorLayout (CText &text, unsigned int tab_size) : CAbstractNonWrappingTextEditorLayout (text)
{
    ASSERT (tab_size > 0);

    CTabbedTextEditorLayout::tab_size = tab_size;
}

void CTabbedTextEditorLayout::SetTabSize (unsigned int tab_size)
{
    ASSERT (tab_size > 0);

    CTabbedTextEditorLayout::tab_size = tab_size;

    width = -1;
    row_widths.clear ();
}

void CTabbedTextEditorLayout::GetCellByLinePosition (unsigned int line, unsigned int position, TEXTCELL &text_cell)
{
    ASSERT (line < text.GetLinesCount ());
    ASSERT (position <= text.GetLineLength (line));

    unsigned int mp = min (position + 1, text.GetLineLength (line));

    TCHAR *temp = (TCHAR *)alloca (mp * sizeof (TCHAR));
    text.GetCharsRange (line, 0, mp, temp);

    int column = 0;
    for (unsigned int i = 0; i < position; i++)
    {
        if (temp [i] != L'\t')
            column++;
        else
            column += tab_size - column % tab_size;
    }

    if (position == text.GetLineLength (line))
    {
        if (line == text.GetLinesCount () - 1)
            text_cell.flags = TCF_SELECTABLE | TCF_EOF;
        else
            text_cell.flags = TCF_SELECTABLE | TCF_EOL;

        text_cell.character = 0;
    }
    else
    {
        TCHAR ch = temp [position];

        if (ch != L'\t')
        {
            text_cell.flags = TCF_PRINTABLE | TCF_SELECTABLE;
            text_cell.character = temp [position];
        }
        else if ((position + 1) % tab_size != 0)
        {
            text_cell.flags = TCF_SELECTABLE | TCF_TAB | TCF_TABSTART;
            text_cell.character = 0;
        }
        else
        {
            text_cell.flags = TCF_SELECTABLE | TCF_TAB | TCF_TABSTART | TCF_TABEND;
            text_cell.character = 0;
        }
    }

    text_cell.line = line;
    text_cell.position = position;
    text_cell.row = line;
    text_cell.column = column;
}

#pragma endregion
