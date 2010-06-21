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

public:
    virtual CText & GetText ();
    virtual CUndoManager & GetUndoManager ();

    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    virtual ~CEditorDocument();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};
