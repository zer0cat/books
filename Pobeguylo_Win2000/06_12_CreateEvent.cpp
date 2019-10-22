#include <windows.h>
#include <iostream.h>

HANDLE hInEvent;
CHAR lpEventName[] = "InEventName";

int main()
{
  DWORD dwWaitResult;

  char szAppName[] = "C:\\ConsoleProcess.exe";
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  // создем событие, отмечающее ввод символа
  hInEvent = CreateEvent(NULL, FALSE, FALSE, lpEventName);
  if (hInEvent == NULL)
    return GetLastError();

  // запускаем процесс, который ждет ввод символа
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  if (!CreateProcess(szAppName, NULL, NULL, NULL, FALSE,
      CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
    return 0;
  // закрываем дескрипторы этого процесса
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // ждем оповещение о наступлении события о вводе символа
  dwWaitResult = WaitForSingleObject(hInEvent, INFINITE);
  if (dwWaitResult != WAIT_OBJECT_0)
    return dwWaitResult;

  cout << "A symbol has got." << endl;

  CloseHandle(hInEvent);

  cout << "Press any key to exit.";
  cin.get();

  return 0;
}