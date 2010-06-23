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

BOOL CEditorDocument::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)

    return TRUE;
}


void CEditorDocument::OnChange (
    unsigned int start_line, unsigned int start_position, 
    unsigned int old_end_line, unsigned int old_end_position, 
    unsigned int new_end_line, unsigned int new_end_position)
{
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
