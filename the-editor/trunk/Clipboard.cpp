#include "StdAfx.h"
#include "Clipboard.h"

bool CClipboard::HasText ()
{
    bool result = false;

    if (owner.OpenClipboard () != 0)
    {
        UINT format = 0;
        while ((format = EnumClipboardFormats (format)))
        {
            if (format == CF_UNICODETEXT || format == CF_TEXT)
            {
                result = true;
                break;
            }
        }

        CloseClipboard ();
    }

    return result;
}

TCHAR *CClipboard::GetText ()
{
    TCHAR *result = NULL;

    if (owner.OpenClipboard () != 0)
    {
        HANDLE h;
        h = GetClipboardData (CF_UNICODETEXT);
        if (h != 0)
        {
            TCHAR *unicodetext = (TCHAR *)GlobalLock (h);
            if (unicodetext != NULL)
            {
                unsigned int l = _tcslen (unicodetext);
                result = new TCHAR [l + 1];
                _tcscpy_s (result, l + 1, unicodetext);
            }
            GlobalUnlock (h);
        }
        else
        {
            h = GetClipboardData (CF_TEXT);
            if (h != 0)
            {
                char *text = (char *)GlobalLock (h);
                if (text != NULL)
                {
                    int l = MultiByteToWideChar (CP_ACP, 0, text, -1, NULL, 0);
                    result = new TCHAR [l];
                    if (MultiByteToWideChar (CP_ACP, 0, text, -1, result, l) == 0)
                    {
                        delete [] result;
                        result = NULL;
                    }
                }
                GlobalUnlock (h);
            }
        }

        CloseClipboard ();
    }

    return result;
}

bool CClipboard::SetText (const TCHAR *text)
{
    ASSERT (text != NULL);

    bool result = false;

    if (owner.OpenClipboard () != 0)
    {
        if (EmptyClipboard () != 0)
        {
            unsigned int l = _tcslen (text);

            HGLOBAL h;

            h = GlobalAlloc (GMEM_MOVEABLE, (l + 1) * sizeof (TCHAR));
            if (h != 0)
            {
                TCHAR *buffer = (TCHAR *)GlobalLock (h);

                if (buffer != NULL)
                {
                    _tcscpy_s (buffer, l + 1, text);
                    GlobalUnlock (h);
                    SetClipboardData (CF_UNICODETEXT, h);
                    result = true;
                }
            }

            int ll = WideCharToMultiByte (CP_ACP, 0, text, -1, NULL, 0, "?", NULL);
            if (ll > 0)
            {
                h = GlobalAlloc (GMEM_MOVEABLE, (ll) * sizeof (char));
                if (h != 0)
                {
                    char *buffer = (char *)GlobalLock (h);

                    if (buffer != NULL)
                    {
                        if (WideCharToMultiByte (CP_ACP, 0, text, -1, buffer, ll, "?", NULL) > 0)
                        {
                            GlobalUnlock (h);
                            SetClipboardData (CF_TEXT, h);
                            result = true;
                        }
                        else
                        {
                            GlobalUnlock (h);
                            GlobalFree (h);
                        }
                    }
                }
            }
        }

        CloseClipboard ();
    }

    return result;
}
