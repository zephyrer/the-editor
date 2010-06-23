#include "StdAfx.h"
#include "EditorDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CEditorDocument

// CEditorDocument

IMPLEMENT_DYNCREATE (CEditorDocument, CDocument)

BEGIN_MESSAGE_MAP (CEditorDocument, CDocument)
END_MESSAGE_MAP()

// CEditorDocument construction/destruction

CEditorDocument::CEditorDocument () : 
    undo_manager (), text (char_buffer)
{
    char_buffer.SetUndoManager (&undo_manager);
    text.SetListener (this);
    text.SetUndoManager (&undo_manager);
}

CEditorDocument::~CEditorDocument()
{
    // Do nothing
}

CText & CEditorDocument::GetText ()
{
    return text;
}

CUndoManager & CEditorDocument::GetUndoManager ()
{
    return undo_manager;
}

void CEditorDocument::DeleteContents ()
{
    char_buffer.ReplaceCharsRange (0, char_buffer.GetSize (), 0, NULL);
    undo_manager.ClearUndoHistory ();
}

BOOL CEditorDocument::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    SetModifiedFlag (FALSE);

    return TRUE;
}

BOOL CEditorDocument::OnOpenDocument (LPCTSTR lpszPathName)
{
    CFile file;

    if (!file.Open (lpszPathName, CFile::modeRead | CFile::shareDenyWrite)) return FALSE;

    vector <char> data;

    char buffer [65536];
    unsigned int l;
    unsigned int pos = 0;
    while ((l = file.Read (buffer, 65536)) > 0)
    {
        data.resize (pos + l);
        memcpy (&data [pos], buffer, l);
        pos += l;
    }

    file.Close ();

    if (data.size () > 0)
    {
        int s = MultiByteToWideChar (CP_ACP, 0, &data [0], data.size (), NULL, 0);
        vector <TCHAR> unicode;

        if (s > 0)
        {
            unicode.resize (s);
            MultiByteToWideChar (CP_ACP, 0, &data [0], data.size (), &unicode [0], s);
        }

        char_buffer.ReplaceCharsRange (0, char_buffer.GetSize (), s, &unicode [0]);
    }

    SetModifiedFlag (FALSE);
}

BOOL CEditorDocument::OnSaveDocument (LPCTSTR lpszPathName)
{
    unsigned int size = char_buffer.GetSize ();

    vector <char> data;

    if (size > 0)
    {
        vector <TCHAR> unicode;
        unicode.resize (size);
        char_buffer.GetCharsRange (0, size, &unicode [0]);

        int s = WideCharToMultiByte (CP_ACP, 0, &unicode [0], size, NULL, 0, "?", NULL);

        if (s > 0)
        {
            data.resize (s);
            WideCharToMultiByte (CP_ACP, 0, &unicode [0], size, &data [0], s, "?", NULL);
        }
    }

    CFile file;

    if (!file.Open (lpszPathName, CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate)) return FALSE;

    if (data.size () > 0)
    {
        file.Write (&data [0], data.size ());
    }

    file.Close ();

    SetModifiedFlag (FALSE);

    return TRUE;
}

void CEditorDocument::OnChange (
    unsigned int start_line, unsigned int start_position, 
    unsigned int old_end_line, unsigned int old_end_position, 
    unsigned int new_end_line, unsigned int new_end_position)
{
    SetModifiedFlag (TRUE);

    UpdateAllViews (NULL, 0, &CDocChange (start_line, old_end_line - start_line + 1, new_end_line - start_line + 1));
}

void CEditorDocument::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}

// CEditorDocument diagnostics

#ifdef _DEBUG
void CEditorDocument::AssertValid() const
{
    CDocument::AssertValid();
}

void CEditorDocument::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

#pragma endregion
