#pragma once

#include "UndoManager.h"

class CCharBufferListener
{
public:
    virtual void OnChange (unsigned int start, unsigned int old_count, unsigned int new_count) = 0;
};

class CCharBuffer : public CObject
{
protected:
    CCharBufferListener *listener;
    CUndoManager *undo_manager;

    virtual void FireOnChange (unsigned int start, unsigned int old_count, unsigned int new_count);

public:
    inline CCharBuffer () : listener (NULL), undo_manager (NULL)
    {
        // Do nothing
    }

    virtual void SetListener (CCharBufferListener *listener);
    virtual void SetUndoManager (CUndoManager *undo_manager);

    virtual unsigned int GetSize () = 0;
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []) = 0;
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []) = 0;

    virtual ~CCharBuffer ();
};

class CVectorCharBuffer : public CCharBuffer
{
protected:
    std::vector <TCHAR> data;

public:
    virtual unsigned int GetSize ();
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []);

    virtual ~CVectorCharBuffer ();
};
