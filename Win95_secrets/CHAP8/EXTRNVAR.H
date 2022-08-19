//==================================
// PEDUMP - Matt Pietrek 1995
// FILE: EXTRNVAR.H
//==================================

// EXTRNVAR.H holds the "extern" definitions for global variables used
// through the program.

extern BOOL fShowRelocations;
extern BOOL fShowRawSectionData;
extern BOOL fShowSymbolTable;
extern BOOL fShowLineNumbers;
extern BOOL fShowIATentries;
extern PIMAGE_SYMBOL PCOFFSymbolTable;
extern DWORD COFFSymbolCount;

extern PIMAGE_COFF_SYMBOLS_HEADER PCOFFDebugInfo;