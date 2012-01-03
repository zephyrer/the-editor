#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

typedef struct tagUTILS_INT_LIST
{
    int length;
    int *data;

    HANDLE heap;
    int capacity;
} UTILS_INT_LIST;

int utils_bsearch_int (int value, const int list [], int length);

BOOL utils_initialize_int_list (UTILS_INT_LIST *list, int initial_capacity);
BOOL utils_destroy_int_list (UTILS_INT_LIST *list);
BOOL utils_replace_int_list_range (UTILS_INT_LIST *list, int offset, int length, int replacement_length, const int replacement []);
BOOL utils_int_list_append (UTILS_INT_LIST *list, int value);

#endif
