#include <windows.h>
#include <iostream.h>

HANDLE hOutEvent, hAddEvent;

DWORD WINAPI thread(LPVOID)
{
  for (int i = 0; i < 10; ++i)
    if (i == 4)
    {
      SetEvent(hOutEvent);
      WaitForSingleObject(hAddEvent, INFINITE);
    }
    
  return 0;
}

int main()
{
  HANDLE  hThread;
  DWORD  IDThread;

  // создаем события с автоматическим сбросом
  hOutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (hOutEvent == NULL)
    return GetLastError();
  hAddEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (hAddEvent == NULL)
    return GetLastError();

  // создаем поток thread
  hThread = CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // ждем, пока поток thread выполнит половину работы
  WaitForSingleObject(hOutEvent, INFINITE);
  // выводим значение переменной
  cout << "A half of the work is done." << endl;
  cout << "Press any key to continue." << endl;
  cin.get();
  // разрешаем дальше работать потоку thread
  SetEvent(hAddEvent);

  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hThread);

  CloseHandle(hOutEvent);
  CloseHandle(hAddEvent);

  cout << "The work is done." << endl;

  return 0;
}