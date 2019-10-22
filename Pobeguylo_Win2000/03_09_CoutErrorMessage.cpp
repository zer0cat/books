#include <windows.h>
#include <iostream.h>

void CoutErrorMessage()
{
  char prefix[] = "Ошибка Win32 API: ";
  LPVOID lpMsgBuf;

  CharToOem(prefix,prefix);  // перекодируем заголовок

  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // язык по умолчанию
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
  );
  // перекодируем русские буквы
  CharToOem((char*)lpMsgBuf,(char*)lpMsgBuf);
  // выводим сообщение об ошибке на консоль
  cout << prefix << (char*)lpMsgBuf << endl;
  // освобождаем буфер
  LocalFree(lpMsgBuf);
}