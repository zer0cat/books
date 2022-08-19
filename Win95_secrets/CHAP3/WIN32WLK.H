//==================================
// WIN32WLK - Matt Pietrek 1995
// FILE: WIN32WLK.H
//==================================
#define IDC_GROUPBOX1       101
#define IDC_RB_PROCESSES    102
#define IDC_RB_THREADS      103
#define IDC_RB_MODULES      104
#define IDC_LB_MAIN_LIST    105
#define IDC_LB_DETAILS      106
#define IDC_GROUPBOX2       108
#define IDC_DETAILS_TYPE    109

#define LB_ITEM_PROCESS             1
#define LB_ITEM_THREAD              2
#define LB_ITEM_HMODULE             3
#define LB_ITEM_HANDLE_TABLE        4
#define LB_ITEM_MODREF_LIST         5
#define LB_ITEM_TIB                 6

typedef struct
{
    DWORD   value;
    LPSTR   name;
} DWORD_FLAGS;
