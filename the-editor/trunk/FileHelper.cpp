#include "StdAfx.h"

#include "FileHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CFileHelper

bool CFileHelper::LoadFile (LPCTSTR file_name, std::vector <char> &buffer)
{
    ASSERT (file_name != NULL);

    bool status = false;
    CFile *file = NULL;
    
    try
    {
        file = new CFile (file_name, CFile::modeRead | CFile::shareDenyWrite);

        unsigned long long length = file->GetLength ();
        if (length <  UINT_MAX)
        {
            unsigned int new_length = (unsigned int)length;
            buffer.resize (new_length);

            unsigned int read_length = 0;
            if (length > 0)
                read_length =  file->Read (&buffer [0], new_length);

            if (read_length != new_length) status = false;

            status = true;
        }
    }
    catch (CFileException *ex)
    {
        ex->Delete ();
        status = false;
    }

    if (file != NULL)
    {
        file->Close ();
        delete file;
    }

    return status;
}

bool CFileHelper::SaveFile (LPCTSTR file_name, std::vector <char> &buffer)
{
    ASSERT (file_name != NULL);

    bool status = false;
    CFile *file = NULL;
    
    try
    {
        file = new CFile (file_name, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);

        unsigned int length = (unsigned int)buffer.size ();
        if (length > 0)
            file->Write (&buffer [0], length);

        status = true;
    }
    catch (CFileException *ex)
    {
        ex->Delete ();
        status = false;
    }

    if (file != NULL)
    {
        file->Close ();
        delete file;
    }

    return status;
}

#pragma endregion
