#include <windows.h>
#include <iostream.h>

HANDLE hOutEvent[2], hAddEvent;

DWORD WINAPI thread_1(LPVOID)
{
  for (int i = 0; i < 10; ++i)
    if (i == 4)
    {
      SetEvent(hOutEvent[0]);
      WaitForSingleObject(hAddEvent, INFINITE);
    }
    
  return 0;
}

DWORD CALLBACK thread_2(LPVOID)
{
  for (int i = 0; i < 10; ++i)
    if (i == 4)
    {
      SetEvent(hOutEvent[1]);
      WaitForSingleObject(hAddEvent, INFINITE);
    }
    
  return 0;
}

int main()
{
  HANDLE  hThread_1, hThread_2;
  DWORD  IDThread_1, IDThread_2;

  // создаем события с автоматическим сбросом
  hOutEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (hOutEvent[0] == NULL)
    return GetLastError();
  hOutEvent[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (hOutEvent[1] == NULL)
    return GetLastError();

  // создаем событие с ручным сбросом
  hAddEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (hAddEvent == NULL)
    return GetLastError();

  // создаем потоки
  hThread_1 = CreateThread(NULL, 0, thread_1, NULL, 0, &IDThread_1);
  if (hThread_1 == NULL)
    return GetLastError();
  hThread_2 = CreateThread(NULL, 0, thread_2, NULL, 0, &IDThread_2);
  if (hThread_2 == NULL)
    return GetLastError();

  // ждем, пока потоки счетчики выполнят половину работы
  WaitForMultipleObjects(2, hOutEvent, TRUE, INFINITE);
  cout << "A half of the work is done." << endl;
  cout << "Press any key to continue." << endl;
  cin.get();

  // разрешаем потокам счетчикам продолжать работу
  PulseEvent(hAddEvent);

  // ждем завершения потоков
  WaitForSingleObject(hThread_1, INFINITE);
  WaitForSingleObject(hThread_2, INFINITE);

  // закрываем дескрипторы
  CloseHandle(hThread_1);
  CloseHandle(hThread_2);

  CloseHandle(hOutEvent[0]);
  CloseHandle(hOutEvent[1]);
  CloseHandle(hAddEvent);

  cout << "The work is done." << endl;

  return 0;
}