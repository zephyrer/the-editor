#include "StdAfx.h"
#include "UndoManager.h"

#pragma region CUndoableAction

CUndoableAction::~CUndoableAction ()
{
    // Do nothing
}

#pragma endregion

#pragma region CUndoManager

CUndoManager::~CUndoManager ()
{
    if (current_transaction != NULL) FinishTransaction ();
    ClearUndoHistory ();
}

bool CUndoManager::IsWithinTransaction ()
{
    return current_transaction != NULL;
}

void CUndoManager::StartTransaction ()
{
    ASSERT (current_transaction == NULL);

    for (unsigned int c = redo_history.size (), i = 0; i < c; i++)
    {
        vector <CUndoableAction *> *transaction = redo_history [i];

        for (unsigned int cc = transaction->size (), j = 0; j < cc; j++)
            delete (*transaction) [j];

        delete transaction;
    }

    redo_history.clear ();

    current_transaction = new vector <CUndoableAction *> ();
}

void CUndoManager::FinishTransaction ()
{
    ASSERT (current_transaction != NULL);

    if (current_transaction->size () > 0)
    {
        undo_history.insert (undo_history.end (), current_transaction);
    }
    else
    {
        delete current_transaction;
    }

    current_transaction = NULL;
}

void CUndoManager::AddAction (CUndoableAction *action)
{
    ASSERT (current_transaction != NULL);

    current_transaction->insert (current_transaction->end (), action);
}

void CUndoManager::ClearUndoHistory ()
{
    ASSERT (current_transaction == NULL);

    for (unsigned int c = undo_history.size (), i = 0; i < c; i++)
    {
        vector <CUndoableAction *> *transaction = undo_history [i];

        for (unsigned int cc = transaction->size (), j = 0; j < cc; j++)
            delete (*transaction) [j];

        delete transaction;
    }

    undo_history.clear ();

    for (unsigned int c = redo_history.size (), i = 0; i < c; i++)
    {
        vector <CUndoableAction *> *transaction = redo_history [i];

        for (unsigned int cc = transaction->size (), j = 0; j < cc; j++)
            delete (*transaction) [j];

        delete transaction;
    }

    redo_history.clear ();
}

bool CUndoManager::IsUndoAvailable ()
{
    ASSERT (current_transaction == NULL);

    return undo_history.size () > 0;
}

void CUndoManager::Undo ()
{
    ASSERT (current_transaction == NULL);
    ASSERT (undo_history.size () > 0);

    vector <CUndoableAction *> *transaction = undo_history [undo_history.size () - 1];
    undo_history.pop_back ();

    current_transaction = new vector <CUndoableAction *> ();

    for (unsigned int c = transaction->size (), i = 0; i < c; i++)
    {
        CUndoableAction *action = (*transaction) [c - i - 1];

        action->Undo ();

        delete action;
    }
    delete transaction;

    ASSERT (current_transaction != NULL);

    if (current_transaction->size () > 0)
        redo_history.insert (redo_history.end (), current_transaction);
    else delete current_transaction;

    current_transaction = NULL;
}

bool CUndoManager::IsRedoAvailable ()
{
    ASSERT (current_transaction == NULL);

    return redo_history.size () > 0;
}

void CUndoManager::Redo ()
{
    ASSERT (current_transaction == NULL);
    ASSERT (redo_history.size () > 0);

    vector <CUndoableAction *> *transaction = redo_history [redo_history.size () - 1];
    redo_history.pop_back ();

    current_transaction = new vector <CUndoableAction *> ();

    for (unsigned int c = transaction->size (), i = 0; i < c; i++)
    {
        CUndoableAction *action = (*transaction) [c - i - 1];

        action->Undo ();

        delete action;
    }
    delete transaction;

    ASSERT (current_transaction != NULL);

    if (current_transaction->size () > 0)
        undo_history.insert (undo_history.end (), current_transaction);
    else delete current_transaction;

    current_transaction = NULL;
}

#pragma endregion
