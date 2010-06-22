#include "StdAfx.h"
#include "CharBuffer.h"

#pragma region CCharBuffer

CCharBuffer::~CCharBuffer(void)
{
    // Do nothing
}

#pragma endregion

#pragma region CVectorCharBuffer

unsigned int CVectorCharBuffer::GetSize ()
{
    return data.size ();
}

TCHAR CVectorCharBuffer::GetCharAt (unsigned int position)
{
    ASSERT (position < data.size ());

    return data [position];
}

void CVectorCharBuffer::GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer [])
{
    ASSERT (start <= data.size ());
    ASSERT (start + count <= data.size ());
    ASSERT (count == 0 || buffer != NULL);

    if (count > 0)
        memcpy (buffer, &data [start], count * sizeof (TCHAR));
}

void CVectorCharBuffer::SetCharAt (unsigned int position, TCHAR character)
{
    ASSERT (position < data.size ());

    data [position] = character;
}

void CVectorCharBuffer::InsertCharAt (unsigned int position, TCHAR character)
{
    ASSERT (position <= data.size ());

    data.insert (data.begin () + position, character);
}

void CVectorCharBuffer::RemoveCharAt (unsigned int position)
{
    ASSERT (position < data.size ());

    data.erase (data.begin () + position);
}

void CVectorCharBuffer::ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
{
    ASSERT (start <= data.size ());
    ASSERT (start + count <= data.size ());
    ASSERT (replacement_length == 0 || replacement != NULL);

    if (count > replacement_length)
        data.erase (data.begin () + start + replacement_length, data.begin () + start + count);
    else if (count < replacement_length)
        data.insert (data.begin () + start + count, replacement_length - count, 0);

    if (replacement_length > 0)
        memcpy (&data [start], replacement, replacement_length * sizeof (TCHAR));
}

CVectorCharBuffer::~CVectorCharBuffer (void)
{
    // Do nothing
}

#pragma endregion
