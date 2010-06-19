#include "StdAfx.h"

#include "Text.h"

#pragma region CAbstractText

void CAbstractText::GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer [])
{
    ASSERT (line < GetLinesCount ());
    ASSERT (start_position <= GetLineLength (line));
    ASSERT (start_position + count <= GetLineLength (line));
    ASSERT (buffer != NULL);

    for (unsigned int i = 0; i < count; i++)
        buffer [i] = GetCharAt (line, start_position + i);
}

void CAbstractText::ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
{
    ASSERT (line < GetLinesCount ());
    ASSERT (start_position <= GetLineLength (line));
    ASSERT (start_position + count <= GetLineLength (line));
    ASSERT (replacement != NULL);

    unsigned int overlap = min (count, replacement_length);
    for (unsigned int i = 0; i < overlap; i++)
        SetCharAt (line, start_position + i, replacement [i]);

    if (count > replacement_length)
    {
        for (unsigned int i = overlap; i < count; i++)
            RemoveCharAt (line, overlap);
    }
    else
    {
        for (unsigned int i = overlap; i < replacement_length; i++)
            InsertCharAt (line, i, replacement [i]);
    }
}

#pragma endregion

#pragma region CSimpleInMemoryText

unsigned int CSimpleInMemoryText::GetLinesCount ()
{
    return text.size ();
}

unsigned int CSimpleInMemoryText::GetLineLength (unsigned int line)
{
    ASSERT (line < GetLinesCount ());

    return text [line].size ();
}

TCHAR CSimpleInMemoryText::GetCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position < GetLineLength (line));

    return text [line][position];
}

void CSimpleInMemoryText::InsertCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position <= GetLineLength (line));

    text [line].insert (text [line].begin () + position, character);
}

void CSimpleInMemoryText::SetCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position < GetLineLength (line));

    text [line][position] = character;
}

void CSimpleInMemoryText::RemoveCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position < GetLineLength (line));

    text [line].erase (text [line].begin () + position);
}

void CSimpleInMemoryText::BreakLineAt (unsigned int line, unsigned int position)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position <= GetLineLength (line));

    vector <TCHAR> new_line (text [line].begin () + position, text [line].end ());

    text [line].erase (text [line].begin () + position, text [line].end ());

    text.insert (text.begin () + line + 1, new_line);
}

void CSimpleInMemoryText::JoinLines (unsigned int line)
{
    ASSERT (line < GetLinesCount () - 1);

    text [line].insert (text [line].end (), text [line + 1].begin (), text [line + 1].end ());
    text.erase (text.begin () + line + 1);
}

void CSimpleInMemoryText::InsertLineAt (unsigned int line, unsigned int length, TCHAR characters [])
{
    ASSERT (line <= GetLinesCount ());

    text.insert (text.begin () + line, vector <TCHAR> (&characters [0], &characters [length]));
}

void CSimpleInMemoryText::RemoveLineAt (unsigned int line)
{
    ASSERT (line >= 0);
    ASSERT (line < GetLinesCount ());

    text.erase (text.begin () + line);
}

#pragma endregion
