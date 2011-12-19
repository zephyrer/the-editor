#include "editorctl.h"

void editorctl_set_next_char (EDITORCTL_UNICODE_CHAR ch, char **ptr)
{
    *(*ptr)++ = (char)ch;
}
