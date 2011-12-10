#pragma once

#include "Text.h"
#include "UndoManager.h"

struct CDocChange : public CObject
{
    bool text_replaced;
    unsigned int first_line;
    unsigned int original_lines_count;
    unsigned int new_lines_count;

    inline CDocChange (bool text_replaced, unsigned int first_line, unsigned int original_lines_count, unsigned int new_lines_count) :
        text_replaced (text_replaced), first_line (first_line), original_lines_count (original_lines_count), new_lines_count (new_lines_count)
    {
    }
};

class CEditorDocument : public CDocument, CTextListener
{
protected:
    CUndoManager *undo_manager;
    CCharBuffer *char_buffer;
    CText *text;

    CEditorDocument ();
    DECLARE_DYNCREATE (CEditorDocument)

    DECLARE_MESSAGE_MAP()

public:
    virtual CText &GetText ();
    virtual CUndoManager & GetUndoManager ();

    virtual void DeleteContents ();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

    virtual void OnChange (
        unsigned int start_line, unsigned int start_position, 
        unsigned int old_end_line, unsigned int old_end_position, 
        unsigned int new_end_line, unsigned int new_end_position);

    virtual void Serialize(CArchive& ar);
    virtual ~CEditorDocument();

#ifdef _DEBUG
    virtual void AssertValid () const;
    virtual void Dump (CDumpContext& dc) const;
#endif
};
