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

class CInsertLineAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;

public:
    inline CInsertLineAction (CSimpleInMemoryText &text, unsigned int line) :
        CSimpleInMemoryTextAction (text), line (line)
    {}

    virtual void Undo ()
    {
        text.RemoveLineAt (line);
    }
};

class CRemoveLineAction : public CSimpleInMemoryTextAction
{
protected:
    unsigned int line;
    unsigned int length;
    TCHAR *characters;

public:
    inline CRemoveLineAction (CSimpleInMemoryText &text, unsigned int line, unsigned int length, TCHAR characters []) :
        CSimpleInMemoryTextAction (text), line (line), length (length)
    {
        CRemoveLineAction::characters = new TCHAR [length];
        memcpy (CRemoveLineAction::characters, characters, length * sizeof (TCHAR));
    }

    virtual ~CRemoveLineAction ()
    {
        delete [] characters;
    }

    virtual void Undo ()
    {
        text.InsertLineAt (line, length, characters);
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

    text.insert (text.begin () + line, vector <TCHAR> (&characters [0], &characters [length]));

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CInsertLineAction (*this, line));
}

void CSimpleInMemoryText::RemoveLineAt (unsigned int line)
{
    ASSERT (line >= 0);
    ASSERT (line < GetLinesCount ());

    unsigned int ll = GetLineLength (line);
    TCHAR *characters = (TCHAR *)alloca (ll * sizeof (TCHAR));
    GetCharsRange (line, 0, ll, characters);

    text.erase (text.begin () + line);

    if (undo_manager.IsWithinTransaction ())
        undo_manager.AddAction (new CRemoveLineAction (*this, line, ll, characters));
}

#pragma endregion
