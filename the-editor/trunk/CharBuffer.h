#pragma once

#include "UndoManager.h"

typedef unsigned int UNICHAR;

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
    virtual void GetCharsRange (unsigned int start, unsigned int count, UNICHAR buffer []) = 0;
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, UNICHAR replacement []) = 0;

    virtual bool Load (LPCTSTR file_name) = 0;
    virtual bool Save (LPCTSTR file_name) = 0;

    virtual ~CCharBuffer ();
};

class CUndoableCharBuffer : public CCharBuffer
{
protected:
    CUndoManager *undo_manager;

public:
    inline CUndoableCharBuffer () : undo_manager (NULL)
    {
        // Do nothing
    }

    virtual void SetUndoManager (CUndoManager *undo_manager);
};

class CVectorCharBuffer : public CUndoableCharBuffer
{
protected:
    unsigned int code_page;
    std::vector <UNICHAR> *data;

public:
    inline CVectorCharBuffer (unsigned int code_page) : code_page (code_page), data (NULL)
    {
        data = new std::vector <UNICHAR> ();
    }

    virtual unsigned int GetSize ();
    virtual void GetCharsRange (unsigned int start, unsigned int count, UNICHAR buffer []);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, UNICHAR replacement []);

    virtual bool Load (LPCTSTR file_name);
    virtual bool Save (LPCTSTR file_name);

    virtual ~CVectorCharBuffer ();
};

class CVector8BitCharBuffer : public CUndoableCharBuffer
{
protected:
    unsigned int code_page;
    std::vector <char> *data;

public:
    inline CVector8BitCharBuffer (unsigned int code_page) : code_page (code_page), data (NULL)
    {
        data = new std::vector <char> ();
    }

    virtual unsigned int GetSize ();
    virtual void GetCharsRange (unsigned int start, unsigned int count, UNICHAR buffer []);
    virtual void Get8BitCharsRange (unsigned int start, unsigned int count, char buffer []);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, UNICHAR replacement []);
    virtual void Replace8BitCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, char replacement []);

    virtual bool Load (LPCTSTR file_name);
    virtual bool Save (LPCTSTR file_name);

    virtual ~CVector8BitCharBuffer ();
};
