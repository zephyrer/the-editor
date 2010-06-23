#pragma once

#include "afx.h"

#include "UndoManager.h"
#include "CharBuffer.h"

class CTextListener
{
public:
    virtual void OnChange (
        unsigned int start_line, unsigned int start_position, 
        unsigned int old_end_line, unsigned int old_end_position, 
        unsigned int new_end_line, unsigned int new_end_position) = 0;
};

class CText : public CObject
{
protected:
    CTextListener *listener;
    CUndoManager *undo_manager;

    virtual void FireOnChange (
        unsigned int start_line, unsigned int start_position, 
        unsigned int old_end_line, unsigned int old_end_position, 
        unsigned int new_end_line, unsigned int new_end_position);

public:
    inline CText () : listener (NULL), undo_manager (NULL)
    {
        // Do nothing
    }

    virtual void SetListener (CTextListener *listener);
    virtual void SetUndoManager (CUndoManager *undo_manager);

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
    virtual void InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters []) = 0;
    virtual void RemoveLinesAt (unsigned int line, unsigned int count) = 0;
};

class CCharBufferText : public CText, CCharBufferListener
{
protected:
    CCharBuffer &data;

    unsigned int lines_count;
    std::vector <unsigned int> line_start;
    std::vector <unsigned int> line_length;

    virtual unsigned int GetLastLineEndingBefore (unsigned int position);
    virtual unsigned int GetFirstLineStartingAfter (unsigned int position);
    virtual void SplitIntoLines (unsigned int start, unsigned int count, std::vector <unsigned int> &starts, std::vector <unsigned int> &lengths);
    virtual void ReplaceRange (std::vector <unsigned int> &v, unsigned int start, unsigned int count, std::vector <unsigned int> &replacement, unsigned int replacement_start, unsigned int replacement_count);

public:
    CCharBufferText (CCharBuffer &data);

    virtual unsigned int GetLinesCount ();
    virtual unsigned int GetLineLength (unsigned int line);
    virtual TCHAR GetCharAt (unsigned int line, unsigned int position);
    virtual void GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer []);
    virtual void InsertCharAt (unsigned int line, unsigned int position, TCHAR character);
    virtual void SetCharAt (unsigned int line, unsigned int position, TCHAR character);
    virtual void RemoveCharAt (unsigned int line, unsigned int position);
    virtual void ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement []);
    virtual void BreakLineAt (unsigned int line, unsigned int position);
    virtual void JoinLines (unsigned int line);
    virtual void InsertLineAt (unsigned int line, unsigned int length, TCHAR characters []);
    virtual void RemoveLineAt (unsigned int line);
    virtual void InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], TCHAR *characters []);
    virtual void RemoveLinesAt (unsigned int line, unsigned int count);

    virtual void OnChange (unsigned int start, unsigned int old_count, unsigned int new_count);
};
