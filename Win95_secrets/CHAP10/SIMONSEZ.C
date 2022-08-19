//==================================
// SIMONSEZ - Matt Pietrek 1995
// FILE: HOOKAPI.C
//==================================
#include <windows.h>
#include <malloc.h>
#include "hookapi.h"

// Make a typedef for the WINAPI function we're going to intercept
typedef int (__stdcall *MESSAGEBOXPROC)(HWND, LPCSTR, LPCSTR, UINT);

MESSAGEBOXPROC PfnOriginalMessageBox;   // for storing original address

//
// A special version of MessageBox that always prepends "Simon Sez: "
// to the text that will be displayed.
//
int WINAPI MyMessageBox( HWND hWnd, LPCSTR lpText,
                         LPCSTR lpCaption, UINT uType )
{
    int retValue;               // real MessageBox return value
    PSTR lpszRevisedString;     // pointer to our modified string
    
    // Allocate space for our revised string - add 40 bytes for new stuff
    lpszRevisedString = malloc( lstrlen(lpText) + 40 );

    // Now modify the original string to first say "Simon Sez: "
    if ( lpszRevisedString )
    {
        lstrcpy(lpszRevisedString, "Simon Sez: ");
        lstrcat(lpszRevisedString, lpText);
    }
    else                                    // If malloc() failed, just
        lpszRevisedString = (PSTR)lpText;   // use the original string.
    
    // Chain on to the original function in USER32.DLL.
    retValue = PfnOriginalMessageBox(hWnd,lpszRevisedString,lpCaption,uType);

    if ( lpszRevisedString != lpText )  // If we sucessfully allocated string
        free( lpszRevisedString );      // memory, then free it.
            
    return retValue;    // Return whatever the real MessageBox returned
}

int APIENTRY WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                        LPSTR lpszCmdLine, int nCmdShow )
{
    MessageBox(0, "MessageBox Isn't Intercepted Yet", "Test", MB_OK);
    
    // Intercept the calls that this module (TESTHOOK) makes to
    // MessageBox() in USER32.DLL.  The function that intercepts the
    // calls will be MyMessageBox(), above.

    PfnOriginalMessageBox = (MESSAGEBOXPROC) HookImportedFunction(
                    GetModuleHandle(0),     // Hook our own module
                    "USER32.DLL",           // MessageBox is in USE32.DLL
                    "MessageBoxA",          // function to intercept
                    (PROC)MyMessageBox);    // interception function

    if ( !PfnOriginalMessageBox )   // Make sure the interception worked
    {
        MessageBox(0, "Couldn't hook function", 0, MB_OK);
        return 0;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!  WARNING  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// When built with optimizations, the VC++ compiler loads a
	// register with the address of MessageBoxA, and then makes all
	// subsequent calls through it.  This can cause the MessageBox call
	// below to not go through the Import Address table that we just patched.
	// For this reason, the .MAK file for this program does not use the
	// /O2 or /O1 switches.  This usually won't be a problem, but it
	// was in this particularly simple program.  ACCKK!!!!

    // Call MessageBox again.  However, since we've now intercepted
    // MessageBox, control should first go to our own function
    // (MyMessageBox), rather than the MessageBox() code in USER32.DLL.

    MessageBox(0, "MessageBox Is Now Intercepted", "Test", MB_OK);

    return 0;
}
