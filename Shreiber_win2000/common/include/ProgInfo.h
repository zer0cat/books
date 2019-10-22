
// __________________________________________________________
//
//                         ProgInfo.h
//               Program Info Definitions V1.02
//                03-08-1999 Sven B. Schreiber
//                        sbs@orgon.com
// __________________________________________________________
    
#ifndef _PROGINFO_H_
#define _PROGINFO_H_
#include <windows.h>

// =================================================================
// DISCLAIMER
// =================================================================

/*

This software is provided "as is" and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantibility and fitness for a particular purpose are disclaimed.
In no event shall the author Sven B. Schreiber be liable for any
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

04-29-1998 V1.00 Original version (SBS).

02-07-1999 V1.01 Update (SBS).

    Replaced all '$' macro parameter prefixes by '_', because some
    compilers don't accept non-standard characters in symbols.

03-08-1999 V1.02 Update (SBS).

    Added the macros TSIZEA(), TSIZEW(), lstrcpybA(), and
    lstrcpybW() to facilitate development of dual ANSI/Unicode
    modules.

*/

// =================================================================
// MACROS
// =================================================================

#if MAIN_VERSION_LOW < 10

#define _V2(_a,_b)               _a ## .0 ## _b
#define _V2X(_a,_b)         V ## _a ## .0 ## _b

#else // #if MAIN_VERSION_LOW < 10

#define _V2(_a,_b)               _a ## .  ## _b
#define _V2X(_a,_b)         V ## _a ## .  ## _b

#endif // #if MAIN_VERSION_LOW < 10 #else

#define V2(_a,_b)           _V2(_a,_b)
#define V2X(_a,_b)          _V2X(_a,_b)

// -----------------------------------------------------------------

#define _V4(_a,_b,_c)       _a ## . ## _b ## .0. ## _c
#define V4(_a,_b,_c)        _V4(_a,_b,_c)

// -----------------------------------------------------------------

#define MAIN_V              V2X (MAIN_VERSION_HIGH, \
                                 MAIN_VERSION_LOW)

#define MAIN_VERSION        V2  (MAIN_VERSION_HIGH, \
                                 MAIN_VERSION_LOW)

#define MAIN_VERSION_QUAD   V4  (MAIN_VERSION_HIGH, \
                                 MAIN_VERSION_LOW,  \
                                 MAIN_BUILD)

#define MAIN_VERSION_BINARY    ((MAIN_VERSION_HIGH * 100) \
                               + MAIN_VERSION_LOW)

// -----------------------------------------------------------------

#define _SA(_a)                  #_a
#define _SW(_a)             L ## #_a

#define SA(_a)              _SA(_a)
#define SW(_a)              _SW(_a)

#define TSIZEA(_a)           sizeof (_a)
#define TSIZEW(_a)          (sizeof (_a) / WORD_)

#define lstrcpybA(_a,_b)    lstrcpynA (_a, _b, TSIZEA (_a))
#define lstrcpybW(_a,_b)    lstrcpynW (_a, _b, TSIZEW (_a))

// -----------------------------------------------------------------

#ifdef  UNICODE

#define S(_a)               SW(_a)
#define _T(_a)              L ## _a
#define TSIZE(_a)           TSIZEW(_a)
#define lstrcpyb(_a,_b)     lstrcpybW(_a,_b)

#else   // #ifdef UNICODE

#define S(_a)               SA(_a)
#define _T(_a)              _a
#define TSIZE(_a)           TSIZEA(_a)
#define lstrcpyb(_a,_b)     lstrcpybA(_a,_b)

#endif  // #ifdef UNICODE #else

// -----------------------------------------------------------------

#define T(_a)               _T(_a)

#define _PAIR(_a,_b)        _a ## _b
#define PAIR(_a,_b)         _PAIR(_a,_b)

// =================================================================
// PROGRAM INFORMATION
// =================================================================

#define MAIN_ID                 MAIN_PREFIX.MAIN_MODULE
#define MAIN_ID_VERSION         MAIN_ID.MAIN_VERSION_HIGH
#define MAIN_FILENAME           MAIN_MODULE.MAIN_EXTENSION
#define MAIN_CAPTION            MAIN_NAME MAIN_V
#define MAIN_COMMENT            MAIN_DATE MAIN_AUTHOR

// -----------------------------------------------------------------

#define MAIN_CLASS              MAIN_MODULE.MAIN_VERSION_QUAD
#define MAIN_MENU               MAIN_MODULE.Menu
#define MAIN_ICON               MAIN_MODULE.Icon
#define MAIN_CURSOR             MAIN_MODULE.Cursor
#define MAIN_BITMAP             MAIN_MODULE.Bitmap
#define MAIN_DIALOG             MAIN_MODULE.Dialog
#define MAIN_ABOUT              MAIN_MODULE.About

// -----------------------------------------------------------------

#define MAIN_COPYRIGHT          Copyright \xA9 MAIN_YEAR
#define MAIN_COPYRIGHT_EX       MAIN_COPYRIGHT MAIN_COMPANY

// -----------------------------------------------------------------

#define MAIN_DATE_US            MAIN_MONTH-MAIN_DAY-MAIN_YEAR
#define MAIN_DATE_GERMAN        MAIN_DAY.MAIN_MONTH.MAIN_YEAR
#define MAIN_DATE               MAIN_DATE_US

// -----------------------------------------------------------------

#define MAIN_LANGUAGE           0409

#ifdef  UNICODE
#define MAIN_CODEPAGE           04B0
#else
#define MAIN_CODEPAGE           04E4
#endif

#define MAIN_TRANSLATION        PAIR (MAIN_LANGUAGE, MAIN_CODEPAGE)

// -----------------------------------------------------------------

#ifdef  MAIN_EXE
#define MAIN_VFT                VFT_APP
#define MAIN_EXTENSION          exe
#endif

#ifdef  MAIN_DLL
#define MAIN_VFT                VFT_DLL
#define MAIN_EXTENSION          dll
#endif

// -----------------------------------------------------------------

#define ABOUT_CAPTION           About MAIN_NAME
#define ABOUT_ICON              MAIN_ICON
#define ABOUT_DIALOG            MAIN_ABOUT
#define ABOUT_TEXT1             MAIN_FILENAME
#define ABOUT_TEXT2             MAIN_CAPTION
#define ABOUT_TEXT3             MAIN_COMMENT
#define ABOUT_TEXT4             MAIN_EMAIL

////////////////////////////////////////////////////////////////////
#ifdef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// HEADER FILES
// =================================================================

#include <ver.h>

// =================================================================
// VERSION INFO
// =================================================================

#define MAIN_RC_VERSION \
VS_VERSION_INFO VERSIONINFO \
\
FILEVERSION     MAIN_VERSION_HIGH, MAIN_VERSION_LOW, 0, MAIN_BUILD \
PRODUCTVERSION  MAIN_VERSION_HIGH, MAIN_VERSION_LOW, 0, MAIN_BUILD \
FILEFLAGSMASK   VS_FFI_FILEFLAGSMASK \
FILEFLAGS       0 \
FILEOS          VOS_UNKNOWN \
FILETYPE        MAIN_VFT \
FILESUBTYPE     VFT2_UNKNOWN \
  { \
  BLOCK "StringFileInfo" \
    { \
    BLOCK SA(MAIN_TRANSLATION) \
      { \
      VALUE "OriginalFilename", SA(MAIN_FILENAME\0) \
      VALUE "InternalName",     SA(MAIN_MODULE\0) \
      VALUE "ProductName",      SA(MAIN_NAME\0) \
      VALUE "FileDescription",  SA(MAIN_CAPTION\0) \
      VALUE "CompanyName",      SA(MAIN_COMPANY\0) \
      VALUE "ProductVersion",   SA(MAIN_VERSION_QUAD\0) \
      VALUE "FileVersion",      SA(MAIN_VERSION_QUAD\0) \
      VALUE "LegalCopyright",   SA(MAIN_COPYRIGHT_EX\0) \
      VALUE "Comments",         SA(MAIN_COMMENT\0) \
      } \
    } \
  BLOCK "VarFileInfo" \
      { \
      VALUE "Translation", 0xMAIN_LANGUAGE, 0xMAIN_CODEPAGE \
      } \
  }

// =================================================================
// RESOURCES
// =================================================================

#define MAIN_RC_MENU   MAIN_MENU   MENU
#define MAIN_RC_ICON   MAIN_ICON   ICON   MAIN_MODULE.ico
#define MAIN_RC_CURSOR MAIN_CURSOR CURSOR MAIN_MODULE.cur
#define MAIN_RC_BITMAP MAIN_BITMAP BITMAP MAIN_MODULE.bmp

////////////////////////////////////////////////////////////////////
#endif // #ifdef _RC_PASS_
////////////////////////////////////////////////////////////////////

#endif // #ifndef _PROGINFO_H_

// =================================================================
// END OF FILE
// =================================================================
