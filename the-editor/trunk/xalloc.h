#ifndef XALLOC_H
#define XALLOC_H

#include <stddef.h>

extern void * xalloc (size_t);
extern void * xrealloc (void *, size_t);
extern void xfree (void *);

#endif