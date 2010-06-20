#pragma once

#include "afx.h"

#include "UndoManager.h"

using namespace std;

class CText : public CObject
{
protected:
    CUndoManager &undo_manager;

public:
    inline CText (CUndoManager &undo_manager) : undo_manager (undo_manager) {}

    virtual unsigned int GetLinesCount () = 0;
    virtual unsigned int GetLineLength (unsigned int line) = 0;
    virtual TCHAR GetCharAt (unsigned int line, unsigned int position) = 0;
    virtual void GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer []) = 0;
    virtual void InsertCharAt (unsigned int line, unsigned int position, TCHAR character) = 0;
    virtual void SetCharAt (unsigned int line, unsigned int position, TCHAR character) = 0;
    virtual void RemoveCharAt (unsigned int line, unsigned int position) = 0;
    virtual void ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement []) = 0;
    virtual void BreakLineAt (unsigned int line, unsigned int position) = 0;
    virtual void JoinLines (unsigned int line) = 0;
    virtual void InsertLineAt (unsigned int line, unsigned int length, TCHAR characters []) = 0;
    virtual void RemoveLineAt (unsigned int line) = 0;
};

class CAbstractText :
    public CText
{
public:
    inline CAbstractText (CUndoManager &undo_manager) : CText (undo_manager) {}

    virtual void GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer []);
    virtual void ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement []);
};

class CSimpleInMemoryText :
    public CAbstractText
{
protected:
    vector <vector <TCHAR>> text;

public:
    inline CSimpleInMemoryText (CUndoManager &undo_manager) : CAbstractText (undo_manager) {}

    virtual unsigned int GetLinesCount ();
    virtual unsigned int GetLineLength (unsigned int line);
    virtual TCHAR GetCharAt (unsigned int line, unsigned int position);
    virtual void InsertCharAt (unsigned int line, unsigned int position, TCHAR character);
    virtual void SetCharAt (unsigned int line, unsigned int position, TCHAR character);
    virtual void RemoveCharAt (unsigned int line, unsigned int position);
    virtual void BreakLineAt (unsigned int line, unsigned int position);
    virtual void JoinLines (unsigned int line);
    virtual void InsertLineAt (unsigned int line, unsigned int length, TCHAR characters []);
    virtual void RemoveLineAt (unsigned int line);
};
