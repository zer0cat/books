//==================================
// SIMONSEZ - Matt Pietrek 1995
// FILE: HOOKAPI.H
//==================================

// Returns: Original address of intercepted function (for chaining on)
PROC
WINAPI
HookImportedFunction(
        HMODULE hFromModule,        // Module to intercept calls from
        PSTR    pszFunctionModule,  // Module to intercept calls to
        PSTR    pszFunctionName,    // Function to intercept calls to
        PROC    pfnNewProc          // New function (replaces old function)
        );
