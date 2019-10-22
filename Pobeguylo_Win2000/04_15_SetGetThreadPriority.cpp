#include <windows.h>
#include <conio.h>

int main()
{
  HANDLE  hThread;
  DWORD  dwPriority;

  // получаем псевдодескриптор текущего потока
  hThread = GetCurrentThread();
  
  // узнаем уровень приоритета текущего процесса
  dwPriority = GetThreadPriority(hThread);
  _cprintf("The priority level of the thread = %d.\n", dwPriority);

  // понижаем приоритет текущего потока
  if (!SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST))
  {
    _cputs("Set thread priority failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }
  // узнаем уровень приоритет текущего потока
  dwPriority = GetThreadPriority(hThread);
  _cprintf("The priority level of the thread = %d.\n", dwPriority);

  // повышаем приоритет текущего потока
  if (!SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST))
  {
    _cputs("Set thread priority failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }
  // узнаем уровень приоритета текущего потока
  dwPriority = GetThreadPriority(hThread);
  _cprintf("The priority level of the thread = %d.\n", dwPriority);

  _cputs("Press any key to exit.\n");
  _getch();

  return 0;
}