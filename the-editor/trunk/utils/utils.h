#ifndef UTILS_H
#define UTILS_H

int utils_bsearch_int (int value, const int list [], int length);
int utils_max_int (const int list [], int length);
int utils_update_max_int (const int list [], int list_length, int offset, int length, int replacement_length, const int replacement [], int old_max);
void utils_increase_int (int list [], int list_length, int delta);

#endif
