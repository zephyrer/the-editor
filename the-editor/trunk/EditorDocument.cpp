
// EditorDocument.cpp : implementation of the CEditorDocument class
//

#include "stdafx.h"

#include "EditorDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditorDocument

IMPLEMENT_DYNCREATE(CEditorDocument, CDocument)

BEGIN_MESSAGE_MAP(CEditorDocument, CDocument)
END_MESSAGE_MAP()

// CEditorDocument construction/destruction

CEditorDocument::CEditorDocument() : undo_manager (), text (undo_manager)
{
    text.InsertLineAt (0, 0, NULL);
}

CEditorDocument::~CEditorDocument()
{
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




// CEditorDocument serialization

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
