#include "StdAfx.h"
#include "CharBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CCharBuffer

void CCharBuffer::FireOnChange (unsigned int start, unsigned int old_count, unsigned int new_count)
{
    ASSERT (start <= GetSize ());
    ASSERT (start + new_count <= GetSize ());

    if (listener != NULL && (old_count != 0 || new_count != 0))
        listener->OnChange (start, old_count, new_count);
}

void CCharBuffer::SetListener (CCharBufferListener *listener)
{
    ASSERT (CCharBuffer::listener == NULL);
    ASSERT (listener != NULL);

    CCharBuffer::listener = listener;
}

void CCharBuffer::SetUndoManager (CUndoManager *undo_manager)
{
    ASSERT (CCharBuffer::undo_manager == NULL);
    ASSERT (undo_manager != NULL);

    CCharBuffer::undo_manager = undo_manager;
}

CCharBuffer::~CCharBuffer(void)
{
    // Do nothing
}

#pragma endregion

#pragma region CVectorCharBuffer

class CVectorCharBufferChangeAction : public CUndoableAction
{
protected:
    CVectorCharBuffer &buffer;
    unsigned int start;
    unsigned int old_count;
    unsigned int new_count;
    TCHAR *characters;

public:
    inline CVectorCharBufferChangeAction (
        CVectorCharBuffer &buffer,
        unsigned int start,
        unsigned int old_count,
        unsigned int new_count,
        TCHAR *characters) : buffer (buffer)
    {
        CVectorCharBufferChangeAction::start = start;
        CVectorCharBufferChangeAction::old_count = old_count;
        CVectorCharBufferChangeAction::new_count = new_count;

        if (old_count > 0)
        {
            CVectorCharBufferChangeAction::characters = new TCHAR [old_count];
            memcpy (CVectorCharBufferChangeAction::characters, characters, old_count * sizeof (TCHAR));
        }
        else
        {
            CVectorCharBufferChangeAction::characters = NULL;
        }
    }

    virtual void Undo ()
    {
        buffer.ReplaceCharsRange (start, new_count, old_count, characters);
    }

    virtual ~CVectorCharBufferChangeAction ()
    {
        if (characters != NULL)
            delete [] characters;
    }
};

unsigned int CVectorCharBuffer::GetSize ()
{
    return data.size ();
}

void CVectorCharBuffer::GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer [])
{
    ASSERT (start <= data.size ());
    ASSERT (start + count <= data.size ());
    ASSERT (count == 0 || buffer != NULL);

    if (count > 0)
        memcpy (buffer, &data [start], count * sizeof (TCHAR));
}

void CVectorCharBuffer::ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
{
    ASSERT (start <= data.size ());
    ASSERT (start + count <= data.size ());
    ASSERT (replacement_length == 0 || replacement != NULL);

    bool undo = undo_manager != NULL && undo_manager->IsWithinTransaction ();

    TCHAR *characters;
    if (undo)
    {
        characters = new TCHAR [count];
        memcpy (characters, &data [start], count * sizeof (TCHAR));
    }

    if (count > replacement_length)
        data.erase (data.begin () + start + replacement_length, data.begin () + start + count);
    else if (count < replacement_length)
        data.insert (data.begin () + start + count, replacement_length - count, 0);

    if (replacement_length > 0)
        memcpy (&data [start], replacement, replacement_length * sizeof (TCHAR));

    if (undo) undo_manager->AddAction (new CVectorCharBufferChangeAction (*this, start, count, replacement_length, characters));

    FireOnChange (start, count, replacement_length);
}

CVectorCharBuffer::~CVectorCharBuffer ()
{
    // Do nothing
}

#pragma endregion
