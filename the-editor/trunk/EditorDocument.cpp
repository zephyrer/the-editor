#include "stdafx.h"

#include "EditorDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CTextWrapper

class CTextWrapper : public CText
{
protected:
    CEditorDocument &document;
    CText &text;

public:
    inline CTextWrapper (CUndoManager &undo_manager, CEditorDocument &document, CText &text) : 
        CText (undo_manager), document (document), text (text) 
    {
        int i = 0;
        // Do nothing
    }

    virtual unsigned int GetLinesCount ()
    {
        return text.GetLinesCount ();
    }

    virtual unsigned int GetLineLength (unsigned int line)
    {
        return text.GetLineLength (line);
    }

    virtual TCHAR GetCharAt (unsigned int line, unsigned int position)
    {
        return text.GetCharAt (line, position);
    }

    virtual void GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, TCHAR buffer [])
    {
        text.GetCharsRange (line, start_position, count, buffer);
    }

    virtual void InsertCharAt (unsigned int line, unsigned int position, TCHAR character)
    {
        text.InsertCharAt (line, position, character);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 1));
    }

    virtual void SetCharAt (unsigned int line, unsigned int position, TCHAR character)
    {
        text.SetCharAt (line, position, character);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 1));
    }

    virtual void RemoveCharAt (unsigned int line, unsigned int position)
    {
        text.RemoveCharAt (line, position);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 1));
    }

    virtual void ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, TCHAR replacement [])
    {
        text.ReplaceCharsRange (line, start_position, count, replacement_length, replacement);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 1));
    }

    virtual void BreakLineAt (unsigned int line, unsigned int position)
    {
        text.BreakLineAt (line, position);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 2));
    }

    virtual void JoinLines (unsigned int line)
    {
        text.JoinLines (line);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 2, 1));
    }

    virtual void InsertLineAt (unsigned int line, unsigned int length, TCHAR characters [])
    {
        text.InsertLineAt (line, length, characters);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 0, 1));
    }

    virtual void RemoveLineAt (unsigned int line)
    {
        text.RemoveLineAt (line);
        document.UpdateAllViews (NULL, 0, &CDocChange (line, 1, 0));
    }
};

#pragma endregion

#pragma region CEditorDocument

// CEditorDocument

IMPLEMENT_DYNCREATE (CEditorDocument, CDocument)

BEGIN_MESSAGE_MAP (CEditorDocument, CDocument)
END_MESSAGE_MAP()

// CEditorDocument construction/destruction

CEditorDocument::CEditorDocument () : 
    undo_manager (), text (undo_manager), text_wrapper (new CTextWrapper (undo_manager, *this, text))
{
    text.InsertLineAt (0, 0, NULL);
}

CEditorDocument::~CEditorDocument()
{
    delete text_wrapper;
}

CText & CEditorDocument::GetText ()
{
    return *text_wrapper;
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

#pragma endregion
