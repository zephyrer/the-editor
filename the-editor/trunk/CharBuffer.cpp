#include "StdAfx.h"
#include "CharBuffer.h"
#include "FileHelper.h"

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

CCharBuffer::~CCharBuffer(void)
{
    // Do nothing
}

#pragma endregion

#pragma region CUndoableCharBuffer

void CUndoableCharBuffer::SetUndoManager (CUndoManager *undo_manager)
{
    ASSERT (CUndoableCharBuffer::undo_manager == NULL);
    ASSERT (undo_manager != NULL);

    CUndoableCharBuffer::undo_manager = undo_manager;
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
    UNICHAR *characters;

public:
    inline CVectorCharBufferChangeAction (
        CVectorCharBuffer &buffer,
        unsigned int start,
        unsigned int old_count,
        unsigned int new_count,
        UNICHAR *characters) : buffer (buffer), start (start), old_count (old_count), new_count (new_count)
    {
        if (old_count > 0)
        {
            CVectorCharBufferChangeAction::characters = new UNICHAR [old_count];
            memcpy (CVectorCharBufferChangeAction::characters, characters, old_count * sizeof (UNICHAR));
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
    ASSERT (data != NULL);

    return (unsigned int)data->size ();
}

void CVectorCharBuffer::GetCharsRange (unsigned int start, unsigned int count, UNICHAR buffer [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());
    ASSERT (count == 0 || buffer != NULL);

    if (count > 0)
        memcpy (buffer, &((*data) [start]), count * sizeof (UNICHAR));
}

void CVectorCharBuffer::ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, UNICHAR replacement [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());
    ASSERT (replacement_length == 0 || replacement != NULL);

    bool undo = undo_manager != NULL && undo_manager->IsWithinTransaction ();

    UNICHAR *characters = NULL;
    if (undo && count > 0)
    {
        characters = new UNICHAR [count];
        memcpy (characters, &(*data) [start], count * sizeof (UNICHAR));
    }

    if (count > replacement_length)
        data->erase (data->begin () + start + replacement_length, data->begin () + start + count);
    else if (count < replacement_length)
        data->insert (data->begin () + start + count, replacement_length - count, 0);

    if (replacement_length > 0)
        memcpy (&(*data) [start], replacement, replacement_length * sizeof (UNICHAR));

    if (undo) undo_manager->AddAction (new CVectorCharBufferChangeAction (*this, start, count, replacement_length, characters));

    if (characters != NULL) delete [] characters;

    FireOnChange (start, count, replacement_length);
}

bool CVectorCharBuffer::Load (LPCTSTR file_name)
{
    /*
    ASSERT (data != NULL);
    ASSERT (file_name != NULL);

    std::vector <char> d;

    if (!CFileHelper::LoadFile (file_name, d))
        return false;
    else
    {
        std::vector <UNICHAR> *ud = new std::vector <UNICHAR> ();

        if (d.size () > 0)
        {
            unsigned int l = MultiByteToWideChar (code_page, 0, &d [0], d.size (), NULL, 0);

            if (l > 0)
            {
                ud->resize (l);
                MultiByteToWideChar (code_page, 0, &d [0], d.size (), &(*ud) [0], l);
            }
        }

        unsigned int old_length = data->size ();
        unsigned int new_length = ud->size ();

        delete data;
        data = ud;

        FireOnChange (0, old_length, new_length);

        return true;
    }
    */

    return true;
}

bool CVectorCharBuffer::Save (LPCTSTR file_name)
{
    /*
    ASSERT (data != NULL);
    ASSERT (file_name != NULL);

    std::vector <char> d;

    if (data->size () > 0)
    {
        unsigned int l = WideCharToMultiByte (code_page, 0, &(*data) [0], data->size (), NULL, 0, NULL, NULL);

        if (l > 0)
        {
            d.resize (l);

            WideCharToMultiByte (code_page, 0, &(*data) [0], data->size (), & d [0], l, NULL, NULL);
        }
    }

    return CFileHelper::SaveFile (file_name, d);
    */

    return true;
}

CVectorCharBuffer::~CVectorCharBuffer ()
{
    if (data != NULL) delete data;
}

#pragma endregion

#pragma region CVector8BitCharBuffer

class CVector8BitCharBufferChangeAction : public CUndoableAction
{
protected:
    CVector8BitCharBuffer &buffer;
    unsigned int start;
    unsigned int old_count;
    unsigned int new_count;
    char *characters;

public:
    inline CVector8BitCharBufferChangeAction (
        CVector8BitCharBuffer &buffer,
        unsigned int start,
        unsigned int old_count,
        unsigned int new_count,
        char *characters) : buffer (buffer), start (start), old_count (old_count), new_count (new_count)
    {
        if (old_count > 0)
        {
            CVector8BitCharBufferChangeAction::characters = new char [old_count];
            memcpy (CVector8BitCharBufferChangeAction::characters, characters, old_count * sizeof (char));
        }
        else
        {
            CVector8BitCharBufferChangeAction::characters = NULL;
        }
    }

    virtual void Undo ()
    {
        buffer.Replace8BitCharsRange (start, new_count, old_count, characters);
    }

    virtual ~CVector8BitCharBufferChangeAction ()
    {
        if (characters != NULL)
            delete [] characters;
    }
};

unsigned int CVector8BitCharBuffer::GetSize ()
{
    ASSERT (data != NULL);

    return (unsigned int)data->size ();
}

void CVector8BitCharBuffer::GetCharsRange (unsigned int start, unsigned int count, UNICHAR buffer [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());

    if (count > 0)
    {
        TCHAR * b = (TCHAR *)alloca (count * sizeof (TCHAR));

        int c = MultiByteToWideChar (code_page, 0, &(*data) [start], count, b, count);
        ASSERT (c == count);

        for (unsigned int i = 0; i < count; i++)
            buffer [i] = b [i];
    }
}

void CVector8BitCharBuffer::Get8BitCharsRange (unsigned int start, unsigned int count, char buffer [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());

    if (count > 0)
        memcpy (buffer, &(*data) [start], count * sizeof (char));
}

void CVector8BitCharBuffer::ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, UNICHAR replacement [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());
    ASSERT (replacement_length == 0 || replacement != NULL);

    bool undo = undo_manager != NULL && undo_manager->IsWithinTransaction ();

    char *characters = NULL;
    if (undo && count > 0)
    {
        characters = new char [count];
        memcpy (characters, &(*data) [start], count * sizeof (char));
    }

    if (count > replacement_length)
        data->erase (data->begin () + start + replacement_length, data->begin () + start + count);
    else if (count < replacement_length)
        data->insert (data->begin () + start + count, replacement_length - count, 0);

    if (replacement_length > 0)
    {
        TCHAR *b = (TCHAR *)alloca (replacement_length * sizeof (TCHAR));
        for (unsigned int i = 0; i < replacement_length; i++)
            b [i] = replacement [i];

        int c = WideCharToMultiByte (code_page, 0, b, replacement_length, &(*data) [start], replacement_length, "?", NULL);

        ASSERT (c == replacement_length);
    }

    if (undo) 
        undo_manager->AddAction (
            new CVector8BitCharBufferChangeAction (*this, start, count, replacement_length, characters));

    if (characters != NULL) delete characters;

    FireOnChange (start, count, replacement_length);
}

void CVector8BitCharBuffer::Replace8BitCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, char replacement [])
{
    ASSERT (data != NULL);
    ASSERT (start <= data->size ());
    ASSERT (start + count <= data->size ());
    ASSERT (replacement_length == 0 || replacement != NULL);

    bool undo = undo_manager != NULL && undo_manager->IsWithinTransaction ();

    char *characters = NULL;
    if (undo && count > 0)
    {
        characters = new char [count];
        memcpy (characters, &(*data) [start], count * sizeof (char));
    }

    if (count > replacement_length)
        data->erase (data->begin () + start + replacement_length, data->begin () + start + count);
    else if (count < replacement_length)
        data->insert (data->begin () + start + count, replacement_length - count, 0);

    if (replacement_length > 0)
        memcpy (&data [start], replacement, replacement_length * sizeof (char));

    if (undo) 
        undo_manager->AddAction (
            new CVector8BitCharBufferChangeAction (*this, start, count, replacement_length, characters));

    if (characters != NULL) delete characters;

    FireOnChange (start, count, replacement_length);
}

bool CVector8BitCharBuffer::Load (LPCTSTR file_name)
{
    ASSERT (data != NULL);
    ASSERT (file_name != NULL);

    std::vector <char> *d = new std::vector <char> ();

    if (!CFileHelper::LoadFile (file_name, *d))
    {
        delete d;
        return false;
    }
    else
    {
        unsigned int old_length = data->size ();
        unsigned int new_length = d->size ();

        delete data;
        data = d;

        FireOnChange (0, old_length, new_length);

        return true;
    }
}

bool CVector8BitCharBuffer::Save (LPCTSTR file_name)
{
    ASSERT (data != NULL);
    ASSERT (file_name != NULL);

    return CFileHelper::SaveFile (file_name, *data);
}

CVector8BitCharBuffer::~CVector8BitCharBuffer ()
{
    if (data != NULL) delete data;
}

#pragma endregion
