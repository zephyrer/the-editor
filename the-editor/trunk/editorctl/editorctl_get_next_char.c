#include "editorctl.h"

EDITORCTL_UNICODE_CHAR editorctl_get_next_char (const char **ptr)
{
    return *(*ptr)++;
}
