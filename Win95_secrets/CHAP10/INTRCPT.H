//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: INTRCPT.H
//==================================
BOOL InterceptFunctionsInModule(PVOID baseAddress);

BOOL AddAPIFunction
(
    PSTR pszModule,     // exporting DLL name
    PSTR pszFuncName,   // exported function name
    PBYTE params        // opcode encoded parameters of exported function
);
