#include <windows.h>
#include <iostream.h>

volatile long n;

void producer()
{
  long goods = 0;
  for (;;)
  {
    ++goods;   // производим новое число
    InterlockedExchange((long*)&n, goods);  // помещаем число в контейнер
    Sleep(150);
  }
}

void consumer()
{
  long goods;
  for (;;)
  {
    Sleep(400);
    InterlockedExchange(&goods, n);  // извлекаем число из контейнера
    cout << "Goods are consumed: " << goods << endl;
  }
}

int main()
{
  HANDLE   hThread_p, hThread_c;
  DWORD  IDThread_p, IDThread_c;
  
  cout << "Press any key to terminate threads." << endl;
  // создаем потоки
  hThread_p = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producer,
                           NULL, 0, &IDThread_p);
  if (hThread_p == NULL)
    return GetLastError();
  hThread_c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)consumer, 
                           NULL, 0, &IDThread_c);
  if (hThread_c == NULL)
    return GetLastError();
  
  cin.get();
  
  // прерываем выполнение потоков
  TerminateThread(hThread_p, 0);
  TerminateThread(hThread_c, 0);
  
  // закрываем дескрипторы потоков
  CloseHandle(hThread_c);
  CloseHandle(hThread_p);
  
  return 0;
}
