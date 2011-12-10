#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "xalloc.h"

extern void * xalloc (size_t size)
{
    void * result = malloc (size);
    if (result == NULL)
    {
        fprintf (stderr, "Out of memory while trying to allocate %u bytes", (unsigned int)size);
        exit (-1);
    }
    return result;
}

extern void xfree (void * memblock)
{
    free (memblock);
}

extern void * xrealloc (void *memblock, size_t size)
{
    void * result = realloc (memblock, size);
    if (size > 0 && result == NULL)
    {
        fprintf (stderr, "Out of memory while trying to reallocate %u bytes", (unsigned int)size);
        exit (-1);
    }
    return result;
}