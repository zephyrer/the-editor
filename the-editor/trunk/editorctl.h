#ifndef EDITORCTL_H
#define EDITORCTL_H

#include <windows.h>

/* Messages */
#define ECM_REQUEST_DATA (WM_USER + 0)
#define ECM_SET_MODEL_SIZE (WM_USER + 1)
#define ECM_GET_MODEL_SIZE (WM_USER + 2)
#define ECM_SET_CARET_POSITION (WM_USER + 3)
#define ECM_GET_CARET_POSITION (WM_USER + 4)
#define ECM_SET_CARET_SHAPE (WM_USER + 5)
#define ECM_SET_CELL_SIZE (WM_USER + 6)
#define ECM_GET_CELL_SIZE (WM_USER + 7)
#define ECM_GET_SCROLL_POSITION (WM_USER + 8)
#define ECM_SET_SCROLL_POSITION (WM_USER + 9)

#define ECS_NONE ((editorctl_style_t)0)
#define ECS_NORMAL ((editorctl_style_t)1)
#define ECS_SELECTED ((editorctl_style_t)2)
#define ECS_WHITESPACE ((editorctl_style_t)3)
#define ECS_SELECTED_WHITESPACE ((editorctl_style_t)4)

#define ECCS_NONE ((unsigned char)0)
#define ECCS_INSERT ((unsigned char)1)
#define ECCS_OVERWRITE ((unsigned char)2)

typedef UINT32 editorctl_unicode_char_t;
typedef unsigned char editorctl_style_t;

struct editorctl_data_request
{
    int start_row, end_row;
    int start_column, end_column;

    editorctl_unicode_char_t *buffer;
    editorctl_style_t *styles;
};

extern LPTSTR EDITORCTL_CLASS_NAME;

ATOM editorctl_register_class (HINSTANCE hInstance);

#endif
