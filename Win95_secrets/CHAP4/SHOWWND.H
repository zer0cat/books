//==================================
// SHOWWND - Matt Pietrek 1995
// FILE: SHOWWND.H
//==================================

#define ICON_1  1
#define IDC_LB_MAIN_LIST    101
#define IDC_LB_DETAILS      102
#define IDC_BUTTON_EXIT     103
#define IDC_DETAILS_TYPE    104
#define IDC_BUTTON_REFRESH  105

#define LB_ITEM_WND             1
#define LB_ITEM_CLASS           2
#define LB_ITEM_QUEUE           3

typedef struct
{
    DWORD   value;
    LPSTR   name;
} DWORD_FLAGS;
