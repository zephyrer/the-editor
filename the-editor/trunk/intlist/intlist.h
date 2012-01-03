#ifndef INTLIST_H
#define INTLIST_H

#include <windows.h>

typedef struct tagINTLIST
{
    int length;
    int *data;

    HANDLE heap;
    int capacity;
} INTLIST;

BOOL intlist_initialize (INTLIST *list, int initial_capacity);
BOOL intlist_destroy (INTLIST *list);
BOOL intlist_replace_range (INTLIST *list, int offset, int length, int replacement_length, const int replacement []);
BOOL intlist_append (INTLIST *list, int value);

#endif
