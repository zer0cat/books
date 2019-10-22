
// __________________________________________________________
//
//                         w2k_kill.c
//              Windows 2000 Killer Device V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_KILL_SYS_
#include <ddk\ntddk.h>
#include "w2k_kill.h"

// =================================================================
// DISCLAIMER
// =================================================================

/*

This software is provided "as is" and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed.
In no event shall the author <MyName> be liable for any
direct, indirect, incidental, special, exemplary, or consequential
damages (including, but not limited to, procurement of substitute
goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software,
even if advised of the possibility of such damage.

*/

// =================================================================
// REVISION HISTORY
// =================================================================

/*

08-27-2000 V1.00 Original version (SBS).

*/

// =================================================================
// DISCARDABLE FUNCTIONS
// =================================================================

NTSTATUS DriverEntry (PDRIVER_OBJECT  pDriverObject,
                      PUNICODE_STRING pusRegistryPath);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

// =================================================================
// DRIVER INITIALIZATION
// =================================================================

NTSTATUS DriverEntry (PDRIVER_OBJECT  pDriverObject,
                      PUNICODE_STRING pusRegistryPath)
    {
    return *((NTSTATUS *) 0);
    }

// =================================================================
// END OF PROGRAM
// =================================================================
