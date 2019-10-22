#include <windows.h>
#include <conio.h>

int main()
{
  char lpszCommandLine[] = "C:\\ConsoleProcess.exe p1 p2 p3";

  STARTUPINFO si;
  PROCESS_INFORMATION piCom;

  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);

  // создаем новый консольный процесс
  CreateProcess(NULL, lpszCommandLine, NULL, NULL, FALSE,
      CREATE_NEW_CONSOLE, NULL, NULL, &si, &piCom);
  // закрываем дескрипторы этого процесса
  CloseHandle(piCom.hThread);
  CloseHandle(piCom.hProcess);

  _cputs("The new process is created.\n");
  _cputs("Press any key to finish.\n");
  _getch();

  return 0;
}