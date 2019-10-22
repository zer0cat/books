#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <iostream.h>

CRITICAL_SECTION cs;

DWORD WINAPI thread(LPVOID)
{
  int i,j;
  
  for (j = 0; j < 10; ++j)
    // попытка войти в критическую секцию
    if (TryEnterCriticalSection (&cs))
    {
      for (i = 0; i < 10; i++)
        cout << j << flush;
      cout << endl;
      // выход из критической секции
      LeaveCriticalSection(&cs);
    }
    
  return 0;
}

int main()
{
  int i,j;
  HANDLE  hThread;
  DWORD  IDThread;
  // инициализируем критическую секцию
  InitializeCriticalSection(&cs);

  hThread=CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  for (j = 10; j < 20; j++)
    // попытка войти в критическую секцию
    if (TryEnterCriticalSection(&cs))
    {
      for (i = 0; i < 10; i++)
        cout << j << ' ' << flush;
      cout << endl;
      // выход из критической секции
      LeaveCriticalSection(&cs);
    }
  // ждем завершения работы потока thread
  WaitForSingleObject(hThread, INFINITE);
  // удаляем критическую секцию
  DeleteCriticalSection(&cs);

  return 0;
}