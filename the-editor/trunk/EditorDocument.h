#pragma once

#include "Text.h"
#include "UndoManager.h"

class CEditorDocument : public CDocument
{
protected:
    CSimpleInMemoryText text;
    CUndoManager undo_manager;

    CEditorDocument ();
    DECLARE_DYNCREATE (CEditorDocument)

    DECLARE_MESSAGE_MAP()

    // Generated OLE dispatch map functions

    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

#ifdef SHARED_HANDLERS
    // Helper function that sets search content for a Search Handler
    void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

public:
    virtual CText & GetText ();
    virtual CUndoManager & GetUndoManager ();

    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
    virtual void InitializeSearchContent();
    virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS
    virtual ~CEditorDocument();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};
