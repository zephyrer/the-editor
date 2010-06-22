#pragma once

class CCharBuffer : public CObject
{
public:
    virtual unsigned int GetSize () = 0;
    virtual TCHAR GetCharAt (unsigned int position) = 0;
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []) = 0;
    virtual void SetCharAt (unsigned int position, TCHAR character) = 0;
    virtual void InsertCharAt (unsigned int position, TCHAR character) = 0;
    virtual void RemoveCharAt (unsigned int position) = 0;
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []) = 0;

    virtual ~CCharBuffer (void);
};

class CVectorCharBuffer : public CCharBuffer
{
protected:
    std::vector <TCHAR> data;

public:
    virtual unsigned int GetSize ();
    virtual TCHAR GetCharAt (unsigned int position);
    virtual void GetCharsRange (unsigned int start, unsigned int count, TCHAR buffer []);
    virtual void SetCharAt (unsigned int position, TCHAR character);
    virtual void InsertCharAt (unsigned int position, TCHAR character);
    virtual void RemoveCharAt (unsigned int position);
    virtual void ReplaceCharsRange (unsigned int start, unsigned int count, unsigned int replacement_length, TCHAR replacement []);

    virtual ~CVectorCharBuffer (void);
};
