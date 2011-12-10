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
    undo_manager ()
{
    CUndoManager *um = new CUndoManager ();
    CVectorCharBuffer *cb = new CVectorCharBuffer (CP_UTF8);
    cb->SetUndoManager (um);
    CCharBufferText *t = new CCharBufferText (*cb);
    t->SetListener (this);
    t->SetUndoManager (um);

    undo_manager = um;
    char_buffer = cb;
    text = t;
}

CEditorDocument::~CEditorDocument()
{
    if (text != NULL) delete text;
    if (char_buffer != NULL) delete char_buffer;
    if (undo_manager != NULL) delete undo_manager;
}

CText & CEditorDocument::GetText ()
{
    ASSERT (text != NULL);

    return *text;
}

CUndoManager & CEditorDocument::GetUndoManager ()
{
    ASSERT (undo_manager != NULL);

    return *undo_manager;
}

void CEditorDocument::DeleteContents ()
{
    CUndoManager *um = new CUndoManager ();
    CVectorCharBuffer *cb = new CVectorCharBuffer (CP_UTF8);
    cb->SetUndoManager (um);
    CCharBufferText *t = new CCharBufferText (*cb);
    t->SetListener (this);
    t->SetUndoManager (um);

    if (text != NULL) delete text;
    if (char_buffer != NULL) delete char_buffer;
    if (undo_manager != NULL) delete undo_manager;

    undo_manager = um;
    char_buffer = cb;
    text = t;

    SetModifiedFlag (FALSE);

    UpdateAllViews (NULL, 0, &CDocChange (true, 0, 0, 0));
}

BOOL CEditorDocument::OnOpenDocument (LPCTSTR lpszPathName)
{
    CUndoManager *um = new CUndoManager ();

    CVector8BitCharBuffer *cb = new CVector8BitCharBuffer (CP_ACP);
    if (!cb->Load (lpszPathName))
    {
        delete cb;
        return FALSE;
    }
    cb->SetUndoManager (um);

    CCharBufferText *t = new CCharBufferText (*cb);
    t->SetListener (this);
    t->SetUndoManager (um);


    if (text != NULL) delete text;
    if (char_buffer != NULL) delete char_buffer;
    if (undo_manager != NULL) delete undo_manager;

    undo_manager = um;
    char_buffer = cb;
    text = t;

    SetModifiedFlag (FALSE);

    UpdateAllViews (NULL, 0, &CDocChange (true, 0, 0, 0));

    return TRUE;
}

BOOL CEditorDocument::OnSaveDocument (LPCTSTR lpszPathName)
{
    if (!((CVector8BitCharBuffer *)char_buffer)->Save (lpszPathName))
        return FALSE;

    SetModifiedFlag (FALSE);

    return TRUE;
}

void CEditorDocument::OnChange (
    unsigned int start_line, unsigned int start_position, 
    unsigned int old_end_line, unsigned int old_end_position, 
    unsigned int new_end_line, unsigned int new_end_position)
{
    SetModifiedFlag (TRUE);

    UpdateAllViews (NULL, 0, &CDocChange (false, start_line, old_end_line - start_line + 1, new_end_line - start_line + 1));
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
