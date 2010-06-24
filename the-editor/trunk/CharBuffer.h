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

    virtual void FireOnChange (unsigned int start, unsigned int old_count, unsigned int new_count);

public:
    inline CCharBuffer () : listener (NULL)
    {
        // Do nothing
    }

    virtual void SetListener (CCharBufferListener *listener);

    virtual unsigned int GetSize () = 0;
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []) = 0;
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []) = 0;

    virtual ~CCharBuffer ();
};

class CVectorCharBuffer : public CCharBuffer
{
protected:
    std::vector <TCHAR> data;
    CUndoManager *undo_manager;

public:
    inline CVectorCharBuffer () : undo_manager (NULL)
    {
        // Do nothing
    }

    virtual void SetUndoManager (CUndoManager *undo_manager);

    virtual unsigned int GetSize ();
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []);

    virtual ~CVectorCharBuffer ();
};

class CVector8BitCharBuffer : public CCharBuffer
{
protected:
    std::vector <char> data;
    CUndoManager *undo_manager;
    unsigned int code_page;

public:
    CVector8BitCharBuffer (unsigned int code_page, unsigned int count = 0, char *buffer = NULL);

    virtual std::vector <char> &GetData ();

    virtual unsigned int GetSize ();
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []);

    virtual ~CVector8BitCharBuffer ();
};
