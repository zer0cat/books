//==================================
// SHOW16 - Matt Pietrek 1995
// FILE: SHOW16.H
//==================================
#define IDC_GROUPBOX1   101
#define IDC_RB_TASKS    102
#define IDC_RB_MODULES  103
#define IDC_LB_MAIN_LIST    104
#define IDC_LB_DETAILS  105
#define IDC_BUTTON_EXIT 106
#define IDC_GROUPBOX2   108
#define IDC_DETAILS_TYPE    107

#ifndef RC_INVOKED
#pragma pack (1)
#endif  /* RC_INVOKED */

typedef struct
{
    WORD    value;
    LPSTR   name;
} WORD_FLAGS;

#define LB_ITEM_HMODULE             1
#define LB_ITEM_TASK                2
#define LB_ITEM_SEGMENTS            3
#define LB_ITEM_ENTRY_TABLE         4
#define LB_ITEM_RESOURCES           5
#define LB_ITEM_RESIDENT_NAMES      6
#define LB_ITEM_NONRESIDENT_NAMES   7
