#pragma once

class CFileHelper
{
private:
    inline CFileHelper (void)
    {
        // Do not instantiate me
    }

public:
    static bool LoadFile (LPCTSTR file_name, std::vector <char> &buffer);
    static bool SaveFile (LPCTSTR file_name, std::vector <char> &buffer);
};
