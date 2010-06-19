#pragma once

#include "Text.h"

class CEditorDocument : public CDocument
{
protected:
    CSimpleInMemoryText text;

protected: // create from serialization only
    CEditorDocument ();
    DECLARE_DYNCREATE (CEditorDocument)

// Attributes
public:
    virtual CText & GetText ();

// Operations
public:

// Overrides
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
    virtual void InitializeSearchContent();
    virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
    virtual ~CEditorDocument();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()

    // Generated OLE dispatch map functions

    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

#ifdef SHARED_HANDLERS
    // Helper function that sets search content for a Search Handler
    void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
