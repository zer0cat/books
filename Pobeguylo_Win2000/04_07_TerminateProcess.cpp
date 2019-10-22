#include <windows.h>
#include <conio.h>

int main()
{
  char lpszAppName[] = "C:\\ConsoleProcess.exe";

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);

  // создаем новый консольный процесс
  if (!CreateProcess(lpszAppName, NULL, NULL, NULL, FALSE,
      CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
  {
    _cputs("The new process is not created.\n");
    _cputs("Check a name of the process.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    return 0;
  }

  _cputs("The new process is created.\n");

  while (true)
  {
    char c;

    _cputs("Input 't' to terminate the new console process: ");
    c = _getch();
    if (c == 't')
    {
      _cputs("t\n");
      // завершаем новый процесс
      TerminateProcess(pi.hProcess,1);
      break;
    }
  }

  // закрываем дескрипторы нового процесса в текущем процессе
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return 0;
}