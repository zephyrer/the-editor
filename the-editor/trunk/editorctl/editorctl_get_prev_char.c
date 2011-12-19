#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_prev_char (const char **ptr)
{
    return *--(*ptr);
}
