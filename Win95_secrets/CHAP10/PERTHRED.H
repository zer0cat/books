//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: PERTHRED.H
//==================================
#define MAX_HOOKED_FUNCTIONS 50

typedef struct
{
    PVOID pfnReturnAddress;
    PSTR pszName;
} HOOKED_FUNCTION;

typedef struct
{
    HOOKED_FUNCTION FunctionStack[MAX_HOOKED_FUNCTIONS];
    DWORD FunctionStackPtr;
} PER_THREAD_DATA, *PPER_THREAD_DATA;
