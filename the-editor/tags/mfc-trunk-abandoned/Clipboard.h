#pragma once

class CClipboard
{
protected:
    CWnd &owner;

public:
    inline CClipboard (CWnd &owner) : owner (owner) {}

    virtual bool HasText ();
    virtual TCHAR *GetText ();
    virtual bool SetText (const TCHAR *text);
};
