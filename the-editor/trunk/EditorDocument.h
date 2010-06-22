#pragma once

#include "Text.h"
#include "UndoManager.h"

struct CDocChange : public CObject
{
    unsigned int first_line;
    unsigned int original_lines_count;
    unsigned int new_lines_count;

    inline CDocChange (unsigned int first_line, unsigned int original_lines_count, unsigned int new_lines_count) :
        first_line (first_line), original_lines_count (original_lines_count), new_lines_count (new_lines_count)
    {
    }
};

class CEditorDocument : public CDocument
{
protected:
    CVectorCharBuffer char_buffer;
    CCharBufferText text;
    CText *text_wrapper;
    CUndoManager undo_manager;

    CEditorDocument ();
    DECLARE_DYNCREATE (CEditorDocument)

    DECLARE_MESSAGE_MAP()

public:
    virtual CText &GetText ();
    virtual CUndoManager & GetUndoManager ();

    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    virtual ~CEditorDocument();

#ifdef _DEBUG
    virtual void AssertValid () const;
    virtual void Dump (CDumpContext& dc) const;
#endif
};
