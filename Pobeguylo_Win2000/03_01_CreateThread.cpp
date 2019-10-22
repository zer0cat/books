#include <windows.h>
#include <iostream.h>

volatile int n;

DWORD WINAPI Add(LPVOID iNum)
{
  cout << "Thread is started." << endl;
  n += (int)iNum;
  cout << "Thread is finished." << endl;

  return 0;
}

int main()
{
  int  inc = 10;
  HANDLE  hThread;
  DWORD  IDThread;

  cout << "n = " << n << endl;
  // запускаем поток Add
  hThread = CreateThread(NULL, 0, Add, (void*)inc, 0, &IDThread);
  if (hThread == NULL)
    return GetLastError();

  // ждем, пока поток Add закончит работу
  WaitForSingleObject(hThread, INFINITE);
  // закрываем дескриптор потока Add
  CloseHandle(hThread);

  cout << "n = " << n << endl;

  return 0;
}