#pragma once

using namespace std;

class CUndoableAction : public CObject
{
public:
    virtual ~CUndoableAction ();

    virtual void Undo () = 0;
};

class CUndoManager : public CObject
{
protected:
    vector <vector <CUndoableAction *> *> undo_history;
    vector <vector <CUndoableAction *> *> redo_history;

    vector <CUndoableAction *> * current_transaction;

public:
    inline CUndoManager () : current_transaction (NULL) {}
    virtual ~CUndoManager ();

    virtual void StartTransaction ();
    virtual void FinishTransaction ();
    virtual void AddAction (CUndoableAction *action);
    virtual void ClearUndoHistory ();
    virtual bool IsUndoAvailable ();
    virtual void Undo ();
    virtual bool IsRedoAvailable ();
    virtual void Redo ();
};
