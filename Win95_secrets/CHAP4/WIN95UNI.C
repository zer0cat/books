//==================================
// WIN95UNI - Matt Pietrek 1995
// FILE: WIN95UNI.C
//==================================

#define UNICODE
#include <windows.h>

int main()
{
    MessageBox( 0,
                TEXT("Yes! Really!"),
                TEXT("Unicode in Windows 95?"),
                MB_ICONQUESTION );
    return 0;
}
