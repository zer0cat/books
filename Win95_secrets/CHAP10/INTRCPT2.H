//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: INTRCPT2.H
//==================================

#pragma pack(1)

typedef struct
{
    PVOID   RealProcAddress;
    BYTE    instr_pushad;
    DWORD   instr_lea_eax_esp_plus_32;
    BYTE    instr_push_eax;
    BYTE    instr_push_offset_params;
    DWORD   offset_params;
    BYTE    instr_push_offset_funcName;
    DWORD   offset_funcName;
    BYTE    instr_call_LogFunction;
    DWORD   offset_LogFunction;
    BYTE    instr_popad;
    WORD    instr_jmp_dword_ptr_RealProcAddress;
    DWORD   offset_dword_ptr_RealProcAddrss;
} APIFunction, *PAPIFunction;

#pragma pack ()
