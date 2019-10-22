#include <windows.h>

void ErrorMessageBox()
{
  LPVOID lpMsgBuf;

  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // язык по умолчанию
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
  );
  // Показать ошибку в MessageBox.
  MessageBox(
    NULL, 
    (LPCTSTR)lpMsgBuf, 
    "Ошибка Win32 API", 
    MB_OK | MB_ICONINFORMATION
  );
  // Освободить буфер.
  LocalFree(lpMsgBuf);
}