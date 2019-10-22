#include <windows.h>
#include <conio.h>

int main()
{
  HANDLE  hProcess, hThread;
  BOOL    bPriorityBoost;

  // получаем псевдодескриптор текущего процесса
  hProcess = GetCurrentProcess();

  // узнаем режим динамического повышения приоритетов для процесса
  if (!GetProcessPriorityBoost(hProcess, &bPriorityBoost))
  {
    _cputs("Get process priority boost failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }
  _cprintf("The process priority boost = %d.\n", bPriorityBoost);

  // выключаем режим динамического повышения приоритетов для процесса
  if (!SetProcessPriorityBoost(hProcess, TRUE))
  {
    _cputs("Set process priority boost failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }

  // получаем псевдодескриптор текущего потока
  hThread = GetCurrentThread();
  // узнаем режим динамического повышения приоритетов для потока
  if (!GetThreadPriorityBoost(hThread, &bPriorityBoost))
  {
    _cputs("Get process priority boost failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }
  _cprintf("The thread priority boost = %d.\n", bPriorityBoost);

  // включаем режим динамического повышения приоритетов для потока
  if (!SetThreadPriorityBoost(hThread, FALSE))
  {
    _cputs("Set process priority boost failed.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return GetLastError();
  }
  _cputs("Press any key to exit.\n");
  _getch();

  return 0;
}