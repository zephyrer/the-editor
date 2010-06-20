
// EditorDocument.cpp : implementation of the CEditorDocument class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Editor.h"
#endif

#include "EditorDocument.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditorDocument

IMPLEMENT_DYNCREATE(CEditorDocument, CDocument)

BEGIN_MESSAGE_MAP(CEditorDocument, CDocument)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CEditorDocument, CDocument)
END_DISPATCH_MAP()

// Note: we add support for IID_IEditor to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {369BF9F3-54AA-4810-A0C0-8F678CAA669C}
static const IID IID_IEditor =
{ 0x369BF9F3, 0x54AA, 0x4810, { 0xA0, 0xC0, 0x8F, 0x67, 0x8C, 0xAA, 0x66, 0x9C } };

BEGIN_INTERFACE_MAP(CEditorDocument, CDocument)
    INTERFACE_PART(CEditorDocument, IID_IEditor, Dispatch)
END_INTERFACE_MAP()


// CEditorDocument construction/destruction

CEditorDocument::CEditorDocument() : undo_manager (), text (undo_manager)
{
    EnableAutomation();

    AfxOleLockApp();

    text.InsertLineAt (0, 80, L"01234567012345670123456701234567012345670123456701234567012345670123456701234567");

    for (int i = 0; i < 1000; i++)
    {
        TCHAR temp [1024];

        _stprintf_s (temp, 1024, L"    \t\t\t           ==========================\t===============\t%d\t==============================================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d-%d", i, i, i);
//        _stprintf (temp, L">\t=\t==\t===", i);

        text.InsertLineAt (i, _tcslen (temp), temp);
    }
}

CEditorDocument::~CEditorDocument()
{
    AfxOleUnlockApp();
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

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CEditorDocument::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
    // Modify this code to draw the document's data
    dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

    CString strText = _T("TODO: implement thumbnail drawing here");
    LOGFONT lf;

    CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
    pDefaultGUIFont->GetLogFont(&lf);
    lf.lfHeight = 36;

    CFont fontDraw;
    fontDraw.CreateFontIndirect(&lf);

    CFont* pOldFont = dc.SelectObject(&fontDraw);
    dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
    dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CEditorDocument::InitializeSearchContent()
{
    CString strSearchContent;
    // Set search contents from document's data. 
    // The content parts should be separated by ";"

    // For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
    SetSearchContent(strSearchContent);
}

void CEditorDocument::SetSearchContent(const CString& value)
{
    if (value.IsEmpty())
    {
        RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
    }
    else
    {
        CMFCFilterChunkValueImpl *pChunk = NULL;
        ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
        if (pChunk != NULL)
        {
            pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
            SetChunkValue(pChunk);
        }
    }
}

#endif // SHARED_HANDLERS

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
