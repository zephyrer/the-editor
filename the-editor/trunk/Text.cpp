#include "StdAfx.h"
#include "Text.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
    ASSERT (replacement_length == 0 || replacement != NULL);

    unsigned int overlap = min (count, replacement_length);
    for (unsigned int i = 0; i < overlap; i++)
        SetCharAt (line, start_position + i, replacement [i]);

    if (count > replacement_length)
    {
        for (unsigned int i = overlap; i < count; i++)
            RemoveCharAt (line, start_position + overlap);
    }
    else
    {
        for (unsigned int i = overlap; i < replacement_length; i++)
            InsertCharAt (line, start_position + i, replacement [i]);
    }
}

void CAbstractText::InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters [])
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (count == 0 || length != NULL);
    ASSERT (count == 0 || characters != NULL);

    for (unsigned int i = 0; i < count; i++)
        InsertLineAt (line + i, length [i], characters [i]);
}

void CAbstractText::RemoveLinesAt (unsigned int line, unsigned int count)
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (line + count <= GetLinesCount ());

    for (unsigned int i = count; i > 0; i--)
        RemoveLineAt (line + i - 1);
}

#pragma endregion

#pragma region CSimpleInMemoryText

class CSimpleInMemoryTextAction : public CUndoableAction
{
protected:
    CSimpleInMemoryText &text;

public:
    inline CSimpleInMemoryTextAction (CSimpleInMemoryText &text) : text (text) {}
};

class CReplaceCharsRangeAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;
    unsigned int position;
    unsigned int count;
    unsigned int replacement_length;
    TCHAR *characters;

public:
    inline CReplaceCharsRangeAction (CSimpleInMemoryText &text, unsigned int line, unsigned int position, unsigned int count, unsigned int replacement_length, TCHAR characters []) :
        CSimpleInMemoryTextAction (text), line (line), position (position), count (count), replacement_length (replacement_length)
    {
        CReplaceCharsRangeAction::characters = new TCHAR [count];
        memcpy (CReplaceCharsRangeAction::characters, characters, count * sizeof (TCHAR));
    }

    virtual ~CReplaceCharsRangeAction ()
    {
        delete [] characters;
    }

    virtual void Undo ()
    {
        text.ReplaceCharsRange (line, position, replacement_length, count, characters);
    }
};

class CInsertLinesAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;
    unsigned int count;

public:
    inline CInsertLinesAction (CSimpleInMemoryText &text, unsigned int line, unsigned int count) :
        CSimpleInMemoryTextAction (text), line (line), count (count)
    {}

    virtual void Undo ()
    {
        text.RemoveLinesAt (line, count);
    }
};

class CRemoveLinesAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;
    unsigned int count;
    unsigned int *length;
    TCHAR **characters;
    TCHAR *buffer;

public:
    inline CRemoveLinesAction (CSimpleInMemoryText &text, unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters []) :
        CSimpleInMemoryTextAction (text), line (line), count (count)
    {
        CRemoveLinesAction::length = new unsigned int [count];

        unsigned int size = 0;
        for (unsigned int i = 0; i < count; i++)
        {
            CRemoveLinesAction::length [i] = length [i];
            size += length [i];
        }

        CRemoveLinesAction::characters = new TCHAR* [count];
        buffer = new TCHAR [size];
        unsigned int pos = 0;
        for (unsigned int i = 0; i < count; i++)
        {
            CRemoveLinesAction::characters [i] = &buffer [pos];
            memcpy (CRemoveLinesAction::characters [i], characters [i], length [i]);
            pos += length [i];
        }
    }

    virtual ~CRemoveLinesAction ()
    {
        delete [] length;
        delete [] buffer;
        delete [] characters;
    }

    virtual void Undo ()
    {
        text.InsertLinesAt (line, count, length, characters);
    }
};

class CJoinLinesAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;
    unsigned int position;

public:
    inline CJoinLinesAction (CSimpleInMemoryText &text, unsigned int line, unsigned int position) :
        CSimpleInMemoryTextAction (text), line (line), position (position)
    {}

    virtual void Undo ()
    {
        text.BreakLineAt (line, position);
    }
};

class CBreakLineAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;

public:
    inline CBreakLineAction (CSimpleInMemoryText &text, unsigned int line) :
        CSimpleInMemoryTextAction (text), line (line)
    {}

    virtual void Undo ()
    {
        text.JoinLines (line);
    }
};

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

void CSimpleInMemoryText::GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer [])
{
    ASSERT (line < GetLinesCount ());
    ASSERT (start_position <= GetLineLength (line));
    ASSERT (start_position + count <= GetLineLength (line));
    ASSERT (buffer != NULL);

    if (count > 0)
        memcpy (buffer, &text [line][start_position], count * sizeof (TCHAR));
}

void CSimpleInMemoryText::InsertCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position <= GetLineLength (line));

    text [line].insert (text [line].begin () + position, character);

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CReplaceCharsRangeAction (*this, line, position, 0, 1, NULL));
}

void CSimpleInMemoryText::SetCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position < GetLineLength (line));

    TCHAR old_ch = text [line][position];
    text [line][position] = character;

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CReplaceCharsRangeAction (*this, line, position, 1, 1, &old_ch));
}

void CSimpleInMemoryText::RemoveCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position < GetLineLength (line));

    TCHAR old_ch = text [line][position];
    text [line].erase (text [line].begin () + position);

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CReplaceCharsRangeAction (*this, line, position, 1, 0, &old_ch));
}

void CSimpleInMemoryText::ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
{
    ASSERT (line < GetLinesCount ());
    ASSERT (start_position <= GetLineLength (line));
    ASSERT (start_position + count <= GetLineLength (line));
    ASSERT (replacement_length == 0 || replacement != NULL);

    TCHAR *temp = (TCHAR *)alloca (count * sizeof (TCHAR));
    GetCharsRange (line, start_position, count, temp);

    unsigned int overlap = min (count, replacement_length);
    for (unsigned int i = 0; i < overlap; i++)
        text [line][start_position + i] = replacement [i];

    if (count > replacement_length)
    {
        text [line].erase (
            text [line].begin () + start_position + overlap, 
            text [line].begin () + start_position + count);
    }
    else
    {
        text [line].insert (text [line].begin () + start_position + overlap, replacement_length - overlap, 0);
        for (unsigned int i = overlap; i < replacement_length; i++)
            text [line][start_position + i] = replacement [i];
    }

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CReplaceCharsRangeAction (*this, line, start_position, count, replacement_length, temp));
}

void CSimpleInMemoryText::BreakLineAt (unsigned int line, unsigned int position)
{
    ASSERT (line < GetLinesCount ());
    ASSERT (position <= GetLineLength (line));

    vector <TCHAR> new_line (text [line].begin () + position, text [line].end ());

    text [line].erase (text [line].begin () + position, text [line].end ());

    text.insert (text.begin () + line + 1, new_line);

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CBreakLineAction (*this, line));
}

void CSimpleInMemoryText::JoinLines (unsigned int line)
{
    ASSERT (line < GetLinesCount () - 1);

    unsigned int ll = text [line].size ();

    text [line].insert (text [line].end (), text [line + 1].begin (), text [line + 1].end ());
    text.erase (text.begin () + line + 1);

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CJoinLinesAction (*this, line, ll));
}

void CSimpleInMemoryText::InsertLineAt (unsigned int line, unsigned int length, TCHAR characters [])
{
    ASSERT (line <= GetLinesCount ());

    InsertLinesAt (line, 1, &length, &characters);
}

void CSimpleInMemoryText::RemoveLineAt (unsigned int line)
{
    ASSERT (line < GetLinesCount ());

    RemoveLinesAt (line, 1);
}

void CSimpleInMemoryText::InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters [])
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (count == 0 || length != NULL);
    ASSERT (count == 0 || characters != NULL);

    if (count == 0) return;

    text.insert (text.begin () + line, count, vector <TCHAR> ());
    for (unsigned int i = 0; i < count; i++)
    {
        TCHAR *c = characters [i];
        text [line + i].resize (length [i]);
        if (length [i] > 0)
            memcpy (&text [line + i][0], c, length [i] * sizeof (TCHAR));
    }

    if (undo_manager.IsWithinTransaction ())
       undo_manager.AddAction (new CInsertLinesAction (*this, line, count));
}

void CSimpleInMemoryText::RemoveLinesAt (unsigned int line, unsigned int count)
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (line + count <= GetLinesCount ());

    unsigned int *ll = new unsigned int [count];
    TCHAR **characters = new TCHAR* [count];

    for (unsigned int i = 0; i < count; i++)
    {
        ll [i] = text [line + i].size ();;
        characters [i] = ll [i] > 0 ? &text [line + i][0] : NULL;
    }

    CRemoveLinesAction *action = NULL;

    if (undo_manager.IsWithinTransaction ())
        action = new CRemoveLinesAction (*this, line, count, ll, characters);

    delete [] ll;
    delete [] characters;

    text.erase (text.begin () + line, text.begin () + line + count);

    if (undo_manager.IsWithinTransaction ())
    {
        ASSERT (action != NULL);
        undo_manager.AddAction (action);
    }
}

#pragma endregion

#pragma region CCharBufferText

CCharBufferText::CCharBufferText (CUndoManager &undo_manager, CCharBuffer &data) : CText (undo_manager), data (data)
{
    TCHAR buffer [65536];

    unsigned int size = data.GetSize ();
    unsigned int ls = 0;
    TCHAR pch = 0;
    for (unsigned int i = 0; i < size; i++)
    {
        if (i % 65536 == 0)
            data.GetCharsRange (i, min (65536, size - i), buffer);

        TCHAR ch = buffer [i % 65536];
        if (ch == L'\n' || ch == L'\r')
        {
            if (pch == 0 || pch == ch)
            {
                line_start.push_back (ls);
                line_length.push_back (i - ls);
                pch = ch;
            }
            else
            {
                pch = 0;
            }

            ls = i;
        }
        else pch = 0;
    }
}

unsigned int CCharBufferText::GetLinesCount ()
{
    return line_length.size ();
}

unsigned int CCharBufferText::GetLineLength (unsigned int line)
{
    ASSERT (line < line_length.size ());

    return line_length [line];
}

TCHAR CCharBufferText::GetCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    return data.GetCharAt (line_start [line] + position);
}

void CCharBufferText::GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer [])
{
    ASSERT (line < line_length.size ());
    ASSERT (start_position <= line_length [line]);
    ASSERT (start_position + count <= line_length [line]);
    ASSERT (count == 0 || buffer != NULL);

    data.GetCharsRange (line_start [line] + start_position, count, buffer);
}

void CCharBufferText::InsertCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < line_length.size ());
    ASSERT (position <= line_length [line]);

    data.InsertCharAt (line_start [line] + position, character);

    line_length [line] += 1;

    unsigned int size = line_length.size ();
    for (unsigned int i = line + 1; i < size; i++)
        line_start [i] += 1;
}

void CCharBufferText::SetCharAt (unsigned int line, unsigned int position, TCHAR character)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    data.SetCharAt (line_start [line] + position, character);
}

void CCharBufferText::RemoveCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    data.RemoveCharAt (line_start [line] + position);

    line_length [line] -= 1;

    unsigned int size = line_length.size ();
    for (unsigned int i = line + 1; i < size; i++)
        line_start [i] -= 1;
}

void CCharBufferText::ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
{
    ASSERT (line < line_length.size ());
    ASSERT (start_position <= line_length [line]);
    ASSERT (start_position + count <= line_length [line]);
    ASSERT (replacement_length == 0 || replacement != NULL);

    data.ReplaceCharsRange (line_start [line] + start_position, count, replacement_length, replacement);

    int delta = replacement_length - count;

    line_length [line] += delta;

    unsigned int size = line_length.size ();
    for (unsigned int i = line + 1; i < size; i++)
        line_start [i] += delta;
}

void CCharBufferText::BreakLineAt (unsigned int line, unsigned int position)
{
    ASSERT (line < line_length.size ());
    ASSERT (position <= line_length [line]);

    data.ReplaceCharsRange (line_start [line] + position, 0, 2, L"\r\n");

    line_length.insert (line_length.begin () + line + 1, line_length [line] - position);
    line_length [line] = position;
    line_start.insert (line_start.begin () + line + 1, line_start [line] + position + 2);

    unsigned int size = line_length.size ();
    for (unsigned int i = line + 2; i < size; i++)
        line_start [i] += 2;
}

void CCharBufferText::JoinLines (unsigned int line)
{
    ASSERT (line < line_length.size () - 1);

    unsigned int delta = line_start [line + 1] - line_start [line] - line_length [line];

    data.ReplaceCharsRange (line_start [line + 1] - delta, delta, 0, NULL);

    line_length [line] += line_length [line + 1];

    line_length.erase (line_length.begin () + line + 1);
    line_start.erase (line_start.begin () + line + 1);

    unsigned int size = line_length.size ();
    for (unsigned int i = line + 1; i < size; i++)
        line_start [i] -= delta;
}

void CCharBufferText::InsertLineAt (unsigned int line, unsigned int length, TCHAR characters [])
{
    ASSERT (line <= line_length.size () - 1);

    unsigned int size = line_length.size ();
    if (size == 0)
    {
        data.ReplaceCharsRange (0, 0, length, characters);
        line_start.insert (line_start.begin (), 0);
        line_length.insert (line_length.begin (), length);
    }
    else if (line == 0)
    {
        data.ReplaceCharsRange (0, 0, length, characters);
        data.ReplaceCharsRange (0, length, 2, L"\r\n");
        line_start.insert (line_start.begin (), 0);
        line_length.insert (line_length.begin (), length);

        for (unsigned int i = 1; i <= size; i++)
            line_start [i] += length + 2;
    }
    else
    {
        data.ReplaceCharsRange (line_start [line - 1] + line_length [line - 1], 0, 2, L"\r\n");
        data.ReplaceCharsRange (line_start [line - 1] + line_length [line - 1] + 2, 0, length, characters);
        line_start.insert (line_start.begin () + line, line_start [line - 1] + line_length [line - 1] + 2);
        line_length.insert (line_length.begin () + line, length);

        for (unsigned int i = line + 1; i <= size; i++)
            line_start [i] += length + 2;
    }
}

void CCharBufferText::RemoveLineAt (unsigned int line)
{
    ASSERT (line < line_length.size ());

    unsigned int delta;

    if (line < line_length.size () - 1)
        delta = line_start [line + 1] - line_start [line];
    else
        delta = data.GetSize () - line_start [line];

    data.ReplaceCharsRange (line_start [line], delta, 0, NULL);
    line_length.erase (line_length.begin () + line);
    line_start.erase (line_start.begin () + line);

    unsigned int size = line_length.size ();
    for (unsigned int i = line; i < size; i++)
        line_start [i] -= delta;
}

void CCharBufferText::InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters [])
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (count == 0 || length != NULL);
    ASSERT (count == 0 || characters != NULL);

    for (unsigned int i = 0; i < count; i++)
        InsertLineAt (line + i, length [i], characters [i]);
}

void CCharBufferText::RemoveLinesAt (unsigned int line, unsigned int count)
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (line + count <= GetLinesCount ());

    for (unsigned int i = count; i > 0; i--)
        RemoveLineAt (line + i - 1);
}

#pragma endregion
